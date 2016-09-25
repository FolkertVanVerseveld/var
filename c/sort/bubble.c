#include "sort.h"

void bsort(int *a, unsigned n) {
	unsigned i, x;
	do {
		x = 0;
		for (i = 1; i < n; ++i)
			if (a[i - 1] > a[i])
				swap(&a[i - 1], &a[x = i]);
		--n;
	} while (x);
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	bsort(arr, ARRSZ);
	stop(arr);
	return 0;
}
