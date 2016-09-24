#ifndef SORT
#define SORT

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRSZ 16
#define MAX 100

void swap(int *a, int *b) {
	int c;
	c = *a;
	*a = *b;
	*b = c;
}

void rnd(int *a, unsigned n, unsigned max) {
	unsigned i;
	for (i = 0; i < n; ++i)
		a[i] = rand()%max;
}

void dump(int *a, unsigned n) {
	unsigned i;
	for (i = 0; i < n; ++i)
		printf(" %d", a[i]);
	putchar('\n');
}

int init(int **a, unsigned n) {
	srand(time(NULL));
	*a = malloc(n * sizeof(int));
	if (!*a) {
		perror("malloc");
		return 1;
	}
	rnd(*a, ARRSZ, MAX);
	dump(*a, ARRSZ);
	return 0;
}

void stop(int *a) {
	dump(a, ARRSZ);
	free(a);
}

#endif
