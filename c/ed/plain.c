// public domain
/*
simple hex editor supporting:
* random direct access
* files up to 16 EiB
* peek 16 bytes at a time
* poke randomly
* search for bytes
* dump file size
* truncate file

commands (XX are hex numbers, ... is variable length):
XX        peek 16 bytes at XX
XX XX...  poke ... at XX
q         quit (no spaces; must be first character)
g         dump file name and size
h XX...   search (hunt) for XX...
hXX XX... search (hunt) for XX... starting at XX
tXX       truncate file to XX bytes
mYY XX n  move N bytes from XX to YY
v         start visual environment using ncurses

note: all write operations are directly executed
note: you can *not* undo any operation
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "view.h"

#define CMDBUFSZ 80

static int fd = -1;
static struct stat st;
static char *map, *name;
static size_t mapsz;
static char cmd[CMDBUFSZ];

#define READONLY 1

static unsigned flags;

static void cleanup(void)
{
	if (map) {
		munmap(map, mapsz);
		map = NULL;
	}
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}

static int ch2hex(int ch, int def)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	return def;
}

static int poke(size_t fpos, unsigned pos)
{
	if (flags & READONLY) {
		fputs("readonly\n", stderr);
		return 1;
	}
	if (fpos >= mapsz) {
		if (!mapsz)
			fputs("file empty\n", stderr);
		else
			fprintf(stderr, "%zX: too large (max: %zX)\n", fpos, mapsz - 1);
		return 1;
	}
	size_t max = mapsz - fpos;
	char pa[CMDBUFSZ];
	unsigned pai = 0;
	for (int ch = cmd[pos]; ch; ch = cmd[++pos]) {
		if (isxdigit(ch)) {
			ch = cmd[pos + 1];
			if (ch && isxdigit(ch))
				pa[pai] = ch2hex(cmd[pos], 0xf) << 4 | ch2hex(ch, 0xf);
			else
				pa[pai] = ch2hex(cmd[pos], 0xf);
			++pai;
			++pos;
		} else if (isspace(ch))
			continue;
		else
			return -1;
	}
	if (pai > max) {
		fprintf(stderr, "overflow: max: %zX, but got: %u\n", max, pai);
		return 1;
	}
	memcpy(&map[fpos], pa, pai);
	return 0;
}

static void peek(size_t fpos, unsigned n)
{
	size_t pos = fpos;
	for (unsigned i = 0; n; --n, ++pos) {
		if (++i == 0x20) {
			putchar('\n');
			i = 0;
		}
		if (pos >= mapsz)
			fputs("~~ ", stdout);
		else
			printf("%02hX ", map[pos] & 0xff);
	}
	putchar('\n');
}

static int hunt(unsigned pos)
{
	unsigned posp = pos;
	size_t start = 0;
	if (isxdigit(cmd[pos])) {
		sscanf(&cmd[pos], "%zX", &start);
		while (isxdigit(cmd[pos]))
			++pos;
	}
	if (!isspace(cmd[pos]))
		return -1;
	for (++pos; isspace(cmd[pos]); ++pos)
		;
	if (!(cmd[pos]))
		return -1;
	char pa[CMDBUFSZ];
	unsigned pai = 0;
	for (int ch = cmd[pos]; ch; ch = cmd[++pos]) {
		if (isxdigit(ch)) {
			ch = cmd[pos + 1];
			if (ch && isxdigit(ch))
				pa[pai] = ch2hex(cmd[pos], 0xf) << 4 | ch2hex(ch, 0xf);
			else
				pa[pai] = ch2hex(cmd[pos], 0xf);
			++pai;
			++pos;
		} else if (isspace(ch))
			continue;
		else
			return -1;
	}
	// naive search
	size_t fpos, max = mapsz;
	if (pai > max) {
		fprintf(stderr, "not found:%s", &cmd[posp]);
		return 1;
	}
	for (fpos = start; fpos <= max - pai; ++fpos)
		if (map[fpos] == pa[0] && !memcmp(&map[fpos], pa, pai)) {
			printf("%zX\n", fpos);
			return 0;
		}
	fprintf(stderr, "not found:%s", &cmd[posp]);
	return 1;
}

static int ftrunc(unsigned pos)
{
	if (flags & READONLY) {
		fputs("readonly\n", stderr);
		return 1;
	}
	if (!cmd[pos])
		return -1;
	size_t size = mapsz;
	if (isxdigit(cmd[pos])) {
		sscanf(&cmd[pos], "%zX", &size);
		while (isxdigit(cmd[pos]))
			++pos;
	}
	if (cmd[pos] != '\n' && cmd[pos])
		return -1;
	if (size == mapsz)
		return 0;
	if (ftruncate(fd, size)) {
		perror("can't truncate");
		return 1;
	}
	char *nmap = mremap(map, mapsz, size, MREMAP_MAYMOVE);
	if (nmap == MAP_FAILED) {
		perror("can't truncate");
		exit(1);
	}
	if (map != nmap)
		memcpy(nmap, map, mapsz);
	if (msync(map, mapsz, MS_SYNC))
		perror("can't sync");
	map = nmap;
	mapsz = size;
	return 0;
}

static int bmove(unsigned pos)
{
	if (flags & READONLY) {
		fputs("readonly\n", stderr);
		return 1;
	}
	if (!isxdigit(cmd[pos]))
		return -1;
	size_t n, ofrom, oto;
	sscanf(&cmd[pos], "%zX", &oto);
	while (isxdigit(cmd[pos]))
		++pos;
	if (!cmd[pos] || !isspace(cmd[pos]))
		return -1;
	while (isspace(cmd[pos]))
		++pos;
	if (!isxdigit(cmd[pos]))
		return -1;
	sscanf(&cmd[pos], "%zX", &ofrom);
	while (isxdigit(cmd[pos]))
		++pos;
	if (!cmd[pos] || !isspace(cmd[pos]))
		return -1;
	while (isspace(cmd[pos]))
		++pos;
	if (!isxdigit(cmd[pos]))
		return -1;
	sscanf(&cmd[pos], "%zX", &n);
	if (n >= mapsz) {
		if (!mapsz)
			fputs("file empty\n", stderr);
		else
			fprintf(stderr, "block too large: %zX (max: %zX)\n", n, mapsz - 1);
		return 1;
	}
	if (ofrom == oto)
		return 0;
	size_t max = mapsz - n;
	if (ofrom > max) {
		fprintf(stderr, "bad source: %zX (max: %zX)\n", ofrom, max);
		return 1;
	}
	if (oto > max) {
		fprintf(stderr, "bad dest: %zX (max: %zX)\n", oto, max);
		return 1;
	}
	memmove(&map[oto], &map[ofrom], n);
	printf("moved %zX bytes from %zX to %zX\n", n, ofrom, oto);
	return 0;
}

static int parse(void)
{
	unsigned pos;
	for (pos = 0; isspace(cmd[pos]); ++pos)
		;
	int ch;
	size_t off = 0;
	if (!(ch = cmd[pos]))
		return 0;
	if (isxdigit(ch)) {
		sscanf(cmd, "%zX", &off);
		for (++pos; isxdigit(cmd[pos]); ++pos)
			;
		if (cmd[pos] != '\n' && isspace(cmd[pos])) {
			for (++pos; cmd[pos] && isspace(cmd[pos]); ++pos)
				;
			return poke(off, pos);
		}
		switch (cmd[pos]) {
		case '\n':
		case '\0':
			peek(off, 16);
			break;
		default: goto fail;
		}
		goto ok;
	}
	switch (cmd[pos]) {
	case 'g': {
		struct stat st;
		fstat(fd, &st);
		printf("%s, size: %zX\n", name, st.st_size);
		break;
	}
	case 'h':
		return hunt(pos + 1);
	case 't':
		return ftrunc(pos + 1);
	case 'm':
		return bmove(pos + 1);
	case 'v':
		return visual();
	default: goto fail;
	}
ok:
	return 0;
fail:
	return -1;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s file\n", argc > 0 ? argv[0] : "plain");
		return 1;
	}
	int ret = 1;
	atexit(cleanup);
	map = NULL;
	mapsz = 0;
	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			perror(argv[1]);
			goto fail;
		}
		flags |= READONLY;
	}
	if (fstat(fd, &st)) {
		perror(argv[1]);
		goto fail;
	}
	int prot = PROT_READ;
	if (!(flags & READONLY))
		prot |= PROT_WRITE;
	map = mmap(NULL, mapsz = st.st_size, prot, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		perror(argv[1]);
		goto fail;
	}
	name = argv[1];
	while (fgets(cmd, CMDBUFSZ, stdin) && *cmd != 'q') {
		cmd[CMDBUFSZ - 1] = '\0';
		ret = parse();
		if (ret < 0)
			fprintf(stderr, "? %s", cmd);
	}
	ret = 0;
fail:
	cleanup();
	return ret;
}
