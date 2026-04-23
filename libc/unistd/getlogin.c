#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

static char _name[64]; /* NAME_MAX ? */

char * getlogin(void) {
	int fds[] = {
		STDIN_FILENO,
		STDOUT_FILENO,
		STDERR_FILENO,
		-1
	};
	int tty, i;
	for (i = 0; fds[i] != -1; i++) {
		tty = fds[i];
		if (isatty(tty))
			break;
	}
	if (fds[i] == -1) {
		errno = ENOTTY;
		return NULL;
	}
	struct stat statbuf;
	if (fstat(tty, &statbuf) == -1)
		return NULL;
	struct passwd * passwd = getpwuid(statbuf.st_uid);
	if (!passwd || !passwd->pw_name)
		return NULL;
	snprintf(_name, sizeof(_name), "%s", passwd->pw_name);
	return _name;
}
