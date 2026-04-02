/**
 * @brief Examine symlinks and print the path they point to.
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2015 Mike Gerow
 */
#include <stdio.h>
#include <unistd.h>

#define MAX_LINK_SIZE 4096

int main(int argc, char * argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s LINK\n", argv[0]);
		return 1;
	}
	char * name = argv[1];
	char buf[MAX_LINK_SIZE];
	if (readlink(name, buf, sizeof(buf)) == -1)
		return 1;
	fprintf(stdout, "%s\n", buf);
	return 0;
}
