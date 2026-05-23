#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <syscall.h>
#include <syscall_nums.h>
#include <errno.h>
#include <bits/dirent.h>

DEFN_SYSCALL3(readdir, SYS_READDIR, int, int, void *);

DIR * opendir (const char * dirname) {
	int fd = open(dirname, O_RDONLY|O_DIRECTORY);
	if (fd == -1)
		return NULL;
	DIR * dir = malloc(sizeof(DIR));
	if (dir == NULL)
		return NULL;
	dir->fd = fd;
	dir->cur_entry = -1;
	return dir;
}

int closedir (DIR * dir) {
	if (dir && (dir->fd != -1)) {
		return close(dir->fd);
	} else {
		return -EBADF;
	}
}

struct dirent * readdir (DIR * dirp) {
	static struct dirent ent;
	struct dirent * d = NULL;

	int ret = syscall_readdir(dirp->fd, ++dirp->cur_entry, &ent);
	switch (ret) {
	case -1:
		errno = -ret;
		/* fallthru */
	case 0:
		memset(&ent, 0, sizeof(struct dirent));
		break;
	default:
		d = &ent;
	}
	return d;
}

long telldir(DIR * dirp) {
	return (long)dirp->cur_entry;
}

void rewinddir(DIR * dirp) {
	dirp->cur_entry = -1;
}

void seekdir(DIR * dirp, long loc) {
	dirp->cur_entry = loc;
}
