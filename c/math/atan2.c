#include <stdio.h>
#include <math.h>

int main(void)
{
	int dx[] = {1, 0, -1};
	int dy[] = {1, 0, -1};
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			printf("%d,%d: %f\n", dx[i], dy[j], atan2(dy[j], dx[i]) * 180.0 / M_PI);
	return 0;
}
