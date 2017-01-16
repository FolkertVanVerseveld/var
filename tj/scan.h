#ifndef SCAN_H
#define SCAN_H

#ifdef __cplusplus
extern "C" {
#else
#endif

#define OPT_LIST 1
#define OPT_SLOW 2

extern unsigned flags;

void tjfilter(const char *arg);
int parse(const char *name, const char *road, const char *from, const char *to, unsigned km);
void tjstat(void);

#ifdef __cplusplus
}
#endif

#endif
