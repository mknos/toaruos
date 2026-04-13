#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

char * mktemp(char * template) {
	size_t len = strlen(template);
	char * suffix = template + len - 6;
	if (len < 6 || strcmp(suffix, "XXXXXX") != 0) {
		memset(template, 0, len);
		errno = EINVAL;
		return template;
	}
	static int _i = 0;
	int pid = (int)getpid() % 10000;
	snprintf(suffix, 7, "%04d%02d", pid, _i);
	_i = (_i + 1)  % 100;
	return template;
}

int mkstemp(char * template) {
	mktemp(template);
	return open(template, O_RDWR | O_CREAT, 0600);
}
