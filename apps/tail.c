/*
 * Copyright (c) 2021 Juhani 'nortti' Krekelä.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * tail.c
 * Output the trailing part of files.
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

#ifdef HEAD
#define TAIL false
#else
#define TAIL true
#endif

static bool byte_mode = false;
static bool beginning_relative = !TAIL;
static off_t file_offset = 10;
static size_t buffer_size = 10;
static bool verbose = false;
static bool quiet = false;
static bool follow = false;
static bool path_mode = false;

static void xputchar(int c)
{
	if ( putchar(c) < 0 )
		perror("stdout"), exit(1);
}

static void xputstr(const char* str)
{
	if ( fputs(str, stdout) < 0 )
		perror("stdout"), exit(1);
}

static int from_beginning(FILE* fp, const char* filename)
{
	int failure = 0;

	off_t offset = 0;
	while ( true )
	{
		offset++;

		if ( !TAIL && file_offset < offset )
			break;

		char* line = NULL;
		size_t size = 0;
		int c = 0;
		ssize_t length = 0;
		if ( byte_mode )
			c = fgetc(fp);
		else
			length = getline(&line, &size, fp);
		if ( c < 0 || length < 0 )
		{
			if ( !feof(fp) )
			{
				perror(filename);
				failure = 1;
			}
			if ( !byte_mode )
				free(line);
			break;
		}

		if ( (TAIL && file_offset <= offset) ||
		     (!TAIL && offset <= file_offset) )
		{
			if ( byte_mode )
				xputchar(c);
			else
				xputstr(line);
		}

		if ( !byte_mode )
			free(line);
	}

	return failure;
}

static int from_end(FILE* fp, const char* filename)
{
	int failure = 0;

	void* buffer = calloc(buffer_size, byte_mode ? 1 : sizeof(char*));
	if ( !buffer )
		perror("calloc"), exit(1);
	char** lines = !byte_mode ? buffer : NULL;
	unsigned char* bytes = byte_mode ? buffer : NULL;
	size_t buffer_used = 0;

	size_t offset = 0;
	while ( true )
	{
		offset++;

		char* line = NULL;
		size_t size = 0;
		int c = 0;
		ssize_t length = 0;
		if ( byte_mode )
			c = fgetc(fp);
		else
			length = getline(&line, &size, fp);
		if ( c < 0 || length < 0 )
		{
			if ( !feof(fp) )
			{
				perror(filename);
				failure = 1;
			}
			free(line);
			break;
		}

		if ( TAIL )
		{
			if ( buffer_size == 0 )
			{
				free(line);
				break;
			}

			size_t index = offset % buffer_size;
			if ( buffer_used < buffer_size )
				buffer_used++;
			else if ( !byte_mode )
				free(lines[index]);

			if ( byte_mode )
				bytes[index] = c;
			else
				lines[index] = line;
		}
		else
		{
			if ( buffer_size == 0 )
			{
				if ( byte_mode )
					xputchar(c);
				else
					xputstr(line);
				free(line);
				continue;
			}

			size_t index = offset % buffer_size;
			if ( buffer_used < buffer_size )
				buffer_used++;
			else
			{
				if ( byte_mode )
					xputchar(bytes[index]);
				else
				{
					xputstr(lines[index]);
					free(lines[index]);
				}
			}

			if ( byte_mode )
				bytes[index] = c;
			else
				lines[index] = line;
		}
	}

	if ( TAIL )
	{
		for ( size_t i = 0; i < buffer_used; i++ )
		{
			size_t index = (offset - buffer_used + i) % buffer_size;
			if ( byte_mode )
				xputchar(bytes[index]);
			else
				xputstr(lines[index]);
		}
	}

	if ( !byte_mode )
	{
		for ( size_t i = 0; i < buffer_used; i++ )
		{
			size_t index = (offset - buffer_used + i) % buffer_size;
			free(lines[index]);
		}
	}
	free(buffer);

	return failure;
}

static int process_file(FILE* fp, const char* filename)
{
	int failure = beginning_relative ? from_beginning(fp, filename) :
	                                   from_end(fp, filename);

	if ( fflush(stdout) != 0 )
		perror("fflush"), exit(1);

	return failure;
}

noreturn static void follow_file(FILE* fp, const char* filename)
{
	long previous_size = ftell(fp);
	if ( previous_size == -1 && errno != ESPIPE )
		perror("ftello");
	int reopen_errno = 0;
	while ( true )
	{
		// Currently we have nothing like kqueue or inotify, so keep polling.
		// This is used also by e.g. 2.9BSD tail(1) and GNU tail(1) as fallback.
		sleep(1);

		struct stat st;
		if ( fstat(fileno(fp), &st) < 0 )
			perror("fstat"), exit(1);

		if ( st.st_size < previous_size )
		{
			fprintf(stderr, "tail: File truncated: %s\n", filename);
			if ( fseek(fp, 0, SEEK_END) < 0 )
				perror(filename);
		}
		previous_size = st.st_size;

		bool reopen_file = false;
		struct stat path_st;
		if ( fp != stdin && path_mode && stat(filename, &path_st) == 0 &&
			 (st.st_dev != path_st.st_dev || st.st_ino != path_st.st_ino) )
		{
			reopen_file = true;
			previous_size = path_st.st_size;
		}

		clearerr(fp);
		while ( true )
		{
			int c = fgetc(fp);
			if ( c < 0 && feof(fp) )
				break;
			else if ( c < 0 )
				perror(filename), exit(1);
			xputchar(c);
		}

		if ( fflush(stdout) != 0 )
			perror("stdout"), exit(1);

		// Reopen file only after draining the old one, in case there was any
		// change of contents before the file was replaced.
		// Additionally, keep the old file open as long as we can't succesfully
		// open the new one.
		if ( reopen_file )
		{
			FILE* new_fp = fopen(filename, "r");
			if ( new_fp )
			{
				fclose(fp);
				fp = new_fp;
				reopen_errno = 0;
				fprintf(stderr, "tail: reopened: %s\n", filename);
			}
			else if ( errno != reopen_errno )
			{
				perror(filename);
				reopen_errno = errno;
			}
		}
	}
}

static void parse_number(const char* num_string)
{
	// Handle explicit marking of whether it's relative to beginning or end.
	const char* str = num_string;

	if ( *str == '+' )
	{
		beginning_relative = true;
		str++;
	}
	else if ( *str == '-' )
	{
		beginning_relative = false;
		str++;
	}

	if ( *str == '+' || *str == '-' )
		fprintf(stderr, "Invalid number of %s: %s\n",
		     byte_mode ? "bytes" : "lines", num_string), exit(1);

	char* end;
	errno = 0;
	long int parsed = strtol(str, &end, 10);
	if ( errno == ERANGE ||
		 (beginning_relative && (off_t) parsed != parsed) ||
		 (!beginning_relative && (size_t) parsed != (uintmax_t) parsed) )
		fprintf(stderr, "Number of %s too large\n",
			 byte_mode ? "bytes" : "lines"), exit(1);
	if ( !*str || *end || errno )
		fprintf(stderr, "Invalid number of %s: %s\n",
			 byte_mode ? "bytes" : "lines", num_string), exit(1);

	if ( beginning_relative )
		file_offset = parsed;
	else
		buffer_size = parsed;
}

int main(int argc, char* argv[])
{
	const struct option longopts[] =
	{
		{"bytes", required_argument, NULL, 'c'},
		{"lines", required_argument, NULL, 'n'},
		{"follow", no_argument, NULL, 'f'},
		{"quiet", no_argument, NULL, 'q'},
		{"verbose", no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};
#ifndef HEAD
	const char* opts = "c:fFn:qv";
#else
	const char* opts = "c:n:qv";
#endif
	while ( true )
	{
		// Handle the historical but still widely used -num option format.
		if ( optind < argc && argv[optind][0] == '-' &&
		     isdigit((unsigned char) argv[optind][1]) )
		{
			parse_number(&argv[optind][1]);
			optind++;
			continue;
		}

		int opt = getopt_long(argc, argv, opts, longopts, NULL);
		if ( opt == -1 )
			break;
		switch ( opt )
		{
		case 'c':
		case 'n':
			byte_mode = opt == 'c';
			parse_number(optarg);
			break;
		case 'f': follow = true; path_mode = false; break;
		case 'F': follow = true; path_mode = true; break;
		case 'q': quiet = true; verbose = false; break;
		case 'v': verbose = true; quiet = false; break;
		default: return 1;
		}
	}

	if ( follow && 1 < argc - optind )
		fprintf(stderr, "-%c cannot be used with multiple files\n",
		     path_mode ? 'F' : 'f');

	int failure = 0;

	if ( argc - optind < 1 )
	{
		if ( verbose )
		{
			if ( printf("==> standard input <==\n") < 0 )
				perror("stdout"), exit(1);
		}

		failure |= process_file(stdin, "standard input");

		// -f is ignored if stdin is a fifo.
		struct stat st;
		if ( fstat(0, &st) < 0 )
			perror("fstat"), exit(1);
		if ( follow && !S_ISFIFO(st.st_mode) )
			follow_file(stdin, "standard input");
	}
	else
	{
		for ( int i = optind; i < argc; i++ )
		{
			bool is_stdin = !strcmp(argv[i], "-");
			const char* filename = is_stdin ? "standard input" : argv[i];
			if ( verbose || (!quiet && 1 < argc - optind) )
			{
				if ( printf("%s==> %s <==\n", i == optind ? "" : "\n",
				            filename) < 0 )
					perror("stdout"), exit(1);
			}

			FILE* fp = is_stdin ? stdin : fopen(argv[i], "r");
			if ( !fp )
			{
				perror(argv[i]);
				failure = 1;
				continue;
			}

			failure |= process_file(fp, filename);

			struct stat st;
			if ( fstat(fileno(fp), &st) < 0 )
				perror("fstat"), exit(1);
			if ( follow && (!is_stdin || !S_ISFIFO(st.st_mode)) )
				follow_file(fp, filename);

			if ( !is_stdin )
				fclose(fp);
		}
	}

	return failure;
}
