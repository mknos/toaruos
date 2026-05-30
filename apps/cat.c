/**
 * @brief cat - Concatenate files
 *
 * Concatenates files together to standard output.
 * In a supporting terminal, you can then pipe
 * standard out to another file or other useful
 * things like that.
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2013-2018 K. Lange
 */
#include <err.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE 4096

static char * _file;

int doit(int fd) {
	char buf[CHUNK_SIZE];
	ssize_t r;

	while ((r = read(fd, buf, sizeof(buf))) > 0)
		write(STDOUT_FILENO, buf, r);
	if (r == -1) {
		warn("%s: write error", _file);
		return 1;
	}
	return 0;
}

int main(int argc, char ** argv) {
	int ret = 0;

	if (argc == 1) {
		_file = "stdin";
		return doit(0);
	}

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i],"-")) {
			_file = "stdin";
			doit(0);
			continue;
		}
		_file = argv[i];
		int fd = open(argv[i], O_RDONLY);
		if (fd == -1) {
			warn("%s", argv[i]);
			ret = 1;
			continue;
		}

		ret |= doit(fd);
		close(fd);
	}
	return ret;
}
