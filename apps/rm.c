/**
 * @brief rm - Unlink files
 *
 * TODO: Support recursive, directory removal, etc.
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2013-2018 K. Lange
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#ifndef IS_MV
#define APP_NAME "rm"
static int recursive = 0;
#endif

static int rm_thing(char * tmp);

static int rm_directory(char * source) {
	DIR * dirp = opendir(source);
	if (dirp == NULL) {
		fprintf(stderr, "could not open %s\n", source);
		return 1;
	}
	struct dirent * ent;
	while ((ent = readdir(dirp)) != NULL) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		char tmp[strlen(source)+strlen(ent->d_name)+2];
		sprintf(tmp, "%s/%s", source, ent->d_name);
		int status = rm_thing(tmp);
		if (status) return status;
		rewinddir(dirp);
	}
	closedir(dirp);

	if (unlink(source) == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", source, strerror(errno));
		return 1;
	}
	return 0;
}

static int rm_thing(char * tmp) {
	struct stat statbuf;
	if (lstat(tmp, &statbuf) == -1) {
		fprintf(stderr, APP_NAME ": stat failed for '%s': %s\n", tmp, strerror(errno));
		return 1;
	}
	if (S_ISDIR(statbuf.st_mode)) {
		if (!recursive) {
			fprintf(stderr, APP_NAME ": %s: is a directory\n", tmp);
			return 1;
		}
		return rm_directory(tmp);
	} else {
		if (unlink(tmp) == -1) {
			fprintf(stderr, APP_NAME ": %s: %s\n", tmp, strerror(errno));
			return 1;
		}
		return 0;
	}
}

#ifndef IS_MV
static int rm_top_level(char **argv, int argc, int optind) {
	int ret = 0;

	for (int i = optind; i < argc; ++i) {
		ret |= rm_thing(argv[i]);
	}

	return ret;
}

int main(int argc, char * argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "fRr")) != -1) {
		switch (opt) {
			case 'R':
			case 'r':
				recursive = 1;
				break;
			case 'f':
				/* ignore */
				break;
			default:
				return 1;
		}
	}

	return rm_top_level(argv, argc, optind);
}
#endif
