/*
 * options parser
 *
 * Takes multiple arguments and parses them either in short or long form.
 * E.g. -h --help. Also recognizes -- as argument stop indicator.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define VERSION "0"

static int help = 0;
static const char *address = NULL;

static struct option long_opt[] = {
	{"help", no_argument, 0, 'h'},
	{"port", required_argument, 0, 'p'},
	{"address", required_argument, 0, 'a'},
	{0, 0, 0, 0}
};

static int parse_opt(int argc, char **argv)
{
	int c, o_i;
	while (1) {
		/*
		 * `:' means required argument
		 * `::' means optional argument (glibc extension)
		 */
		c = getopt_long(argc, argv, "hp:a:", long_opt, &o_i);
		if (c == -1) break;
		switch (c) {
		case 'h':
			help = 1;
			puts(
				"Stub longopt parser\n"
				"usage: longopt OPTIONS FILES...\n"
				"version " VERSION "\n"
				"available options:\n"
				"ch long    description\n"
				" h help    this help\n"
				" p port    network endpoint number\n"
				" a address endpoint IP\n"
			);
			break;
		case 'p': {
			int port;
			port = atoi(optarg);
			if (port < 1 || port > 65535) {
				fprintf(stderr, "%s: bad port, use 1-65535\n", optarg);
				return -1;
			}
			break;
		};
		case 'a':
			address = optarg;
			break;
		}
	}
	return o_i;
}

int main(int argc, char **argv)
{
	int ret, argp;
	ret = parse_opt(argc, argv);
	if (ret < 0)
		return -ret;
	puts("remaining args:");
	for (argp = optind; argp < argc; ++argp)
		puts(argv[argp]);
	return 0;
}
