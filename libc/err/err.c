#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <va_list.h>

extern char *_argv_0;

void err(int rc, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fputs(_argv_0, stderr);
	fputs(": ", stderr);
	(void)xvasprintf(cb_fprintf, stderr, fmt, args);
	va_end(args);
	fputs(": ", stderr);
	fputs(strerror(errno), stderr);
	fputc('\n', stderr);
	exit(rc);
}
