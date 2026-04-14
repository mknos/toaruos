#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

int isatty(int fd) {
	int dtype = ioctl(fd, IOCTLDTYPE, NULL);
	if (dtype == -1) return 0;
	if (dtype == IOCTL_DTYPE_TTY) return 1;
	errno = EINVAL;
	return 0;
}
