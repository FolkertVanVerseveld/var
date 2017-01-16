#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "scan.h"

unsigned flags = OPT_SLOW;

static int walk(const char *name, const void *data, size_t n)
{
	const char *str = data, *end = str + n;
	while (str < end) {
		char *lf = memchr(str, '\n', (size_t)(end - str));
		if (!lf)
			break;
		*lf = '\0';
		char *sep = strchr(str, ':');
		if (!sep)
			goto next;
		size_t seplen = (size_t)(sep - str);
		if (!strncmp(str, "Totaal", seplen))
			goto next;
		*sep = '\0';
		char *naar = strstr(sep + 1, " naar ");
		if (!naar)
			goto next;
		char *from = sep + 2;
		*naar = '\0';
		char *to = naar + strlen(" naar ");
		char *sep2 = strchr(to, ':');
		if (!sep2)
			goto next;
		*sep2 = '\0';
		char *km = strstr(sep2 + 1, " km,");
		if (!km)
			goto next;
		*km = '\0';
		char *type = km + strlen(" km,") + 1;
		while (!isspace(*km))
			--km;
		++km;
		int length = atoi(km);
		if (length < 0)
			goto next;
		char *vk = strstr(type, "verkeer");
		if (vk) {
			while (!isspace(*vk))
				--vk;
			*vk = '\0';
			while (isspace(*type))
				++type;
			if (!strcmp("Langzaam rijdend", type) && !(flags & OPT_SLOW))
				goto next;
		}
		if (parse(name, str, from, to, (unsigned)length))
			goto next;
next:
		str = lf + 1;
	}
	return 0;
}

static struct option long_opt[] = {
	{"help", no_argument, 0, 'h'},
	{"list", no_argument, 0, 't'},
	{0, 0, 0, 0},
};

static int parse_opt(int argc, char **argv)
{
	int c;
	while (1) {
		int option_index;
		c = getopt_long(argc, argv, "f:tj", long_opt, &option_index);
		if (c == -1) break;
		switch (c) {
		case 'f':
			tjfilter(optarg);
			break;
		case 't':
			flags |= OPT_LIST;
			break;
		case 'j':
			flags &= ~OPT_SLOW;
			break;
		case 'h':
			printf("usage: %s dir\n", argc > 0 ? argv[0] : "stat");
			exit(0);
			break;
		default:
			fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
		}
	}
	return optind;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		goto usage;
	int argp = parse_opt(argc, argv);
	if (argp >= argc) {
usage:
		fprintf(stderr, "usage: %s dir\n", argc > 0 ? argv[0] : "stat");
		return 1;
	}
	const char *dir = argv[argp];
	size_t arglen = strlen(dir);
	char *path = malloc(arglen + 6);
	if (!path) {
		perror(dir);
		return 1;
	}
	struct dirent **list;
	int n;
	char *data = NULL;
	size_t cap = 0;
	if ((n = scandir(dir, &list, NULL, NULL)) < 0) {
		perror(dir);
		return 1;
	}
	for (int i = 0; i < n; ++i) {
		char *name = list[i]->d_name;
		struct stat st;
		int fd = -1;
		if (strlen(name) != 4)
			goto next;
		sprintf(path, "%s/%s", dir, name);
		if ((fd = open(path, O_RDONLY)) == -1 || fstat(fd, &st)) {
			perror(name);
			goto next;
		}
		size_t n = st.st_size;
		if (n > 65536) {
			fputs("file too big\n", stderr);
			continue;
		}
		if (n > cap) {
			char *newdata = realloc(data, cap = n * 2 + 256);
			if (!newdata) {
				perror(name);
				goto next;
			}
			data = newdata;
		}
		ssize_t in = read(fd, data, n);
		if (in < 0 || (size_t)in != n || walk(name, data, n)) {
			perror(name);
			goto next;
		}
next:
		free(list[i]);
		if (fd != -1)
			close(fd);
	}
	tjstat();
	if (data)
		free(data);
	free(list);
	free(path);
	return 0;
}
