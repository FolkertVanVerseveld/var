#include "sort.h"

#define K 10

void bsort(int *a, unsigned n) {
	unsigned bn[K];
	int *b[K], *bb;
	unsigned i, j, r;
	bb = malloc(n * K * sizeof(int));
	if (!bb) {
		perror("malloc");
		return;
	}
	for (i = 0; i < K; ++i) {
		b[i] = bb + i * n;
		bn[i] = 0;
	}
	for (i = 0; i < n; ++i) {
		j = a[i] / K;
		b[j][bn[j]++] = a[i];
	}
	r = 0;
	for (i = 0; i < K; ++i)
		for (j = 0; j < bn[i]; ++j)
			a[r++] = b[i][j];
	for (i = 1; i < n; ++i)
		for (j = i; j > 0 && a[j - 1] > a[j]; --j)
			swap(&a[j], &a[j - 1]);
	free(bb);
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	bsort(arr, ARRSZ);
	stop(arr);
	return 0;
}
