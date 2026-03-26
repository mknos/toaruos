/**
 * @brief Sort input lines.
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <toaru/list.h>

int compare0(const void * arg1, const void * arg2) {
	const char *a = *(const char **) arg1;
	const char *b = *(const char **) arg2;
	while (1) {
		while (*a == *b || tolower(*a) == tolower(*b)) {
			if (!*a) return 0;
			a++;
			b++;
		}
		while (*a && !isalnum(*a)) a++;
		while (*b && !isalnum(*b)) b++;
		if (tolower(*a) == tolower(*b)) continue;
		return (tolower(*a) < tolower(*b)) ? -1 : 1;
	}
}

int compare(const char * a, const char * b) {
	while (1) {
		while (*a == *b || tolower(*a) == tolower(*b)) {
			if (!*a) return 0;
			a++;
			b++;
		}
		while (*a && !isalnum(*a)) a++;
		while (*b && !isalnum(*b)) b++;
		if (tolower(*a) == tolower(*b)) continue;
		return (tolower(*a) < tolower(*b)) ? -1 : 1;
	}
}

int numcompare(const char * a, const char * b) {
	int64_t x, y;

	while (1) {
		// skip space
		while (isspace((unsigned char) *a)) a++;
		if (isdigit((unsigned char) *a) || *a == '-')
			x = atoi(a);
		else
			x = 0;

		while (isspace((unsigned char) *b)) b++;
		if (isdigit((unsigned char) *b) || *b == '-')
			y = atoi(b);
		else
			y = 0;

		if (x == y) return 0;
		if (x > y) return 1;
		return -1;
	}
}

void usage(void) {
	fprintf(stderr, "usage: sort [-nqr] [file ...]\n");
	exit(1);
}

int main(int argc, char * argv[]) {
	int quicksort = 0;
	int reverse = 0;
	int numeric = 0;
	int opt;

	list_t * lines = list_create();
	list_t * files = list_create();

	while ((opt = getopt(argc, argv, "nqr")) != -1) {
		switch (opt) {
			case 'n':
				numeric = 1;
				break;
			case 'q':
				quicksort = 1;
				break;
			case 'r':
				reverse = 1;
				break;
			default:
				usage();
		}
	}

	if (optind == argc) {
		/* No arguments */
		list_insert(files, stdin);
	} else {
		while (optind < argc) {
			FILE * f = fopen(argv[optind], "r");
			if (!f) {
				fprintf(stderr, "%s: %s: %s\n", argv[0], argv[optind], strerror(errno));
			} else {
				list_insert(files, f);
			}
			optind++;
		}
	}

	char line_buf[4096] = {0};
	foreach (node, files) {
		FILE * f = node->value;
		while (!feof(f)) {
			if (!fgets(line_buf, sizeof(line_buf), f))
				break;
			if (!strchr(line_buf,'\n')) {
				fprintf(stderr, "%s: oversized line\n", argv[0]);
			}
			char * line = strdup(line_buf);
			if (line == NULL) {
				perror("sort: strdup");
				exit(1);
			}
			if (quicksort) {
				list_insert(lines, line);
				continue;
			}

			node_t * next = NULL;
			foreach (lnode, lines) {
				char * cmp = lnode->value;
				int match;
				if (numeric) {
					if (reverse)
						match = numcompare(cmp, line);
					else
						match = numcompare(line, cmp);
				} else {
					if (reverse)
						match = compare(cmp, line);
					else
						match = compare(line, cmp);
				}
				if (match < 0) {
					next = lnode;
					break;
				}
			}
			if (next) {
				list_insert_before(lines, next, line);
			} else {
				list_insert(lines, line);
			}
		}
	}

	if (quicksort) {
		char **ln = calloc(lines->length, sizeof(char *));
		if (ln == NULL) {
			perror("sort: calloc");
			exit(1);
		}
		size_t i = 0;
		foreach (lnode, lines) {
			ln[i] = lnode->value;
			i++;
		}
		qsort(ln, lines->length, sizeof(char *), &compare0);
		for (i = 0; i < lines->length; i++)
			fputs(ln[i], stdout);
		free(ln);
		return 0;
	}

	foreach (lnode, lines) {
		char * line = lnode->value;
		fprintf(stdout, "%s", line);
	}
	return 0;
}
