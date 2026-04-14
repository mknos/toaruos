#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

static char _name[64]; /* NAME_MAX ? */

char * getlogin(void) {

	int tty = STDIN_FILENO;
	if (!isatty(tty)) {
		tty = STDOUT_FILENO;
		if (!isatty(tty)) {
			tty = STDERR_FILENO;
			if (!isatty(tty)) {
				errno = ENOTTY;
				return NULL;
			}
		}
	}

	char * name = ttyname(tty);
	if (!name)
		return NULL;
	struct stat statbuf;
	if (stat(name, &statbuf) == -1)
		return NULL;
	struct passwd * passwd = getpwuid(statbuf.st_uid);
	if (!passwd || !passwd->pw_name)
		return NULL;
	snprintf(_name, sizeof(_name), "%s", passwd->pw_name);
	return _name;
}
