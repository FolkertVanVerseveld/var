#include "sort.h"

/*
pick pivot from array
reorder array all el < pivot before it and all el > pivot after it (equal go either way)
recursively apply first two steps
*/

void quicksort(int *a, int low, int high) {
	int i, j, pivot;
	i = low; j = high;
	pivot = a[low + (high-low) / 2];
	while (i <= j) {
		while (a[i] < pivot)
			i++;
		while (a[j] > pivot)
			j--;
		if (i <= j) {
			swap(&a[i], &a[j]);
			i++;
			j--;
		}
	}
	if (low < j)
		quicksort(a, low, j);
	if (i < high)
		quicksort(a, i, high);	
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	quicksort(arr, 0, ARRSZ - 1);
	stop(arr);
	return 0;
}
