#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef int key_t;

typedef struct {
	key_t *data;
	size_t len;
} array;

bool arrayInit(array *a, size_t len) {
	a->data = malloc(sizeof(key_t) * len);
	if (!a->data)
		return false;
	a->len = len;
	return true;
}

void arrayFree(array *a) {
	if (a->data)
		free(a->data);
	a->data = NULL;
}

int main(void) {
	return 0;
}
