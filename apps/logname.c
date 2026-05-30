/**
 * @brief Display the user's name, as returned by getlogin()
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void usage(void) {
	fprintf(stderr, "usage: logname\n");
	exit(1);
}

int main(int argc, char ** argv) {
	int opt;
	while ((opt = getopt(argc, argv, "")) != -1)
		usage();
	if (optind < argc) {
		fprintf(stderr, "%s: extra operand: '%s'\n", argv[0], argv[optind]);
		usage();
	}
	char * name = getlogin();
	if (!name) {
		fprintf(stderr, "%s: failed to determine login name\n", argv[0]);
		return 1;
	}
	fprintf(stdout, "%s\n", name);
	return 0;
}
