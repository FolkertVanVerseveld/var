/*
 * Segmentation violation handler
 *
 * Simple handler that reports crashes and prints some diagnostics.
 * You can disable the hander by including -DNO_SEGV_HANDLER in CFLAGS.
 *
 * Copyright Folkert van Verseveld
 * License: Apache 2.0
 */

#ifndef NO_SEGV_HANDLER

#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <string.h>

#define SEGV_TRACESZ 256

static volatile int segvcaught = 0;

static void segvhandler(int sig)
{
	void *trace[SEGV_TRACESZ];
	size_t n;
	if (segvcaught) {
		fprintf(stderr, "panic: recaught signal %d\naborting\n", sig);
		abort();
	}
	segvcaught = 1;
	n = backtrace(trace, SEGV_TRACESZ);
	fprintf(stderr, "Fatal error: signal %d\nBacktrace:\n", sig);
#ifndef NO_SEGV_DUMP_TRACE
	backtrace_symbols_fd(trace, n, 2); /* == STDERR_FILENO */
#else
	char **list = backtrace_symbols(trace, SEGV_TRACESZ);
	char buf[4096];
	snprintf(
		buf, sizeof buf,
		"The application has crashed. Copy this text *exactly* if you want to report a bug.\n"
		"Debug info: got signal %d\nBacktrace:\n", sig
	);
	for (char *out = buf + strlen(buf), **trace = list; *trace && n; ++trace, --n) {
		int d = snprintf(out, (sizeof buf) - (out - buf), "%s\n", *trace);
		if (!d) break;
		out += d;
	}
	puts(buf);
#endif
	exit(sig & 127);
}

#endif

void segvhandler_init(void)
{
#ifndef NO_SEGV_HANDLER
	signal(SIGSEGV, segvhandler);
#endif
}

#ifdef SEGV_DEMO

int main(void)
{
	int *bogus;
	segvhandler_init();
	puts("still running");
	bogus = 0;
	puts("now trying to trigger crash handler...");
	*bogus = 0xdeadface;
	puts("hmmm, this is really bad.");
	return 255;
}

#endif
