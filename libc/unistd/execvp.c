#include <assert.h>
#include <stdio.h>
#include <syscall.h>
#include <syscall_nums.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

DEFN_SYSCALL3(execve, SYS_EXECVE, char *, char **, char **);

extern char ** environ;

#define DEFAULT_PATH "/bin:/usr/bin"

int execve(const char *name, char * const argv[], char * const envp[]) {
	__sets_errno(syscall_execve((char*)name,(char**)argv,(char**)envp));
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
	assert(file != NULL);
	if (strchr(file, '/') == NULL) {
		/* We don't quite understand "$PATH", so... */
		char * path = getenv("PATH");
		if (path == NULL)
			path = DEFAULT_PATH;
		char * xpath = strdup(path);
		if (xpath == NULL)
			return -1;
		char *s, * p, * last;
		for (s = xpath; ; s = NULL) {
			p = strtok_r(s, ":", &last);
			if (p == NULL)
				break;
			char * exe = NULL;
			if (asprintf(&exe, "%s/%s", p, file) == -1) {
				free(xpath);
				return -1;
			}
			struct stat stat_buf;
			int r = stat(exe, &stat_buf);
			if (r == -1 || !(stat_buf.st_mode & 0111)) {
				free(exe);
				continue; /* XXX not technically correct; need to test perms */
			}
			int ret = execve(exe, argv, envp);
			if (ret == -1) {
				free(xpath);
				free(exe);
			}
			return ret;
		}
		free(xpath);
		errno = ENOENT;
		return -1;
	}
	return execve(file, argv, envp);
}

int execvp(const char *file, char *const argv[]) {
	return execvpe(file, argv, environ);
}

int execv(const char * file, char * const argv[]) {
	return execve(file, argv, environ);
}

int execl(const char *path, const char *arg, ...) {
	int argc = 1; /* Count */
	va_list ap;

	/* Count */
	va_start(ap, arg);
	while (va_arg(ap, char *)) argc++;
	va_end(ap);

	/* Copy */
	char * argv[argc+1];
	va_start(ap, arg);
	argv[0] = (char*)arg;
	for (int i = 1; i <= argc; ++i) argv[i] = va_arg(ap, char*);
	va_end(ap);

	/* Exec */
	return execv(path, argv);
}

int execlp(const char *path, const char *arg, ...) {
	int argc = 1; /* Count */
	va_list ap;

	/* Count */
	va_start(ap, arg);
	while (va_arg(ap, char *)) argc++;
	va_end(ap);

	/* Copy */
	char * argv[argc+1];
	va_start(ap, arg);
	argv[0] = (char*)arg;
	for (int i = 1; i <= argc; ++i) argv[i] = va_arg(ap, char*);
	va_end(ap);

	/* Exec */
	return execvp(path, argv);
}

int execle(const char *path, const char *arg, ...) {
	int argc = 1; /* Count */
	va_list ap;

	/* Count */
	va_start(ap, arg);
	while (va_arg(ap, char *)) argc++;
	va_end(ap);

	/* Copy */
	char * argv[argc+1];
	va_start(ap, arg);
	argv[0] = (char*)arg;
	for (int i = 1; i <= argc; ++i) argv[i] = va_arg(ap, char*);

	char ** envp = va_arg(ap, char**);
	va_end(ap);

	/* Exec */
	return execve(path, argv, envp);
}
