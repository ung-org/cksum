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

#define _XOPEN_SOURCE 700
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* G(x)=x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 */
const uint64_t polynomial = UINT64_C(0x104C11DB7);

int cksum(const char *path)
{
	uint64_t checksum = 0;
	intmax_t octets = 0;

	FILE *f = stdin;
	if (path && strcmp(path, "-")) {
		f = fopen(path, "rb");
	}

	if (f == NULL) {
		return 1;
	}

	while (!feof(f)) {
		uint64_t next = 0;
		octets += fread(&next, sizeof(next), 1, f);
		checksum ^= next;
	}

	if (f != stdin) {
		fclose(f);
	}

	printf("%"PRIu64" %"PRIdMAX" %s\n", checksum, octets, path);
	return 0;
}

int main(int argc, char *argv[])
{
	while (getopt(argc, argv, "") != -1) {
		return 1;
	}

	int r = 0;

	do {
		r |= cksum(argv[optind++]);
	} while (optind < argc);

	return 0;
}
