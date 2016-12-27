/*
linked list with holes

uses stack for tracking empty slots

may be resizable in the future
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef int key_t;
typedef uint64_t claim_t;

#define CLAIM_SLOTSZ 64

struct hlist {
	size_t n, npop, cap;
	size_t *pop;
	claim_t *set;
	key_t *d;
};

#define HLIST_INIT {.pop=NULL,.set=NULL,.d=NULL}

void hinit(struct hlist *l)
{
	l->pop = NULL;
	l->set = NULL;
	l->d = NULL;
}

void hclear(struct hlist *l)
{
	l->n = l->npop = 0;
}

int hnew(struct hlist *l, size_t cap)
{
	int ret = 1;
	size_t *pop = NULL;
	size_t setsz = cap / CLAIM_SLOTSZ + 1;
	uint64_t *set = NULL;
	key_t *d = NULL;
	if (!(pop  = malloc(cap * sizeof(size_t))))
		goto fail;
	if (!(set  = calloc(setsz, sizeof(claim_t))))
		goto fail;
	if (!(d    = malloc(cap * sizeof(size_t))))
		goto fail;
	l->pop  = pop;
	l->set  = set;
	l->d    = d;
	hclear(l);
	l->cap = cap;
	ret = 0;
fail:
	if (ret) {
		if (pop ) free(pop);
		if (set ) free(set);
		if (d   ) free(d);
	}
	return ret;
}

void hfree(struct hlist *l)
{
	if (l->pop) {
		free(l->pop);
		l->pop = NULL;
	}
	if (l->set) {
		free(l->set);
		l->set = NULL;
	}
	if (l->d) {
		free(l->d);
		l->d = NULL;
	}
}

int hresize(struct hlist *l, size_t cap)
{
	int ret = 1;
	size_t ocap = l->cap;
	size_t *pop = NULL;
	size_t setsz = cap / CLAIM_SLOTSZ + 1, osetsz = ocap / CLAIM_SLOTSZ + 1;
	uint64_t *set = NULL;
	key_t *d = NULL;
	if (!(pop  = realloc(l->pop , cap * sizeof(size_t))))
		goto fail;
	if (!(set  = realloc(l->set , cap * sizeof(claim_t))))
		goto fail;
	if (!(d    = realloc(l->d   , cap * sizeof(size_t))))
		goto fail;
	if (cap > ocap && setsz > osetsz)
		memset(&set[ocap], 0, (setsz - osetsz) * sizeof(claim_t));
	l->pop  = pop;
	l->set  = set;
	l->d    = d;
	l->cap  = cap;
	ret = 0;
fail:
	if (ret) {
		if (pop  && pop  != l->pop ) free(pop);
		if (set  && set  != l->set ) free(set);
		if (d    && d    != l->d   ) free(d);
	}
	return ret;
}

void hdump(const struct hlist *l)
{
	printf("n=%zu\n", l->n);
	if (!l->n)
		return;
	for (size_t i = 0; i < l->n; ++i)
		printf(" %d", l->d[i]);
	putchar('\n');
	for (size_t i = 0; i <= l->n / CLAIM_SLOTSZ; ++i)
		printf(" %lx", l->set[i]);
	putchar('\n');
}

int hput(struct hlist *l, key_t v)
{
	if (l->npop) {
		// reclaim empty slot
		size_t i = l->pop[--l->npop];
		l->set[i / CLAIM_SLOTSZ] |= 1 << i % CLAIM_SLOTSZ;
		l->d[i] = v;
		hdump(l);
		return 0;
	}
	// if full
	if (l->n == l->cap) {
		int ret = hresize(l, l->cap << 1);
		if (ret) return ret;
	}
	l->set[l->n / CLAIM_SLOTSZ] |= 1 << l->n % CLAIM_SLOTSZ;
	l->d[l->n++] = v;
	hdump(l);
	return 0;
}

int hremove(struct hlist *l, size_t i)
{
	if (i > l->n)
		return 1;
	// if already empty
	if (!(l->set[i / CLAIM_SLOTSZ] & (1 << i % CLAIM_SLOTSZ))) {
		// cannot remove empty slot
		fprintf(stderr, "empty: %zu\n", i);
		return 1;
	}
	l->set[i / CLAIM_SLOTSZ] &= ~(1 << i % CLAIM_SLOTSZ);
	l->pop[l->npop++] = i;
	return 0;
}

int main(void) {
	struct hlist list;
	hinit(&list);
	hnew(&list, 3);
	hdump(&list);
	hput(&list, 4);
	hput(&list, -2);
	hput(&list, 8);
	hremove(&list, 1);
	hremove(&list, 1);
	hdump(&list);
	hput(&list, 7);
	hput(&list, -3);
	hdump(&list);
	hfree(&list);
	return 0;
}
