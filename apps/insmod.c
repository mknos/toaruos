/**
 * @brief insmod - Load kernel module
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2016 K. Lange
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysfunc.h>

int main(int argc, char * argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <modulepath>\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		fprintf(stderr, "%s: extra operand '%s'\n", argv[0], argv[2]);
		return 1;
	}
	if (getuid() != 0) {
		fprintf(stderr, "%s: only root can do that\n", argv[0]);
		return 1;
	}
	int status = sysfunc(TOARU_SYS_FUNC_INSMOD, &argv[1]);
	if (status != 0) {
		fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
		return status;
	}
	return 0;
}
