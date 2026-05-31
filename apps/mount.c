/**
 * @brief Mount filesystems into the VFS
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2014-2018 K. Lange
 */

#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

int main(int argc, char ** argv) {
	if (argc > 4) {
		warnx("extra operand: '%s'", argv[4]);
		goto usage;
	} else if (argc != 4)
		goto usage;
	if (getuid() != 0)
		errx(1, "only root should run this");
	if (mount(argv[2], argv[3], argv[1], 0, NULL) == -1)
		err(1, "%s", argv[2]);
	return 0;
usage:
	fprintf(stderr, "Usage: %s type device mountpoint\n", argv[0]);
	return 1;
}
