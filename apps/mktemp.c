/**
 * @brief mktemp - create a temporary directory and print its name
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char * argv[]) {
	int opt;
	int dry_run = 0;
	int quiet = 0;
	int directory = 0;

	while ((opt = getopt(argc,argv,"duq")) != -1) {
		switch (opt) {
			case 'd':
				directory = 1;
				break;
			case 'u':
				dry_run = 1;
				break;
			case 'q':
				quiet = 1;
				break;
			default:
				fprintf(stderr, "usage: %s [-dqu] TEMPLATE\n", argv[0]);
				return 1;
		}
	}

	char * template;
	if (optind == argc)
		template = strdup("/tmp/tmp.XXXXXX");
	else
		template = strdup(argv[optind]);

	if (template == NULL)
		err(1, "strdup");
	char * result = mktemp(template);
	if (result == NULL || strlen(result) == 0)
		err(1, "template string rejected");
	if (!quiet)
		fprintf(stdout, "%s\n", result);
	if (dry_run) {
		free(template);
		return 0;
	}
	int rc = 0;
	if (directory) {
		if (mkdir(result, 0777) == -1) {
			warn("mkdir '%s'", result);
			rc = 1;
		}
	} else {
		FILE * f = fopen(result, "w");
		if (f == NULL) {
			warn("%s", result);
			rc = 1;
		}
		fclose(f);
	}
	free(template);
	return rc;
}
