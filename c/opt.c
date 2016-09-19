/*
public domain
Concatenates C strings into one string.

In C, one can use "hell" "o mate" and the compiler will concatenate
this into "hello mate". Since I missed this feature in java, I wrote
this program.
It also removes lines that begin with #. This makes it possible to
use the powerful C preprocessor for e.g. java code.
*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define usage fprintf(stderr, "usage: %s in out\n", argc > 0 ? argv[0] : "opt")
#define fcheck(f,n) if(!f || ferror(f)){perror(n);return 1;}

FILE *in, *out;

#define BUFSZ 4096
#define DBUFSZ BUFSZ

char buf[BUFSZ], dbuf[DBUFSZ], *dp = dbuf, *dpe = dbuf + DBUFSZ;
/*
some state vars
c = comment
q = quote
*/
int c, q;

#define init c=q=0

void drain(void) {
	fwrite(dbuf, sizeof(char), (size_t) (dp - dbuf), out);
	dp = dbuf;
}

void put(int ch) {
	*dp++ = ch;
	if (dp == dpe)
		drain();
}

int parse(char **argv) {
	size_t z;
	dp = dbuf;
	init;
	while ((z = fread(buf, sizeof(char), BUFSZ, in)) > 0) {
		char *p;
		for (p = buf; z > 0; ++p, --z) {
#ifndef linux
			if (*p == '\r')
				continue;
#endif
			if (*p == '\n') {
				if (q)
					put('"');
				init;
				put(*p);
			} else if (c) {
				if (*p == '\n')
					init;
			} else if (q) {
				/* if q odd */
				if (q & 1) {
					/* find another quote */
					if (*p == '"')
						++q;
					else
						put(*p);
				} else {
					if (*p == '"')
						--q;
					else if (!isspace(*p)) {
						put('"');
						put(*p);
						q = 0;
					}
				}
			} else if (*p == '#')
				c = 1;
			else {
				if (*p == '"')
					q = 1;
				put(*p);
			}
		}
	}
	drain();
	fcheck(in, argv[1])
	fcheck(out, argv[2])
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		usage;
		return 1;
	}
	if (strcmp(argv[1], argv[2]) == 0) {
		fprintf(stderr, "these settings will nuke the file %s\n", argv[1]);
		usage;
		return 1;
	}
	in = fopen(argv[1], "rb");
	fcheck(in, argv[1])
	out = fopen(argv[2], "wb");
	fcheck(out, argv[2])
	return parse(argv);
}
