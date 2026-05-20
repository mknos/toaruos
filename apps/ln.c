/**
 * @brief Make symlinks
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2015 Mike Gerow
 *               2018 K. Lange
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static const char usage[] =
"Usage: %s [-s] TARGET NAME\n"
"    -s: Create a symbolic link.\n";

int main(int argc, char * argv[]) {
	int symlink_flag = 0;

	int c;
	while ((c = getopt(argc, argv, "s")) != -1) {
		switch (c) {
			case 's':
				symlink_flag = 1;
				break;
			default:
				fprintf(stderr, usage, argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if (argc - optind < 2) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	char * target = argv[optind];
	char * name = argv[optind + 1];

	int ret = symlink_flag ? symlink(target, name) : link(target, name);
	if (ret == -1) {
		fprintf(stderr, "%s: %s: %s\n", argv[0], name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
