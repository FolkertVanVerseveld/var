/**
 * Very simplistic UTF8 string routines.
 *
 * Made by Folkert van Verseveld
 * Licensed under public domain.
 *
 * Compile using: gcc -Wall -Wextra -pedantic -Wno-parentheses -g -DDEBUG -std=c99 utf8.c -o utf8
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* Table for character length. */
const unsigned char chsz_utf8[] = {
	//      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	/* 0 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 1 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 2 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 8 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* 9 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* A */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* B */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	/* C */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	/* D */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	/* E */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
	/* F */ 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 1, 6, 1, 
};

void dump(const void *ptr, size_t size)
{
	const unsigned char *data = ptr, *end = data + size;

	for (; data < end; ++data)
		printf(" %02hhX", *data);
	putchar('\n');
}

size_t strlen_utf8(const char *s)
{
	size_t n = 0, v;

	for (const unsigned char *ptr = (const unsigned char*)s; v = chsz_utf8[*ptr];) {
		++n;
		ptr += v;
	}

	return n;
}

/* Safe variant of above (i.e. it also works with malformed utf8 chars). */
size_t strlen_utf8_s(const char *s)
{
	size_t n = 0, v;

	for (const unsigned char *ptr = (const unsigned char*)s; v = chsz_utf8[*ptr];) {
		++n;
		while (v --> 0)
			if (!chsz_utf8[*++ptr])
				return n;
	}

	return n;
}

char *nextchr_utf8(const char *s)
{
	return (char*)(s + chsz_utf8[(unsigned char)*s]);
}

char *prevchr_utf8(const char *s)
{
	const unsigned char *ptr = (const unsigned char*)s - 1;

	while ((*ptr & 0xC0) == 0x80)
		--ptr;

	return (char*)ptr;
}

int main(void)
{
	const char *str = "$¢€𐍈";

	printf("str: %s\n", str);
	printf("size: %zu\n", strlen(str));
	dump(str, strlen(str) + 1);
	printf("size_utf8: %zu\n", strlen_utf8(str));
	printf("size_utf8_s: %zu\n", strlen_utf8_s(str));

	const char *ptr = nextchr_utf8(nextchr_utf8(str));

	printf("str + 2: %s\n", ptr);
	printf("str + 2 - 1: %s\n", prevchr_utf8(ptr));

	return 0;
}
