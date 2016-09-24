#include "sort.h"

void bsort(int *a, unsigned n) {
	unsigned i, x;
loop:
	x = 0;
	for (i = 1; i < n; ++i) {
		if (a[i - 1] > a[i]) {
			swap(&a[i - 1], &a[i]);
			x = i;
		}
	}
	--n;
	if (x)
		goto loop;
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	bsort(arr, ARRSZ);
	stop(arr);
	return 0;
}
