#include "sort.h"

void isort(int *a, unsigned n) {
	unsigned i, j;
	for (i = 1; i < n; ++i) {
		for (j = i; j > 0 && a[j - 1] > a[j]; --j)
			swap(&a[j], &a[j - 1]);
	}
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	isort(arr, ARRSZ);
	stop(arr);
	return 0;
}
