/*
 * Copyright (c) 2014, 2026 Jonas 'Sortie' Termansen.
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
 * uniq.c
 * Report or filter out repeated lines in a file
 */

#include <err.h>
#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// TODO: Implement all the features mandated by POSIX.

char* read_line(FILE* fp, const char* fpname, int delim)
{
	char* line = NULL;
	size_t line_size = 0;
	ssize_t amount = getdelim(&line, &line_size, delim, fp);
	if ( amount < 0 )
	{
		free(line);
		if ( ferror(fp) )
			err(1, "read: %s", fpname);
		return NULL;
	}
	if ( (unsigned char) line[amount-1] == (unsigned char) delim )
		line[amount-1] = '\0';
	return line;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

	bool count = false;
	bool delete_singulars = false;
	bool delete_duplicates = false;
	bool zero_terminated = false;

	int opt;
	while ( (opt = getopt(argc, argv, "cduz")) != -1 )
	{
		switch ( opt )
		{
		case 'c': count = true; break;
		case 'd': delete_singulars = true; break;
		case 'u': delete_duplicates = true; break;
		case 'z': zero_terminated = true; break;
		default: return 1;
		}
	}

	if ( 3 <= argc - optind )
		errx(1, "unexpected extra operand: %s", argv[optind + 2]);

	const char* inname = "stdin";
	const char* outname = "stdout";

	if ( 2 <= argc - optind &&
	     !freopen(outname = argv[optind + 1], "w", stdout) )
		err(1, "%s", outname);

	if ( 1 <= argc - optind &&
	     !freopen(inname = argv[optind + 0], "r", stdin) )
		err(1, "%s", inname);

	int delim = zero_terminated ? '\0' : '\n';

	uintmax_t num_repeats = 0;
	char* prev_line = NULL;
	while ( true )
	{
		char* line = read_line(stdin, inname, delim);

		bool first = !prev_line;
		bool different = !first && (!line || strcoll(line, prev_line) != 0);

		if ( delete_singulars && delete_duplicates )
		{
		}
		else if ( delete_singulars )
		{
			if ( different && 1 < num_repeats )
			{
				if ( count )
					printf("%ju ", num_repeats);
				fputs(prev_line, stdout);
				fputc(delim, stdout);
			}
		}
		else if ( delete_duplicates )
		{
			if ( different && num_repeats == 1 )
			{
				if ( count )
					printf("%ju ", num_repeats);
				fputs(prev_line, stdout);
				fputc(delim, stdout);
			}
		}
		else
		{
			if ( count && different )
			{
				printf("%ju ", num_repeats);
				fputs(prev_line, stdout);
				fputc(delim, stdout);
			}
		}

		if ( different )
			num_repeats = 0;

		if ( !line )
			break;

		bool original = first || strcoll(line, prev_line) != 0;
		if ( !count && !delete_singulars && !delete_duplicates && original )
		{
			fputs(line, stdout);
			fputc(delim, stdout);
			if ( ferror(stdout) )
				err(1, "write: %s", outname);
		}

		free(prev_line);
		prev_line = line;
		num_repeats++;
	}
	free(prev_line);

	if ( fflush(stdout) == EOF || ferror(stdout) )
		err(1, "write: %s", outname);

	return 0;
}
