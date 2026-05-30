/**
 * @brief Create directories
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2013-2014 K. Lange
 */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

int makedir(const char * dir, int mask, int parents) {
	if (!parents) return mkdir(dir,mask);

	char * tmp = strdup(dir);
	if (tmp == NULL)
		err(1, "strdup");
	char * c = tmp;
	while ((c = strchr(c+1,'/'))) {
		*c = '\0';
		if (mkdir(tmp, mask) == -1) {
			if (errno != EEXIST)
				return -1;
		}
		*c = '/';
	}
	int ret = mkdir(tmp, mask);
	free(tmp);
	return ret;
}

int main(int argc, char ** argv) {
	int retval = 0;
	int parents = 0;
	int opt;

	while ((opt = getopt(argc, argv, "p")) != -1) {
		switch (opt) {
			case 'p':
				parents = 1;
				break;
			default:
				return 1;
		}
	}
	if (optind == argc)
		errx(1, "missing argument");

	for (int i = optind; i < argc; ++i) {
		if (makedir(argv[i], 0777, parents) == -1) {
			if (parents && errno == EEXIST) continue;
			warn("%s", argv[i]);
			retval = 1;
		}
	}

	return retval;
}
