#include <string.h>
#include "sort.h"

#define K 10

void rsort(int *a, unsigned n) {
	int m, exp, bb[K], *b;
	unsigned i;
	b = malloc(n * sizeof(int));
	for (m = a[0], exp = i = 1; i < n; ++i)
		if (a[i] > m)
			m = a[i];
	while (m / exp > 0) {
		memset(bb, 0, K * sizeof(int));
		for (i = 0; i < n; ++i)
			++bb[(a[i] / exp) % K];
		for (i = 1; i < K; ++i)
			bb[i] += bb[i - 1];
		for (i = n; i > 0;) {
			--i;
			b[--bb[(a[i] / exp) % K]] = a[i];
		}
		memcpy(a, b, n * sizeof(int));
		exp *= K;
#ifdef DEBUG
		printf("pass");
		dump(a, n);
#endif
	}
}

int main(void) {
	int *arr;
	init(&arr, ARRSZ);
	rsort(arr, ARRSZ);
	stop(arr);
	return 0;
}
