#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

static char _tty_name[30]; /* only needs to hold /dev/pty/ttyXXXXXXX */

char * ttyname(int fd) {

	if (!isatty(fd)) {
		errno = ENOTTY;
		return NULL;
	}
	if (ioctl(fd, IOCTLTTYNAME, _tty_name) == -1)
		return NULL;
	return _tty_name;
}

int ttyname_r(int fd, char * buf, size_t buflen) {
	if (!isatty(fd)) return ENOTTY;
	if (buflen < 30) return ERANGE;
	if (ioctl(fd, IOCTLTTYNAME, buf) == -1)
		return errno;
	return 0;
}
