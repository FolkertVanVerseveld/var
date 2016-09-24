#include "sort.h"

void isort(int *a, unsigned n) {
	unsigned i, j;
	for (i = 1; i < n; ++i) {
		for (j = i; j >0 && a[j - 1] > a[j]; --j)
			swap(&a[j], &a[j - 1]);
	}
}

#define MAGIC 5

void hybrid(int *a, int low, int high) {
	int i, j, pivot;
	i = low; j = high;
	if (high - low < MAGIC) {
		isort(a + low, high - low + 1);
		return;
	}
	pivot = a[low + (rand() % (high - low))];
	while (i <= j) {
		while (a[i] < pivot)
			++i;
		while (a[j] > pivot)
			--j;
		if (i <= j) {
			swap(&a[i], &a[j]);
			++i;
			--j;
		}
	}
	if (low < j)
		hybrid(a, low, j);
	if (i < high)
		hybrid(a, i, high);
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	hybrid(arr, 0, ARRSZ - 1);
	stop(arr);
	return 0;
}
