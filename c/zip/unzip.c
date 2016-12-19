/* show zip file listing (similar to unzip(1) -l) */
#include <stddef.h>
#include <stdio.h>
#include <unzip.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s file\n", argc > 0 ? argv[0] : "unzip");
		return 1;
	}
	int ret = 1;
	unzFile zf = unzOpen64(argv[1]);
	unz_global_info64 info;
	if (!zf) {
		perror("open");
		goto fail;
	}
	if (unzGetGlobalInfo64(zf, &info) != UNZ_OK)
		fputs("no global info\n", stderr);
	if (unzGoToFirstFile(zf) != UNZ_OK) {
		fputs("no listing available\n", stderr);
		goto fail;
	}
	int nret;
	size_t total = 0;
	printf("Archive:  %s\n", argv[1]);
	puts(
		"  Length      Date    Time    Name\n"
		"---------  ---------- -----   ----"
	);
	do {
		char fname[4096];
		unz_file_info64 item;
		if (unzGetCurrentFileInfo64(zf, &item, fname, sizeof fname, NULL, 0, NULL, 0) != UNZ_OK) {
			perror("listing failed");
			goto fail;
		}
		printf(
			"%9zu  %4u-%02u-%02u %02u:%02u   %s\n",
			(size_t)item.compressed_size,
			item.tmu_date.tm_year, item.tmu_date.tm_mon + 1, item.tmu_date.tm_mday,
			item.tmu_date.tm_hour, item.tmu_date.tm_min,
			fname
		);
		total += item.compressed_size;
	} while ((nret = unzGoToNextFile(zf)) == UNZ_OK);
	if (nret != UNZ_END_OF_LIST_OF_FILE) {
		perror("listing failed");
		goto fail;
	}
	puts("---------                     -------");
	printf("%9zu                     %zu files\n", total, (size_t)info.number_entry);
	ret = 0;
fail:
	if (unzClose(zf) != UNZ_OK) {
		perror("close");
		return 1;
	}
	return ret;
}
