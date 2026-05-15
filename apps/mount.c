/**
 * @brief Mount filesystems into the VFS
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2014-2018 K. Lange
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

int main(int argc, char ** argv) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s type device mountpoint\n", argv[0]);
		return 1;
	}
	if (getuid() != 0) {
		fprintf(stderr, "%s: only root should run this\n", argv[0]);
		return 1;
	}
	int ret = mount(argv[2], argv[3], argv[1], 0, NULL);
	if (ret < 0) {
		perror(argv[2]);
		return ret;
	}

	return 0;
}
