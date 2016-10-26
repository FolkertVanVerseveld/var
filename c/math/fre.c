#include <stdio.h>

/* Add up 1e8 float 1.0's to demonstrate round-off error.  */
int main(void)
{
	float sum = 0;
	long i;

	for (i = 0; i < 100000000L; ++i)
		sum += 1.0;

	printf("sum = %e (diff = %e)\n", sum, 100000000.0 - sum);

	return 0;
}

