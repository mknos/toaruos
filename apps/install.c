/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
static char copyright[] =
"@(#) Copyright (c) 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";

static char sccsid[] = "@(#)xinstall.c	8.1.1 (2.11BSD) 1996/2/21";
#endif

#include <sys/wait.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _PATH_DEVNULL "/dev/null"

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#define MAXBSIZE 4096

struct passwd *pp;
struct group *gp;
int mode = S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
char *group, *owner, pathbuf[PATH_MAX];

#define	DIRECTORY	0x01		/* Tell install it's a directory. */

void	copy(int, char *, int, char *);
void	install(char *, char *, uint16_t, uint32_t);
void	usage(void);

int
main(int argc, char *argv[])
{
	struct stat from_sb, to_sb;
	uint16_t fset = 0;
	int ch, no_target;
	char *to_name;

	while ((ch = getopt(argc, argv, "cg:m:o:s")) != EOF)
		switch((char)ch) {
		case 'c':
		case 's':
			break;
		case 'g':
			group = optarg;
			break;
		case 'm':
			errno = 0;
			mode = (mode_t)strtoul(optarg, NULL, 8);
			if (errno) {
				fprintf(stderr, "bad mode: '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'o':
			owner = optarg;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

#if 0
	/* get group and owner id's */
	if (group && !(gp = getgrnam(group)))
		err("unknown group %s", group);
	if (owner && !(pp = getpwnam(owner)))
		err("unknown user %s", owner);
#endif

	no_target = stat(to_name = argv[argc - 1], &to_sb);
	if (!no_target && (to_sb.st_mode & S_IFMT) == S_IFDIR) {
		for (; *argv != to_name; ++argv)
			install(*argv, to_name, fset, DIRECTORY);
		exit(0);
	}

	/* can't do file1 file2 directory/file */
	if (argc != 2)
		usage();

	if (!no_target) {
		if (stat(*argv, &from_sb)) {
			fprintf(stderr, "%s: %s\n", *argv, strerror(errno));
			exit(1);
		}
		if ((to_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "%s: not a regular file\n", to_name);
			exit(1);
		}
		if (to_sb.st_dev == from_sb.st_dev &&
		    to_sb.st_ino == from_sb.st_ino) {
			fprintf(stderr, "%s and %s are the same file\n", *argv, to_name);
			exit(1);
		}
		/*
		 * Unlink now... avoid ETXTBSY errors later.  Try and turn
		 * off the append/immutable bits -- if we fail, go ahead,
		 * it might work.
		 */
		(void)unlink(to_name);
	}
	install(*argv, to_name, fset, 0);
	exit(0);
}

/*
 * install --
 *	build a path name and install the file
 */
void
install(char *from_name, char *to_name, uint16_t fset, uint32_t flags)
{
	struct stat from_sb;
	int devnull, from_fd, to_fd, serrno;
	char *p;

	/* If try to install NULL file to a directory, fails. */
	if (flags & DIRECTORY || strcmp(from_name, _PATH_DEVNULL)) {
		if (stat(from_name, &from_sb)) {
			fprintf(stderr, "%s: %s\n", from_name, strerror(errno));
			exit(1);
		}
		if ((from_sb.st_mode & S_IFMT) != S_IFREG) {
			fprintf(stderr, "%s: not a regular file\n", from_name);
			exit(1);
		}
		/* Build the target path. */
		if (flags & DIRECTORY) {
			(void)sprintf(pathbuf, "%s/%s", to_name,
			    (p = strrchr(from_name, '/')) ? ++p : from_name);
			to_name = pathbuf;
		}
		devnull = 0;
	} else {
		devnull = 1;
	}

	/*
	 * Unlink now... avoid ETXTBSY errors later.  Try and turn
	 * off the append/immutable bits -- if we fail, go ahead,
	 * it might work.
	 */
	(void)unlink(to_name);

	/* Create target. */
	if ((to_fd = open(to_name,
	    O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		fprintf(stderr, "%s: %s\n", to_name, strerror(errno));
		exit(1);
	}
	if (!devnull) {
		if ((from_fd = open(from_name, O_RDONLY, 0)) < 0) {
			(void)unlink(to_name);
			fprintf(stderr, "%s: %s\n", from_name, strerror(errno));
			exit(1);
		}
		copy(from_fd, from_name, to_fd, to_name);
		(void)close(from_fd);
	}

	/*
	 * Set owner, group, mode for target; do the chown first,
	 * chown may lose the setuid bits.
	 */
#if 0
	if ((group || owner) &&
	    fchown(to_fd, owner ? pp->pw_uid : -1, group ? gp->gr_gid : -1)) {
		serrno = errno;
		(void)unlink(to_name);
		err("%s: chown/chgrp: %s", to_name, strerror(serrno));
	}
#endif
	if (fchmod(to_fd, mode)) {
		serrno = errno;
		(void)unlink(to_name);
		fprintf(stderr, "%s: chmod: %s\n", to_name, strerror(serrno));
		exit(1);
	}

	(void)close(to_fd);
}

/*
 * copy --
 *	copy from one file to another
 */
void
copy(int from_fd, char *from_name, int to_fd, char *to_name)
{
	int nr, nw;
	int serrno;
	char buf[MAXBSIZE];

	while ((nr = read(from_fd, buf, sizeof(buf))) > 0)
		if ((nw = write(to_fd, buf, nr)) != nr) {
			serrno = errno;
			(void)unlink(to_name);
			fprintf(stderr, "%s: %s\n",
			    to_name, strerror(nw > 0 ? EIO : serrno));
			exit(1);
		}
	if (nr != 0) {
		serrno = errno;
		(void)unlink(to_name);
		fprintf(stderr, "%s: %s\n", from_name, strerror(serrno));
		exit(1);
	}
}

/*
 * usage --
 *	print a usage message and die
 */
void
usage(void)
{
	(void)fprintf(stderr,
"usage: install [-cs] [-g group] [-m mode] [-o owner] file1 file2;\n\tor file1 ... fileN directory\n");
	exit(1);
}
