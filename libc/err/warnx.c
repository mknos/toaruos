#include <stdio.h>
#include <va_list.h>

extern char *_argv_0;

void warnx(const char *fmt, ...) {
	char *s;
	va_list args;
	va_start(args, fmt);
	(void)vasprintf(&s, fmt, args);
	va_end(args);
	fputs(_argv_0, stderr);
	fputs(": ", stderr);
	fputs(s, stderr);
	fputc('\n', stderr);
}
