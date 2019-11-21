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
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_SUM_WIDTH	(32)
#define UINT32_BIT	(32)
#define UINT16_BIT	(16)
#define BLOCK_SIZE	(512)

enum algorithm { UNSPECIFIED, ALTERNATIVE, CRC32 };

struct sum {
	uintmax_t size;
	uintmax_t sum;
};

static char *progname = NULL;

static uint32_t reverse(uint32_t n, int width)
{
	uint32_t r = 0;
	for (int i = 0; i < width; i++) {
		r |= (n & 0x1) << ((width - 1) - i);
		n >>= 1;
	}
	return r;
}

static struct sum sum_crc32(FILE *f)
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

	sum.sum = crc;
	return sum;
}

static struct sum sum_obsolete(FILE *f, int alt)
{
	struct sum sum = { 0 };

	int c;
	while ((c = fgetc(f)) != EOF) {
		sum.size++;

		if (alt) {
			sum.sum = (sum.sum >> 1) +
				((sum.sum & 1) << (UINT16_BIT - 1));
		}

		sum.sum += c;

		if (alt) {
			sum.sum &= UINT16_MAX;
		}
	}

	sum.sum = (sum.sum & UINT16_MAX) + (sum.sum >> UINT16_BIT);

	/* obsolete sum program prints number of 512 byte blocks */
	if (sum.size % BLOCK_SIZE != 0) {
		sum.size += BLOCK_SIZE;
	}
	sum.size /= BLOCK_SIZE;

	return sum;
}

static struct sum sum_unspecified(FILE *f)
{
	return sum_obsolete(f, 0);
}

static struct sum sum_alternative(FILE *f)
{
	return sum_obsolete(f, 1);
}

int cksum(const char *path, enum algorithm alg)
{
	uintmax_t octets = 0;

	FILE *f = stdin;
	if (path && strcmp(path, "-")) {
		f = fopen(path, "rb");
	}

	if (f == NULL) {
		fprintf(stderr, "%s: %s: %s\n", progname, path, strerror(errno));
		return 1;
	}

	struct sum sum;
	switch (alg) {
	case UNSPECIFIED:
		sum = sum_unspecified(f);
		break;

	case ALTERNATIVE:
		sum = sum_alternative(f);
		break;

	case CRC32:
		sum = sum_crc32(f);
		break;

	default:
		break;
	}

	printf("%"PRIuMAX" %"PRIuMAX"", sum.sum, sum.size);

	if (f != stdin) {
		printf(" %s", path);
		fclose(f);
	}
	putchar('\n');

	return 0;
}

static int sum(int argc, char *argv[])
{
	fprintf(stderr, "sum: utility is obsolete; use cksum\n");

	enum algorithm alg = UNSPECIFIED;

	int c;
	while ((c = getopt(argc, argv, "r")) != -1) {
		switch (c) {
		case 'r':
			alg = ALTERNATIVE;
			break;

		default:
			return 1;
		}
	}

	int r = 0;
	do {
		r |= cksum(argv[optind++], alg);
	} while (optind < argc);
	return r;
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	progname = basename(argv[0]);
	if (!strcmp(progname, "sum")) {
		return sum(argc, argv);
	}

	while (getopt(argc, argv, "") != -1) {
		return 1;
	}

	int r = 0;
	do {
		r |= cksum(argv[optind++], CRC32);
	} while (optind < argc);
	return r;
}

