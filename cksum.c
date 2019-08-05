/*
 * UNG's Not GNU
 *
 * Copyright (c) 2011-2017, Jakob Kaivo <jkk@ung.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
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
 */

#define _POSIX_C_SOURCE 2
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* TODO: study http://ross.net/crc/download/crc_v3.txt */

int cksum(const char *path)
{
	const uint_least32_t polynomial =
	/* 0x04c11db7; */	/* x^32 is implicit */
	0x82608edb;		/* +1 is implicit */

	uint_least32_t crc = UINT_LEAST32_MAX;
	intmax_t octets = 0;

	FILE *f = stdin;
	if (path && strcmp(path, "-")) {
		f = fopen(path, "rb");
	}

	if (f == NULL) {
		fprintf(stderr, "cksum: %s: %s\n", path, strerror(errno));
		return 1;
	}

	int c;
	while ((c = fgetc(f)) != EOF) {
		octets++;
		crc ^= (unsigned char)c;
		for (int k = 0; k < CHAR_BIT; k++) {
			crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
		}
	}

	printf("%"PRIuLEAST32" %"PRIdMAX"", crc, octets);
	if (f != stdin) {
		printf(" %s", path);
		fclose(f);
	}
	putchar('\n');

	return 0;
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	while (getopt(argc, argv, "") != -1) {
		return 1;
	}

	int r = 0;
	do {
		r |= cksum(argv[optind++]);
	} while (optind < argc);
	return r;
}
