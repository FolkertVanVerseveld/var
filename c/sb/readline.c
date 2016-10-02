/*
license: public domain
example similar to getline GNU libc extension
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
/*
Readline from f and store in bufx. bufx may be the address of a char NULL ptr.
n and l may be both NULL, but if you call this function multiple times, n should not be a NULL ptr.
n tells readline how many chars fit in bufx. l will hold the number of chars read.
note: assumes '\n' line endings.
Once you're done with readline/readline2, you need to free bufx (i.e. free(*bufx))

readline2 removes the last char (in this case a '\n'), readline does not.
*/
bool readline2(char **restrict bufx, size_t *n, size_t *l, FILE *f);
bool readline(char **restrict bufx, size_t *n, size_t *l, FILE *f);

bool readline2(char **restrict bufx, size_t *n, size_t *l, FILE *f) {
        size_t ll;
        if (readline(bufx, n, &ll, f)) {
                (*bufx)[ll-1] = '\0';
                if (l) *l = ll;
                return true;
        }
        return false;
}

bool readline(char **restrict bufx, size_t *n, size_t *l, FILE *f) {
        size_t bufsz = 1024, ptrsz, read;
        char *buf = *bufx, *ptr, *end;
        if (n && !*n && *n < bufsz) {
                char *nbuf;
                if (!(nbuf = realloc(*bufx, bufsz)))
                        return false;
                buf = nbuf;
        }
        if (!buf && !(buf = malloc(bufsz)))
                return false;
        ptr = buf;
        ptrsz = bufsz;
        while (true) {
                ptrdiff_t off;
                char *nbuf;
                *(end = ptr + ptrsz - 2) = '\n';
                if (!fgets(ptr, ptrsz, f))
                        goto err;
                read = strlen(ptr);
                if (*end == '\n' || (nbuf = memchr(ptr, '\n', ptrsz))) {
                        ptrsz = (bufsz - ptrsz) + read;
                        break;
                }
                bufsz <<= 1;
                off = ptr - buf;
                if (!(nbuf = realloc(buf, bufsz))) {
                        perror("readline");
                        goto err;
                }
                buf = nbuf;
                ptr = buf + off + read;
                ptrsz = buf + bufsz - ptr;
        }
        *bufx = buf;
        if (n) *n = bufsz;
        if (l) *l = ptrsz;
        return true;
err:
        if (buf) free(buf);
        return false;
}

int main(void) {
	size_t len = 2, len2;
	char *buf = NULL;
	while (readline2(&buf, &len, &len2, stdin))
		printf("%zu,%zu\t%s\n", len, len2, buf);
	if (!feof(stdin))
		perror("readline");
	return 0;
}
