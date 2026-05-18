/**
 * @brief cp - Copy files
 *
 * @copyright
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2018 K. Lange
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/stat.h>

#define CHUNK_SIZE 4096

#ifndef IS_MV
#define APP_NAME "cp"
static int recursive = 0;
#endif
static int symlinks = 0;
static int copy_thing(char * tmp, char * tmp2);

static int copy_link(char * source, char * dest, int mode, int uid, int gid) {
	//fprintf(stderr, "need to copy link %s to %s\n", source, dest);
	char tmp[1024];
	if (readlink(source, tmp, sizeof(tmp)) == -1)
		return 1;
	if (symlink(tmp, dest) == -1)
		return 1;
	if (chmod(dest, mode) == -1)
		return 1;
	return 0;
}

static int copy_file(char * source, char * dest, int mode,int uid, int gid) {
	//fprintf(stderr, "need to copy file %s to %s %x\n", source, dest, mode);

	int s_fd = open(source, O_RDONLY);
	if (s_fd == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", source, strerror(errno));
		return 1;
	}
	int d_fd = open(dest, O_WRONLY | O_CREAT, mode);
	if (d_fd == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", dest, strerror(errno));
		return 1;
	}
	ssize_t length = lseek(s_fd, 0, SEEK_END);
	if (length == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", source, strerror(errno));
		return 1;
	}
	if (lseek(s_fd, 0, SEEK_SET) == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", source, strerror(errno));
		return 1;
	}

	//fprintf(stderr, "%d bytes to copy\n", length);

	char buf[CHUNK_SIZE];

	while (length > 0) {
		ssize_t r = read(s_fd, buf, CHUNK_SIZE);
		if (r == -1) {
			fprintf(stderr, APP_NAME ": %s: %s\n", source, strerror(errno));
			return 1;
		}
		//fprintf(stderr, "copying %d bytes from %s to %s\n", r, source, dest);
		ssize_t w = write(d_fd, buf, r);
		if (w == -1) {
			fprintf(stderr, APP_NAME ": %s: %s\n", dest, strerror(errno));
			return 1;
		}
		if (w < r) {
			fprintf(stderr, APP_NAME ": %s: short write\n", dest);
			return 1;
		}
		length -= r; /* Actually should be -w, but let's not get into that now... this should probably use stdio anyway */
		//fprintf(stderr, "%d bytes remaining\n", length);
	}

	close(s_fd);
	close(d_fd);
	return 0;
}

static int copy_directory(char * source, char * dest, int mode, int uid, int gid) {
	DIR * dirp = opendir(source);
	if (dirp == NULL) {
		fprintf(stderr, "Failed to copy directory %s\n", source);
		return 1;
	}

	//fprintf(stderr, "Creating %s\n", dest);
	if (!strcmp(dest, "/")) {
		dest = "";
	} else {
		mkdir(dest, mode);
	}

	int ret = 0;

	struct dirent * ent = readdir(dirp);
	while (ent != NULL) {
		if (!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..")) {
			//fprintf(stderr, "Skipping %s\n", ent->d_name);
			ent = readdir(dirp);
			continue;
		}
		//fprintf(stderr, "not skipping %s/%s → %s/%s\n", source, ent->d_name, dest, ent->d_name);
		char tmp[strlen(source)+strlen(ent->d_name)+2];
		sprintf(tmp, "%s/%s", source, ent->d_name);
		char tmp2[strlen(dest)+strlen(ent->d_name)+2];
		sprintf(tmp2, "%s/%s", dest, ent->d_name);
		//fprintf(stderr,"%s → %s\n", tmp, tmp2);
		ret |= copy_thing(tmp,tmp2);
		ent = readdir(dirp);
	}
	closedir(dirp);
	return ret;
}

static int copy_thing(char * tmp, char * tmp2) {
	struct stat statbuf;
	int ret = symlinks ? lstat(tmp, &statbuf) : stat(tmp, &statbuf);
	if (ret == -1) {
		fprintf(stderr, APP_NAME ": %s: %s\n", tmp, strerror(errno));
		return 1;
	}
	if (S_ISLNK(statbuf.st_mode)) {
		return copy_link(tmp, tmp2, statbuf.st_mode & 07777, statbuf.st_uid, statbuf.st_gid);
	} else if (S_ISDIR(statbuf.st_mode)) {
		if (!recursive) {
			fprintf(stderr, APP_NAME ": %s: omitting directory\n", tmp);
			return 1;
		}
		return copy_directory(tmp, tmp2, statbuf.st_mode & 07777, statbuf.st_uid, statbuf.st_gid);
	} else if (S_ISREG(statbuf.st_mode)) {
		return copy_file(tmp, tmp2, statbuf.st_mode & 07777, statbuf.st_uid, statbuf.st_gid);
	} else {
		fprintf(stderr, APP_NAME ": %s is not any of the required file types?\n", tmp);
		return 1;
	}
}

#ifndef IS_MV
static int copy_top_level(char **argv, int argc, int optind) {
	char * destination = argv[argc-1];
	destination = *destination ? destination : ".";

	int ret = 0;

	struct stat statbuf;
	int r = stat(destination, &statbuf);
	if (r == 0 && S_ISDIR(statbuf.st_mode)) {
		while (optind < argc - 1) {
			char * source = strrchr(argv[optind], '/');
			if (!source) source = argv[optind];
			char output[4096];
			sprintf(output, "%s/%s", destination, source);
			ret |= copy_thing(argv[optind], output);
			optind++;
		}
	} else {
		if (optind < argc - 2) {
			fprintf(stderr, APP_NAME ": target '%s' is not a directory\n", destination);
			return 1;
		}
		ret |= copy_thing(argv[optind], destination);
	}

	return ret;
}

static void usage(void) {
	fprintf(stderr, "usage: cp [-RrP] source target\n"
		"       cp [-RrP] source ... directory\n");
	exit(1);
}

int main(int argc, char ** argv) {

	int opt;
	while ((opt = getopt(argc, argv, "RrP")) != -1) {
		switch (opt) {
			case 'R':
			case 'r':
				recursive = 1;
				symlinks = 1;
				break;
			case 'P':
				symlinks = 0;
				break;
			default:
				usage();
		}
	}

	if (optind < argc - 1) {
		return copy_top_level(argv, argc, optind);
	} else {
		fprintf(stderr, "cp: not enough arguments\n");
		usage();
	}

	return 0;
}
#endif
