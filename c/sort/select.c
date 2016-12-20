#include "sort.h"

void ssort(int *a, unsigned n) {
	unsigned i, j, min;
	for (j = 0; j < n - 1; ++j) {
		min = j;
		for (i = j + 1; i < n; ++i)
			if (a[i] < a[min])
				min = i;
		if (min != j)
			swap(&a[min], &a[j]);
	}
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	ssort(arr, ARRSZ);
	stop(arr);
	return 0;
}
