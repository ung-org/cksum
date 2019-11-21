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

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_SUM_WIDTH	(32)
#define UINT32_BIT	(32)

struct sum {
	uintmax_t size;
	char sum[MAX_SUM_WIDTH];
};

static uint32_t reverse(uint32_t n, int width)
{
	uint32_t r = 0;
	for (int i = 0; i < width; i++) {
		r |= (n & 0x1) << ((width - 1) - i);
		n >>= 1;
	}
	return r;
}

static struct sum crc32(FILE *f)
{
	const uint32_t polynomial = 0x04c11db7;
	uint32_t crc = UINT32_MAX;
	struct sum sum = { 0 };

	int c;
	while ((c = fgetc(f)) != EOF) {
		sum.size++;
		crc ^= reverse((uint8_t)c, CHAR_BIT) << (UINT32_BIT - CHAR_BIT);
		for (int i = 0; i < CHAR_BIT; i++) {
			if (crc & (1 << (UINT32_BIT - 1))) {
				crc = (crc << 1) ^ polynomial;
			} else {
				crc = (crc << 1);
			}
		}
	}
	crc = reverse(crc, CHAR_BIT * sizeof(crc)) ^ UINT32_MAX;

	snprintf(sum.sum, sizeof(sum.sum), "%"PRIu32, crc);
	return sum;
}

int cksum(const char *path)
{
	uintmax_t octets = 0;

	FILE *f = stdin;
	if (path && strcmp(path, "-")) {
		f = fopen(path, "rb");
	}

	if (f == NULL) {
		fprintf(stderr, "cksum: %s: %s\n", path, strerror(errno));
		return 1;
	}

	struct sum sum = crc32(f);
	printf("%s %"PRIuMAX"", sum.sum, sum.size);

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
