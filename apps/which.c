/**
 * @brief which - Figure out which binary will be used
 *
 * Searches through $PATH to find a matching binary, just like
 * how execp* family does it. (Except does our execp actually
 * bother checking permissions? Look into this...)
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

#define DEFAULT_PATH "/bin:/usr/bin"

#define EXIT_MATCH 0
#define EXIT_NOMATCH 1
#define EXIT_ERROR 2

int main(int argc, char * argv[]) {

	int ret_val = EXIT_MATCH;
	int i = 1;
	int print_all = 0;

	if (i < argc && !strcmp(argv[i],"-a")) {
		print_all = 1;
		i++;
	}

	if (i == argc)
		return EXIT_NOMATCH;

	char * path = getenv("PATH");
	if (!path)
		path = DEFAULT_PATH;
	char * xpath = strdup(path);
	if (xpath == NULL)
		err(EXIT_ERROR, "strdup");
	for (; i < argc; ++i) {
		if (strchr(argv[i], '/')) {
			if (access(argv[i], X_OK) == 0)
				printf("%s\n", argv[i]);
		} else {
			char * p, * last;
			int found = 0;
			for ((p = strtok_r(xpath, ":", &last)); p; p = strtok_r(NULL, ":", &last)) {
				char * exe;
				if (asprintf(&exe, "%s/%s", p, argv[i]) == -1)
					err(EXIT_ERROR, "asprintf");
				if (access(exe, X_OK) == 0) {
					found = 1;
					printf("%s\n", exe);
					if (!print_all) {
						free(exe);
						break;
					}
				}
				free(exe);
			}
			if (!found)
				ret_val = EXIT_NOMATCH;
		}
	}
	free(xpath);
	return ret_val;
}
