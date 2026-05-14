/**
 * @brief free - Show free / used / total RAM
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2015-2018 K. Lange
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void) {
	printf(
			"usage: free [-utk]\n"
			"\n"
			" -u     \033[3mshow used instead of free\033[0m\n"
			" -t     \033[3minclude a total\033[0m\n"
			" -k     \033[3muse kilobytes instead of megabytes\033[0m\n"
			"\n");
	exit(1);
}

int main(int argc, char * argv[]) {
	int show_used = 0;
	int use_kilobytes = 0;
	int show_total = 0;

	int c;
	while ((c = getopt(argc, argv, "utk")) != -1) {
		switch (c) {
			case 'u':
				show_used = 1;
				break;
			case 't':
				show_total = 1;
				break;
			case 'k':
				use_kilobytes = 1;
				break;
			default:
				usage();
		}
	}

	const char * unit = "kB";

	FILE * f = fopen("/proc/meminfo", "r");
	if (f == NULL) {
		perror("/proc/meminfo");
		return 1;
	}
	int total, free, used;
	char buf[1024] = {0};
	fgets(buf, 1024, f);
	char * a, * b;

	a = strchr(buf, ' ');
	a++;
	b = strchr(a, '\n');
	*b = '\0';
	total = atoi(a);

	fgets(buf, 1024, f);
	a = strchr(buf, ' ');
	a++;
	b = strchr(a, '\n');
	*b = '\0';
	free = atoi(a);

	fclose(f);
	used = total - free;
	assert(used > 0);

	if (!use_kilobytes) {
		unit = "MB";
		free /= 1024;
		used /= 1024;
		total /= 1024;
	}

	if (show_used) {
		printf("%d %s", used, unit);
	} else {
		printf("%d %s", free, unit);
	}

	if (show_total) {
		printf(" / %d %s", total, unit);
	}

	printf("\n");

	return 0;
}

