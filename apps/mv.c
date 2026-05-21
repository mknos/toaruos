/**
 * @brief Move files
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>

#define APP_NAME "mv"
#define IS_MV

static int recursive = 1;

#include "cp.c"
#include "rm.c"

void usage(void) {
	fprintf(stderr, "usage: mv [-ifn] source_file... destination\n");
	exit(1);
}

int main(int argc, char * argv[]) {
	int opt;
	int interactive = 0;
	int force = 0;
	int noclobber = 0;

	while ((opt = getopt(argc, argv, "ifn")) != -1) {
		switch (opt) {
			case 'i':
				interactive = 1;
				force = noclobber = 0;
				break;
			case 'f':
				force = 1;
				interactive = noclobber = 0;
				break;
			case 'n':
				noclobber = 1;
				interactive = force = 0;
				break;
			default:
				usage();
		}
	}

	if (optind + 1 >= argc)
		usage();

	char * destination = argv[argc-1];

	int target_is_dir = 0;
	struct stat statbuf;
	int exists = 0;
	if ((exists = !stat(destination, &statbuf))) {
		if (S_ISDIR(statbuf.st_mode)) {
			target_is_dir = 1;
		}
	}

	int destination_has_trailing_slash = strlen(destination) && destination[strlen(destination)-1] == '/';
	int multiple_args = optind + 2 < argc;

	if ((multiple_args && !target_is_dir) ||
	    (exists && !target_is_dir && destination_has_trailing_slash)) {
		fprintf(stderr, "%s: %s: Not a directory\n", argv[0], destination);
		return 1;
	}

	int ret = 0;

	for (int i = optind; i < argc - 1; ++i) {
		char * target = destination;
		int skip = 0;
		struct stat src_stat;
		if (lstat(argv[i], &src_stat) == -1) {
			fprintf(stderr, "%s: failed to stat '%s': %s\n", argv[0], argv[i], strerror(errno));
			ret = skip = 1;
		}
		if (!skip && target_is_dir) {
			char * tmp = strdup(argv[i]);
			if (tmp == NULL)
				err(1, "strdup");
			char * bn = basename(tmp);
			if (asprintf(&target, "%s%s%s", destination, destination_has_trailing_slash ? "" : "/", bn) == -1)
				err(1, "asprintf");
			free(tmp);
		}
		if (!skip && !force && stat(target, &statbuf) == 0) {
			if (interactive) { /* || (isatty(STDIN_FILENO) && some_check_for_writability...) */
				fprintf(stderr, "%s: overwrite '%s'? ", argv[0], target);
				fflush(stderr); /* just in case */
				char tmp[10] = {0};
				fgets(tmp, 10, stdin);
				if (tmp[0] != 'y' && tmp[0] != 'Y')
					ret = skip = 1;
			} else if (noclobber)
				skip = 1; // successfully do nothing
		}
		if (!skip && rename(argv[i], target) == -1) {
			if (errno != EXDEV && errno != ENOTSUP) {
				fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
				ret = 1;
			} else if (copy_thing(argv[i], target) || rm_thing(argv[i])) {
				fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
				ret = 1;
			}
		}
		if (target != destination)
			free(target);
	}
	return ret;
}
