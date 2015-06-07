#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

typedef struct bheap_t {
	void **data;
	size_t nmemb;
	size_t capacity;
	int (*keycmp)(void *a, void *b);
	void (*keychange)(void *key, void *delta);
	void (*keyfree)(void *key);
	void (*keydump)(void *key);
} binheap;

bool binheapInit(
	binheap *this, size_t nmemb,
	int (*cmp)(void*,void*),
	void (*change)(void*,void*),
	void (*free)(void*),
	void (*dump)(void*)
)
{
	this->capacity = nmemb;
	this->data = malloc(nmemb * sizeof(void*));
	if (!this->data)
		return false;
	this->nmemb = 0;
	this->keycmp = cmp;
	this->keychange = change;
	this->keyfree = free;
	this->keydump = dump;
	return true;
}

#define parent(x) (((x)-1)/2)
#define right(x) (2*((x)+1))
#define left(x) (right(x)-1)

#define swap(this, a, b) _binhkxchg(this, a, b)
static void _binhkxchg(binheap *this, size_t a, size_t b) {
	void *tmp = this->data[a];
	this->data[a] = this->data[b];
	this->data[b] = tmp;
}

static void _minhsift(binheap *this, size_t i) {
	size_t j;
	while (i > 0) {
		j = parent(i);
		if (this->keycmp(this->data[j], this->data[i]) > 0)
			swap(this, j, i);
		else
			break;
		i = j;
	}
}

static void _maxhsift(binheap *this, size_t i) {
	size_t j;
	while (i > 0) {
		j = parent(i);
		if (this->keycmp(this->data[j], this->data[i]) < 0)
			swap(this, j, i);
		else
			break;
		i = j;
	}
}

#define minheapPut(this, k) binheapPut(this, k, true)
#define maxheapPut(this, k) binheapPut(this, k, false)
bool binheapPut(binheap *this, void *key, bool min) {
	if (this->nmemb >= this->capacity)
		// heap is full
		return false;
	this->data[this->nmemb] = key;
	if (min)
		_minhsift(this, this->nmemb++);
	else
		_maxhsift(this, this->nmemb++);
	return true;
}

static void _minhsup(binheap *this, size_t i) {
	size_t l, r, m;
	r = right(i);
	l = left(i);
	m = i;
	if (l < this->nmemb && this->keycmp(this->data[l], this->data[m]) < 0)
		m = l;
	if (r < this->nmemb && this->keycmp(this->data[r], this->data[m]) < 0)
		m = r;
	if (m != i) {
		swap(this, i, m);
		_minhsup(this, m);
	}
}

static void _maxhsup(binheap *this, size_t i) {
	size_t l, r, m;
	r = right(i);
	l = left(i);
	m = i;
	if (l < this->nmemb && this->keycmp(this->data[l], this->data[m]) > 0)
		m = l;
	if (r < this->nmemb && this->keycmp(this->data[r], this->data[m]) > 0)
		m = r;
	if (m != i) {
		swap(this, i, m);
		_maxhsup(this, m);
	}
}

#define minheapExtract(this) binheapExtract(this, true)
#define maxheapExtract(this) binheapExtract(this, false)
void *binheapExtract(binheap *this, bool min) {
	void *key;
	if (!this->nmemb)
		return NULL;
	key = this->data[0];
	this->data[0] = this->data[--this->nmemb];
	if (min)
		_minhsup(this, 0);
	else
		_maxhsup(this, 0);
	return key;
}

#define minheapChange(this,i,d) binheapChange(this,i,d,true)
#define maxheapChange(this,i,d) binheapChange(this,i,d,false)
void binheapChange(binheap *this, size_t index, void *delta, bool min) {
	this->keychange(this->data[index], delta);
	if (min)
		_minhsup(this, index);
	else
		_maxhsup(this, index);
}

void binheapDump(binheap *this) {
	printf("heap count=%zu\n", this->nmemb);
	printf("heap capacity=%zu\n", this->capacity);
	for (size_t i = 0; i < this->nmemb; ++i) {
		printf("%zu\t", i);
		this->keydump(this->data[i]);
	}
}

void binheapFree(binheap *this) {
	for (size_t i = 0; i < this->nmemb; ++i)
		this->keyfree(this->data[i]);
	free(this->data);
}

/* test functions */

typedef struct dict_t {
	char *host;
	int fd;
	char *name;
} Dict;

int DictCompare(void *a, void *b) {
	Dict *da, *db;
	da = a;
	db = b;
	if (da->fd > db->fd)
		return 1;
	if (da->fd == db->fd) {
		if (da->host && db->host)
			return strcmp(da->host, db->host);
		if (da->host)
			return -1;
		if (db->host)
			return 1;
		return 0;
	}
	return -1;
}

void DictChange(void *k, void *delta) {
	Dict *dict = k;
	int *v = delta;
	dict->fd -= *v;
}

void DictDump(void *k) {
	Dict *d = k;
	printf(
		"[host=%16s, name=%10s,fd=%d]\n",
		d->host ? d->host : "null",
		d->name ? d->name : "null",
		d->fd
	);
}

void DictFree(void *k) {
	Dict *d = k;
	if (d->host)
		free(d->host);
	if (d->name)
		free(d->name);
}

static void randstr(char *str, size_t len) {
	const char cc[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while (len > 0) {
		*str++ = cc[rand() % sizeof(cc)];
		--len;
	}
	*str = '\0';
}

int main(void) {
	int ret = 1;
	binheap heap;
	bool heapinit = false;
	size_t i, size = 5;
	Dict *dicts = NULL;
	srand(time(NULL));
	dicts = malloc(size * sizeof(Dict));
	if (!dicts)
		goto err;
	if (!binheapInit(
		&heap, size,
		DictCompare, DictChange,
		DictFree, DictDump)
	)
	{
		fputs("binheapInit fails\n", stderr);
		goto err;
	}
	heapinit = true;
	binheapDump(&heap);
	puts("filling heap");
	for (i = 0; i < size; ++i) {
		dicts[i].fd = rand();
		if (rand() % 4) {
			size_t hostsz = 4 + (rand() % 12);
			dicts[i].host = malloc(hostsz + 1);
			randstr(dicts[i].host, hostsz);
		} else
			dicts[i].host = NULL;
		if (rand() % 2) {
			size_t namesz = 6 + (rand() % 4);
			dicts[i].name = malloc(namesz + 1);
			randstr(dicts[i].name, namesz);
		} else
			dicts[i].name = NULL;
		if (!minheapPut(&heap, &dicts[i])) {
			fputs("minheapPut fails\n", stderr);
			goto err;
		}
	}
	binheapDump(&heap);
	unsigned index = rand() % size;
	printf("changing heap index %d\n", index);
	puts("changing heap");
	Dict *d = heap.data[index];
	int delta = d->fd;
	minheapChange(&heap, index, &delta);
	binheapDump(&heap);
	puts("extracting heap");
	for (i = 0; i < size; ++i) {
		Dict *d = minheapExtract(&heap);
		if (!d) {
			fputs("minheapExtract fails\n", stderr);
			goto err;
		}
		DictDump(d);
		binheapDump(&heap);
		DictFree(d);
	}
	puts("all tests passed");
	ret = 0;
err:
	if (heapinit)
		binheapFree(&heap);
	if (dicts)
		free(dicts);
	return ret;
}
