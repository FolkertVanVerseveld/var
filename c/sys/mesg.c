/* public domain */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BMAPSZ 32

#define BMAP_RESERVED 0
#define BMAP_FREE 1
#define BMAP_TYPE20 2
#define BMAP_ACPI_NVS 4
#define BMAP_ACPI_DATA 5

struct bmap {
	unsigned long long lower, upper;
	int type;
} e820[BMAPSZ];

const char *tptbl[] = {
	"reserved",
	"free",
	"type 20",
	"acpi nvs",
	"acpi data"
};

#define TPTBLSZ ((sizeof tptbl)/sizeof(tptbl[0]))

unsigned long long m_all[TPTBLSZ];

unsigned b_i = 0;
FILE *pf = NULL;

static void swap(unsigned long long *a, unsigned long long *b)
{
	unsigned long long t;
	t = *a;
	*a = *b;
	*b = t;
}

/* lazy insertion sort */
static void isort(void)
{
	unsigned i, j;
	for (i = 1; i < b_i; ++i) {
		for (j = i; j > 0 && e820[j - 1].lower > e820[j].lower; --j)
			swap(&e820[j].lower, &e820[j - 1].lower);
	}
}

static void cleanup(void)
{
	if (pf) {
		pclose(pf);
		pf = NULL;
	}
}

int main(void)
{
	pf = popen("/bin/dmesg", "r");
	atexit(cleanup);
	do {
		char buf[256];
		size_t n;
		if (!fgets(buf, sizeof buf, pf))
			break;
		buf[(sizeof buf) - 1] = '\0';
		n = strlen(buf);
		if (n) buf[--n] = '\0';
		if (strstr(buf, "BIOS-e820") || strstr(buf, "efi:")) {
			char *ptr;
			if (ptr = strchr(buf, ']'))
				for (++ptr; isspace(*ptr); ++ptr);
			else
				ptr = buf;
			if (*ptr == 'B') {
				int type;
				if (b_i == BMAPSZ) continue;
				ptr = strchr(ptr, '[');
				if (!ptr) continue;
				for (++ptr; isalpha(*ptr) || isspace(*ptr); ++ptr);
				sscanf(ptr, "%llX", &e820[b_i].lower);
				ptr = strchr(ptr, '-');
				if (!ptr) continue;
				++ptr;
				sscanf(ptr, "%llX", &e820[b_i].upper);
				ptr = strchr(ptr, ']');
				if (!ptr) continue;
				for (++ptr; isspace(*ptr); ++ptr);
				type = BMAP_RESERVED;
				if (!strcmp(ptr, "usable"))
					type = BMAP_FREE;
				else if (!strcmp(ptr, "type 20"))
					type = BMAP_TYPE20;
				else if (!strcmp(ptr, "ACPI NVS"))
					type = BMAP_ACPI_NVS;
				else if (!strcmp(ptr, "ACPI data"))
					type = BMAP_ACPI_DATA;
				e820[b_i].type = type;
				++b_i;
			}
		}
	} while (!ferror(pf));
	if (ferror(pf)) {
		perror("/bin/dmesg");
		return 1;
	}
	isort();
	unsigned i;
	for (i = 0; i < b_i; ++i) {
		int type = e820[i].type;
		if (type >= TPTBLSZ)
			type = 0;
		m_all[type] += e820[i].upper - e820[i].lower;
	}
	for (i = 0; i < TPTBLSZ; ++i) {
		const char *stbl = " KMGT", *mag;
		unsigned long long n = m_all[i];
		unsigned rem = 0;
		for (mag = stbl; *mag && n >= 1024; ++mag) {
			rem = n % 1024;
			n /= 1024;
		}
		printf("%10s: %.3f%cB\n", tptbl[i], n + rem / 1024.0f, *mag);
	}
	return 0;
}
