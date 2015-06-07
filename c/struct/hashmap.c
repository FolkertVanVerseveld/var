#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

//#define HASHMAP_LOG

#ifdef HASHMAP_LOG
#define logf(f,...) printf("%s: "f,__func__,##__VA_ARGS__)
#define logs(s) printf("%s: %s\n", __func__, s)
#else
#define logf(f,...) ((void)0)
#define logs(s) ((void)0)
#endif

struct hbuck {
	struct hbuck *next;
	void *key, *value;
};

struct hiter {
	struct hbuck *entry;
	size_t nr;
};

typedef struct hmap {
	struct hbuck **buckets;
	size_t size;
	bool (*equals)(void *a, void *b);
	size_t (*hash)(void *key);
	size_t keysz, valsz;
	struct hiter it;
} hashmap;

bool hashmapInit(hashmap *map, size_t size, bool (*equals)(void*,void*), size_t (*hash)(void*), size_t keysz, size_t valsz) {
	map->buckets = malloc(size * sizeof(struct hbuck*));
	if (!map->buckets)
		return false;
	map->size = size;
	for (size_t i = 0; i < size; ++i)
		map->buckets[i] = NULL;
	map->equals = equals;
	map->hash = hash;
	map->keysz = keysz;
	map->valsz = valsz;
	map->it.entry = NULL;
	map->it.nr = map->size;
	return true;
}

bool hashmapIteratorStart(hashmap *map, void **key, void **value) {
	for (size_t i = 0; i < map->size; ++i) {
		if (map->buckets[i]) {
			struct hbuck *b;
			b = map->buckets[i];
			map->it.nr = i;
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			map->it.entry = b->next;
			return true;
		}
	}
	return false;
}

bool hashmapIteratorNext(hashmap *map, void **key, void **value) {
	// if more entries left
	if (map->it.entry) {
		logs("same bucket");
		if (key)
			*key = map->it.entry->key;
		if (value)
			*value = map->it.entry->value;
		map->it.entry = map->it.entry->next;
		return true;
	}
	logs("next bucket");
	// look for next bucket
	for (++map->it.nr; map->it.nr < map->size; ++map->it.nr) {
		if (map->buckets[map->it.nr]) {
			struct hbuck *b = map->buckets[map->it.nr];
			if (key)
				*key = b->key;
			if (value)
				*value = b->value;
			map->it.entry = b->next;
			return true;
		}
	}
	logs("end of map");
	return false;
}

void hashmapForeachEnd(hashmap *map) {
	map->it.entry = NULL;
	map->it.nr = map->size;
}

bool hashmapForeach(hashmap *map, void **key, void **value) {
	if (!map->it.entry && map->it.nr >= map->size) {
		map->it.nr = 0;
		//logs("init");
		if (hashmapIteratorStart(map, key, value))
			return true;
	} else if (hashmapIteratorNext(map, key, value))
		return true;
	//logs("reset");
	hashmapForeachEnd(map);
	return false;
}

bool hashmapPut(hashmap *map, void *key, void *value) {
	struct hbuck *entry;
	entry = malloc(sizeof(struct hbuck));
	if (!entry)
		return false;
	entry->key = malloc(map->keysz);
	entry->value = malloc(map->valsz);
	if (!entry->key || !entry->value)
		return false;
	memcpy(entry->key, key, map->keysz);
	memcpy(entry->value, value, map->valsz);
	size_t hash;
	hash = map->hash(key);
	struct hbuck *b = map->buckets[hash % map->size];
	entry->next = NULL;
	if (!b) {
		logf("open slot %zu\n", hash % map->size);
		map->buckets[hash % map->size] = entry;
	} else {
		logf("append slot %zu\n", hash % map->size);
		while (b) {
			if (hash == map->hash(b->key) && map->equals(b->key, key)) {
				// oops, already exists
				return false;
			}
			if (b->next)
				b = b->next;
			else
				break;
		}
		b->next = entry;
	}
	return true;
}

struct hbuck *hashmapGet(hashmap *map, void *key) {
	size_t hash;
	hash = map->hash(key);
	struct hbuck *b = map->buckets[hash % map->size];
	if (!b)
		return NULL;
	while (b) {
		if (hash == map->hash(b->key) && map->equals(b->key, key))
			return b;
		b = b->next;
	}
	return NULL;
}

void *hashmapGetKey(hashmap *map, void *key) {
	struct hbuck *b = hashmapGet(map, key);
	return b ? b->key : NULL;
}

void *hashmapGetValue(hashmap *map, void *key) {
	struct hbuck *b = hashmapGet(map, key);
	return b ? b->value : NULL;
}

static void _buckfree(struct hbuck *b) {
	free(b->value);
	free(b->key);
	free(b);
}

bool hashmapRemove(hashmap *map, void *key) {
	size_t hash;
	hash = map->hash(key);
	struct hbuck *b = map->buckets[hash % map->size];
	logf("slot %zu\n", hash % map->size);
	if (!b) {
		logs("nothing here");
		return false;
	}
	struct hbuck *prev = b;
	do {
		if (hash == map->hash(b->key) && map->equals(b->key, key)) {
			if (prev != b) {
				logf("delete in %zu\n", hash % map->size);
				prev->next = b->next;
			} else {
				if (b->next)
					logf("delete first in %zu\n", hash % map->size);
				else
					logf("nuke slot %zu\n", hash % map->size);
				map->buckets[hash % map->size] = b->next;
			}
			_buckfree(b);
			return true;
		}
		logs("goto next");
		prev = b;
		b = b->next;
	} while (b);
	return false;
}

void hashmapFree(hashmap *map) {
	for (size_t i = 0; i < map->size; ++i) {
		if (map->buckets[i]) {
			struct hbuck *b = map->buckets[i];
			while (b) {
				struct hbuck *next;
				next = b->next;
				_buckfree(b);
				b = next;
			}
		}
	}
	free(map->buckets);
}

// my test code

bool myequals(void *a, void *b) {
	int *x = a, *y = b;
	return *x == *y;
}

size_t myhash(void *key) {
	int *hash = key;
	return (unsigned) *hash;
}

#define TESTSZ 10
#define MAX 24

int main(void) {
	hashmap map;
	// make a new hashmap with 4 buckets
	// something that looks like:
	// map<int, int>
	srand(time(NULL));
	int i, kk[TESTSZ], vv[TESTSZ];
	int mag = (rand() % 5) + 3;
	puts(" key   value");
	for (i = 0; i < TESTSZ; ++i) {
		kk[i] = 4 + mag * i;
		vv[i] = rand() % MAX;
		printf("%4d  %4d\n", kk[i], vv[i]);
	}
	puts("creating map");
	if (!hashmapInit(&map, TESTSZ/2, myequals, myhash, sizeof(int), sizeof(int))) {
		puts("failed to create map");
		return 1;
	}
	puts("filling map");
	for (i = 0; i < TESTSZ; ++i) {
		if (!hashmapPut(&map, &kk[i], &vv[i])) {
			puts("failed to fill map");
			return 1;
		}
	}
	// now check if the values we get from our hashmap
	// are the same as the values we had put in it
	puts("checking map");
	for (i = 0; i < TESTSZ; ++i) {
		int *val;
		if (val = hashmapGetValue(&map, &kk[i])) {
			if (vv[i] != *val) {
				printf("value mismatch: %d != %d\n", vv[i], *val);
				puts("map check test failed");
				return 1;
			}
		} else {
			puts("no value");
			puts("map check test failed");
			return 1;
		}
	}
	void *ik, *iv;
#if 0
	if (hashmapIteratorStart(&map, &ik, &iv)) {
		int *k = ik, *v = iv;
		printf("first in map: %d %d\n", *k, *v);
		while (hashmapIteratorNext(&map, &ik, &iv)) {
			k = ik;
			v = iv;
			printf("next: %d %d\n", *k, *v);
		}
	} else {
		puts("the iterator sucks!");
		return 1;
	}
#else
	while (hashmapForeach(&map, &ik, &iv)) {
		int *k = ik, *v = iv;
		printf("%d %d\n", *k, *v);
	}
#endif
	// toggle this if you want to check whether not removing
	// the elements still frees the map correctly
#if 1
	puts("removing map");
	for (i = 0; i < TESTSZ; ++i) {
		if (!hashmapRemove(&map, &kk[i])) {
			printf("key not found: %d\n", kk[i]);
			puts("map remove test failed");
			return 1;
		}
	}
#endif
	puts("freeing map");
	hashmapFree(&map);
	return 0;
}
