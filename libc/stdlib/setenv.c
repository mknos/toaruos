#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int setenv(const char *name, const char *value, int overwrite) {
	if (!name || !*name) {
		errno = EINVAL;
		return -1;
	}
	if (!overwrite && getenv(name))
		return 0;
	if (strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}
	char * tmp;
	if (asprintf(&tmp, "%s=%s", name, value) == -1)
		return -1;
	return putenv(tmp);
}
