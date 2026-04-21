#include <assert.h>
#include <errno.h>
#include <syscall.h>
#include <syscall_nums.h>
#include <sys/stat.h>
#include <string.h>

DEFN_SYSCALL2(statf, SYS_STATF, char *, void *);
DEFN_SYSCALL2(lstat, SYS_LSTAT, char *, void *);

int stat(const char *file, struct stat *st){
	int ret = syscall_statf((char *)file, (void *)st);
	assert(ret <= 0);
	if (ret != 0) {
		errno = -ret;
		memset(st, 0, sizeof(struct stat));
		return -1;
	}
	return 0;
}

int lstat(const char *path, struct stat *st) {
	int ret = syscall_lstat((char *)path, (void *)st);
	assert(ret <= 0);
	if (ret != 0) {
		errno = -ret;
		memset(st, 0, sizeof(struct stat));
		return -1;
	}
	return 0;
}
