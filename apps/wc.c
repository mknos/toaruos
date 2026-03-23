/**
 * @brief wc - count bytes, characters, words, lines...
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#ifdef __toaru__
#include <toaru/decodeutf8.h>
#endif

void usage(void) {
	printf("usage: wc [-clmw] [file ...]\n");
	exit(1);
}

int main(int argc, char * argv[]) {
	int show_lines = 0;
	int show_words = 0;
	int show_chars = 0;
	int show_bytes = 0;

	int opt;

	while ((opt = getopt(argc,argv,"cmlw")) != -1) {
		switch (opt) {
			case 'c':
				show_bytes = 1;
				break;
			case 'm':
				show_chars = 1;
				break;
			case 'l':
				show_lines = 1;
				break;
			case 'w':
				show_words = 1;
				break;
			default:
				usage();
		}
	}

	int retval = 0;
	int total_lines = 0;
	int total_chars = 0;
	int total_words = 0;
	int just_stdin = 0;

	if (optind == argc) {
		argv[optind] = "";
		argc++;
		just_stdin = 1;
	}

	for (int i = optind; i < argc; ++i) {
		if (!*argv[i] && !just_stdin) {
			fprintf(stderr, "%s: invalid zero-length file name\n", argv[0]);
			retval = 1;
			continue;
		}
		FILE * f = (!strcmp(argv[i], "-") || just_stdin) ? stdin : fopen(argv[i], "r");
		if (!f) {
			fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
			retval = 1;
			continue;
		}

		int lines = 0;
		int chars = 0;
		int words = 0;
		int ch;
		uint32_t c;
		int last_was_whitespace = 0;

		while (!feof(f)) {
			ch = getc(f);
			if (ch < 0) break;

			if (show_chars) {
#ifdef __toaru__
				uint32_t state;
				if (!decode(&state, &c, ch)) {
				} else if (state == UTF8_REJECT) {
					state = 0;
				}
#else
				fprintf(stderr, "option -m not implemented\n");
				return 1;
#endif
			} else {
				c = ch;
			}

			chars++;
			if (c == '\n') {
				if (!last_was_whitespace) words++;
				last_was_whitespace = 1;
				lines++;
			} else if (c == ' ') {
				if (last_was_whitespace) continue;
				last_was_whitespace = 1;
				words++;
			} else {
				last_was_whitespace = 0;
			}
		}

		if (!last_was_whitespace && chars > 0) words++;

		if (!show_words && !show_chars && !show_bytes && !show_lines) {
			fprintf(stdout, "%d %d %d %s\n", lines, words, chars, argv[i]);
		} else {
			if (show_lines) fprintf(stdout, "%d ", lines);
			if (show_words) fprintf(stdout, "%d ", words);
			if (show_bytes|show_chars) fprintf(stdout, "%d ", chars);
			fprintf(stdout, "%s\n", argv[i]);
		}

		total_lines += lines;
		total_words += words;
		total_chars += chars;

		if (f != stdin) fclose(f);
		if (just_stdin) return 0;
	}

	if (optind + 1 < argc) {
		if (!show_words && !show_chars && !show_bytes && !show_lines) {
			fprintf(stdout, "%d %d %d %s\n", total_lines, total_words, total_chars, "total");
		} else {
			if (show_lines) fprintf(stdout, "%d ", total_lines);
			if (show_words) fprintf(stdout, "%d ", total_words);
			if (show_bytes|show_chars) fprintf(stdout, "%d ", total_chars);
			fprintf(stdout, "%s\n", "total");
		}
	}

	return retval;
}

