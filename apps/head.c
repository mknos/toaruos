/**
 * @brief head - Print the first `n` lines of a file.
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

int main(int argc, char * argv[]) {
	int n = 10;
	int opt;
	int print_names = 0;
	int retval = 0;

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
				break;
			default:
				return 1;
		}
	}

	if (argc > optind + 1) {
		/* Multiple files */
		print_names = 1;
	}

	if (argc == optind) {
		/* This is silly, but should work due to reasons. */
		argv[optind] = "-";
		argc++;
	}

	for (int i = optind; i < argc; ++i) {
		int is_stdin = !strcmp(argv[i], "-");
		FILE *f = is_stdin ? stdin : fopen(argv[i], "r");
		if (!f) {
			fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
			retval = 1;
			continue;
		}

		char *fname = is_stdin ? "standard input" : argv[i];
		if (print_names)
			fprintf(stdout, "==> %s <==\n", fname);

		int line = 1;

		while (!feof(f)) {
			int c = fgetc(f);
			if (ferror(f)) {
				fprintf(stderr, "%s: read error: %s\n", fname, strerror(errno));
				retval = 1;
				break;
			}
			fputc(c, stdout);

			if (c == '\n') {
				line++;
				if (line > n) break;
			}
		}

		if (f != stdin) {
			fclose(f);
		}
	}

	return retval;
}
