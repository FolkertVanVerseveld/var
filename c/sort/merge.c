#include "sort.h"

/*
you can generate another random array with this:
dd if=/dev/urandom bs=4 count=6 | xxd -g1 | cut -c10-48 | tr 'a-z' 'A-Z' | tr ' ' '\n' | xargs -0 echo "ibase=16" | bc
piping is just so awesome :D
*/

void merge(int *a, int *b, int low, int middle, int high) {
	int i, j, k;
	for (i = low; i <= high; i++)
		b[i] = a[i];
	i = low; j = middle + 1; k = low;
	while (i <= middle && j <= high) {
		if (b[i] <= b[j])
			a[k] = b[i++];
		else
			a[k] = b[j++];
		k++;
	}
	while (i <= middle)
		a[k++] = b[i++];
}

void msort(int *a, int *b, int low, int high) {
	if (low < high) {
		int middle = low + (high - low) / 2;
		msort(a, b, low, middle);
		msort(a, b, middle + 1, high);
		merge(a, b, low, middle, high);
	}
}

int main(void) {
	int *arr, *harr;
	srand(time(NULL));
	arr = malloc(ARRSZ * sizeof(int));
	harr = malloc(ARRSZ * sizeof(int));
	if (!arr || !harr) {
		perror("malloc");
		return 1;
	}
	rnd(arr, ARRSZ, MAX);
	dump(arr, ARRSZ);
	msort(arr, harr, 0, ARRSZ - 1);
	dump(arr, ARRSZ);
	free(harr);
	free(arr);
	return 0;
}
