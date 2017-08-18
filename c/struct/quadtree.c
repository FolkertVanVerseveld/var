/* Copyright 2016-2017 Folkert van Verseveld. Licensed under Apache 2.0 */

/*
 * Quadtree
 *
 * Quadtrees are very efficient in 2D spaces for e.g.:
 * * Collision detection
 * * Unstructured grid
 * * Nearest neighbor search
 *
 * Amongst other things. NOTE: it does not allow dynamic resizing!
 *
 * Copyright Folkert van Verseveld
 * License: Apache 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct point {
	int x, y;
	/* you can add more data here */
};

#define QT_EMPTY 0
#define QT_LEAF 1
#define QT_NODE 2

#define QT_CAP_MIN 4
#define QT_LEAF_MAX 4

struct quadnode {
	struct quadnode *parent;
	union {
		struct {
			struct quadnode *lt, *rt, *lb, *rb;
		} node;
		struct {
			struct point points[QT_LEAF_MAX];
			unsigned count;
		} leaf;
	} data;
	unsigned type;
	int x, y, w, h;
};

struct quadtree {
	struct quadnode *nodes;
	unsigned count, cap;
	unsigned points;
	int x, y, w, h;
};

void point_init(struct point *p)
{
	p->x = p->y = 0;
}

void point_free(struct point *p)
{
	/* you can add cleanup code here */
	point_init(p);
}

int point_create(struct point *p, int x, int y)
{
	p->x = x;
	p->y = y;
	return 0;
}

void quadnode_init(struct quadnode *n)
{
	n->parent = NULL;
	n->type = QT_EMPTY;
	n->x = n->y = n->w = n->h = 0;
}

void quadnode_free(struct quadnode *n)
{
	/* you can add cleanup code here */
	quadnode_init(n);
}

void quadnode_shape(struct quadnode *n, int x, int y, int w, int h)
{
	n->x = x;
	n->y = y;
	n->w = w;
	n->h = h;
}

/*
 * Initialize an object in the leaf.
 * You can add more initialization code here.
 * Currently, this always succeeds (i.e. returns zero).
 */
int quadnode_create_point(struct quadnode *n, const struct point *obj)
{
	n->type = QT_LEAF;
	n->data.leaf.count = 1;
	n->data.leaf.points[0] = *obj;
	return 0;
}

void quadnode_create_node(
	struct quadnode *n,
	struct quadnode *lt, struct quadnode *rt,
	struct quadnode *lb, struct quadnode *rb
)
{
	n->type = QT_NODE;
	n->data.node.lt = lt;
	n->data.node.rt = rt;
	n->data.node.lb = lb;
	n->data.node.rb = rb;
}

void quadtree_init(struct quadtree *q)
{
	q->nodes = NULL;
	q->count = q->cap = 0;
	q->points = 0;
}

void quadtree_free(struct quadtree *q)
{
	if (q->nodes)
		free(q->nodes);
	quadtree_init(q);
}

int quadtree_create(struct quadtree *q, unsigned cap, int x, int y, int w, int h)
{
	if (cap < QT_CAP_MIN)
		return 1;
	struct quadnode *nodes = malloc(cap * sizeof *nodes);
	if (!nodes)
		return 1;
	q->nodes = nodes;
	q->count = 1;
	q->cap = cap;
	q->x = x; q->y = y;
	q->w = w; q->h = h;
	quadnode_init(&q->nodes[0]);
	quadnode_shape(&q->nodes[0], x, y, w, h);
	return 0;
}

int quadnode_has_point(const struct quadnode *n, const struct point *obj)
{
	int l, r, t, b;
	/* determine bounds */
	l = n->x - n->w / 2;
	r = n->x + n->w / 2;
	t = n->y - n->h / 2;
	b = n->y + n->h / 2;
	return obj->x >= l && obj->x < r && obj->y >= t && obj->y < b;
}

int quadtree_has_point(const struct quadtree *q, const struct point *obj)
{
	int l, r, t, b;
	/* determine bounds */
	l = q->x - q->w / 2;
	r = q->x + q->w / 2;
	t = q->y - q->h / 2;
	b = q->y + q->h / 2;
	return obj->x >= l && obj->x < r && obj->y >= t && obj->y < b;
}

void quadnode_dump(const struct quadnode *n, unsigned level);
int quadnode_add_point(struct quadtree *q, struct quadnode *n, struct point *obj);

int quadnode_subdivide(struct quadtree *q, struct quadnode *n)
{
	int retval = 1;
	struct quadnode *lt, *rt, *lb, *rb;
	lt = rt = lb = rb = NULL;
	/*
	 * We need 4 more nodes for subdivision.
	 * NOTE: We cannot resize the quadtree without rewriting all pointer access
	 *       to indirect access with indices, because if the resized block is
	 *       moved, all pointers are invalidated!
	 */
	if (q->count + 3 >= q->cap)
		goto fail;
	/* init all quadrants */
	unsigned count = q->count;
	lt = &q->nodes[count];
	rt = &q->nodes[count + 1];
	lb = &q->nodes[count + 2];
	rb = &q->nodes[count + 3];
	quadnode_init(lt);
	quadnode_init(rt);
	quadnode_init(lb);
	quadnode_init(rb);
	lt->parent = rt->parent = lb->parent = rb->parent = n;
	/* shape quadrants */
	int hw, hh;
	hw = n->w / 2;
	hh = n->w / 2;
	quadnode_shape(lt, n->x - hw / 2, n->y - hh / 2, hw, hh);
	quadnode_shape(rt, n->x + hw / 2, n->y - hh / 2, hw, hh);
	quadnode_shape(lb, n->x - hw / 2, n->y + hh / 2, hw, hh);
	quadnode_shape(rb, n->x + hw / 2, n->y + hh / 2, hw, hh);
	puts("subdivide");
	/* `move' all points to quadrants */
	for (unsigned i = 0, j = n->data.leaf.count; i < j; ++i) {
		struct point *p = &n->data.leaf.points[i];
		if (!quadnode_add_point(q, lt, p))
			continue;
		if (!quadnode_add_point(q, rt, p))
			continue;
		if (!quadnode_add_point(q, lb, p))
			continue;
		if (!quadnode_add_point(q, rb, p))
			continue;
		goto fail;
	}
	quadnode_dump(lt, 0);
	quadnode_dump(rt, 0);
	quadnode_dump(lb, 0);
	quadnode_dump(rb, 0);
	/* apply changes */
	n->type = QT_NODE;
	n->data.node.lt = lt;
	n->data.node.rt = rt;
	n->data.node.lb = lb;
	n->data.node.rb = rb;
	q->count += 4;
	retval = 0;
fail:
	if (retval) {
		if (lt)
			quadnode_free(lt);
		if (rt)
			quadnode_free(rt);
		if (lb)
			quadnode_free(lb);
		if (rb)
			quadnode_free(rb);
	}
	return retval;
}

int quadnode_add_point(struct quadtree *q, struct quadnode *n, struct point *obj)
{
	int retval = 1;
	(void)q;
	if (!quadnode_has_point(n, obj))
		goto fail;
	switch (n->type) {
	case QT_EMPTY:
		/* occupy cell */
		return quadnode_create_point(n, obj);
	case QT_LEAF:
		/* add while not full */
		if (n->data.leaf.count < QT_LEAF_MAX) {
			n->data.leaf.points[n->data.leaf.count++] = *obj;
			return 0;
		}
		/* subdivide */
		retval = quadnode_subdivide(q, n);
		if (retval)
			goto fail;
		/* FALL THROUGH */
	case QT_NODE:
		if (!(retval = quadnode_add_point(q, n->data.node.lt, obj)))
			return 0;
		if (!(retval = quadnode_add_point(q, n->data.node.rt, obj)))
			return 0;
		if (!(retval = quadnode_add_point(q, n->data.node.lb, obj)))
			return 0;
		if (!(retval = quadnode_add_point(q, n->data.node.rb, obj)))
			return 0;
		goto fail;
	}
	retval = 0;
fail:
	return retval;
}

int quadtree_add_point(struct quadtree *q, struct point *obj)
{
	int retval = 1;
	if (!quadtree_has_point(q, obj))
		goto fail;
	retval = quadnode_add_point(q, &q->nodes[0], obj);
fail:
	if (!retval)
		++q->points;
	return retval;
}

void point_dump(const struct point *p)
{
	printf("position: (%d,%d)\n", p->x, p->y);
}

void quadnode_dump_leaf(const struct quadnode *node)
{
	for (unsigned i = 0, n = node->data.leaf.count; i < n; ++i)
		point_dump(&node->data.leaf.points[i]);
}

void quadnode_dump(const struct quadnode *n, unsigned level)
{
	int l, t, r, b;
	const char *type = "???";
	l = n->x - n->w / 2;
	r = n->x + n->w / 2;
	t = n->y - n->h / 2;
	b = n->y + n->h / 2;
	switch (n->type) {
	case QT_EMPTY: type = "empty"; break;
	case QT_LEAF: type = "leaf"; break;
	case QT_NODE: type = "node"; break;
	}
	printf("level %u:\n", level);
	printf("bounds: ((%d,%d),(%d,%d))\n", l, t, r, b);
	printf("type: %s\n", type);
	switch (n->type) {
	case QT_EMPTY: break;
	case QT_LEAF: quadnode_dump_leaf(n); break;
	case QT_NODE:
		quadnode_dump(n->data.node.lt, level + 1);
		quadnode_dump(n->data.node.rt, level + 1);
		quadnode_dump(n->data.node.lb, level + 1);
		quadnode_dump(n->data.node.rb, level + 1);
		break;
	}
}

void quadtree_dump(const struct quadtree *q)
{
	int l, t, r, b;
	l = q->x - q->w / 2;
	r = q->x + q->w / 2;
	t = q->y - q->h / 2;
	b = q->y + q->h / 2;
	puts("quadtree:");
	printf("bounds: ((%d,%d),(%d,%d))\n", l, t, r, b);
	printf("count: %u\n", q->count);
	quadnode_dump(&q->nodes[0], 0);
}

unsigned quadnode_point_count(const struct quadnode *n)
{
	unsigned count;
	switch (n->type) {
	case QT_NODE:
		count = 0;
		count += quadnode_point_count(n->data.node.lt);
		count += quadnode_point_count(n->data.node.rt);
		count += quadnode_point_count(n->data.node.lb);
		count += quadnode_point_count(n->data.node.rb);
		return count;
	case QT_LEAF: return n->data.leaf.count;
	default: return 0;
	}
}

unsigned quadtree_point_count(const struct quadtree *q)
{
	return quadnode_point_count(&q->nodes[0]);
}

/* size must be a power of two */
#define SIZE 256

#define COUNT 40

int main(void)
{
	struct quadtree tree;
	struct point p;
	int retval = 1;

	srand(time(NULL));
	quadtree_init(&tree);
	point_init(&p);

	if (quadtree_create(&tree, 32, 0, 0, SIZE, SIZE))
		goto fail;
	for (unsigned i = 0; i < COUNT; ++i) {
		struct point p;
		int x, y;

		x = (rand() % SIZE) - SIZE / 2;
		y = (rand() % SIZE) - SIZE / 2;
		point_create(&p, x, y);

		if (quadtree_add_point(&tree, &p)) {
			fputs("oops\n", stderr);
			goto fail;
		}
	}
	retval = 0;
fail:
	point_free(&p);
	quadtree_dump(&tree);
	printf("point count: %u\n", tree.points);
	printf("should match: %u\n", quadtree_point_count(&tree));
	quadtree_free(&tree);
	return retval;
}
