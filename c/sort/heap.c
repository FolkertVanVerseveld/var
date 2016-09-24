#include "sort.h"

void perc(int *a, int low, int high) {
	int p, q;
	for (p = a[low]; 2 * low + 1 < high; low = q) {
		q = 2 * low + 1;
		if (q != high - 1 && a[q] < a[q + 1])
			++q;
		if (p < a[q])
			a[low] = a[q];
		else
			break;
	}
	a[low] = p;
}

void heapsort(int *a, int n) {
	int i;
	for (i = n / 2; i >= 0; --i)
		perc(a, i, n);
	for (i = n - 1; i > 0; --i) {
		swap(a, a + i);
		perc(a, 0, i);
	}
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	heapsort(arr, ARRSZ);
	stop(arr);
	return 0;
}
