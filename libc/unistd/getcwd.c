#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <syscall_nums.h>

DEFN_SYSCALL2(getcwd, SYS_GETCWD, char *, size_t);

char *getcwd(char *buf, size_t size) {
	assert(size != 0);
	int did_alloc = 0;
	if (buf == NULL) {
		buf = malloc(size);
		if (buf == NULL)
			return NULL;
		did_alloc = 1;
	}
	char *cwd = (char *)syscall_getcwd(buf, size);
	if (cwd == NULL) {
		if (did_alloc)
			free(buf);
		errno = ERANGE; // size too small
	}
	return cwd;
}
