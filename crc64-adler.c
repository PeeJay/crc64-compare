/* crc64.c -- compute CRC-64
 * Copyright (C) 2013 Mark Adler
 * Version 1.4  16 Dec 2013  Mark Adler
 */

/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler
  madler@alumni.caltech.edu
 */

/* Substantially modified by Paul Jones for usage in bees */

#include <stdio.h>
#include <inttypes.h>
#include "stdbool.h"

static bool init = false;
static uint64_t crc64_table[8][256];

//#define POLY UINT64_C(0xad93d23594c935a9)
#define POLY 0xd800000000000000ULL


/* Fill in a CRC constants table. */
void crc64_init() {
    
	if(!init){
	uint64_t crc;

    /* generate CRCs for all single byte sequences */
    for (int n = 0; n < 256; n++) {
        uint64_t part = n;
				for (int j = 0; j < 8; j++) {
					if (part & 1) {
						part = (part >> 1) ^ POLY;
					}
					else {
						part >>= 1;
					}
				}
		crc64_table[0][n] = part;
    }

    /* generate nested CRC table for slice-by-8 lookup */
    for (int n = 0; n < 256; n++) {
        crc = crc64_table[0][n];
        for (int k = 1; k < 8; k++) {
            crc = crc64_table[0][crc & 0xff] ^ (crc >> 8);
            crc64_table[k][n] = crc;
        }
    }
	}
}

/* Calculate a non-inverted CRC multiple bytes at a time on a little-endian
 * architecture. If you need inverted CRC, invert *before* calling and invert
 * *after* calling.
 * 64 bit crc = process 8 bytes at once;
 */
uint64_t crc64(uint64_t crc, const unsigned char *buf, uint64_t  len) {
    const unsigned char *next = buf;

crc64_init();

    /* process individual bytes until we reach an 8-byte aligned pointer */
    while (len && ((uintptr_t)next & 7) != 0) {
        crc = crc64_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    /* fast middle processing, 8 bytes (aligned!) per loop */
    while (len >= 8) {
        crc ^= *(uint64_t *)next;
        crc = crc64_table[7][crc & 0xff] ^
              crc64_table[6][(crc >> 8) & 0xff] ^
              crc64_table[5][(crc >> 16) & 0xff] ^
              crc64_table[4][(crc >> 24) & 0xff] ^
              crc64_table[3][(crc >> 32) & 0xff] ^
              crc64_table[2][(crc >> 40) & 0xff] ^
              crc64_table[1][(crc >> 48) & 0xff] ^
              crc64_table[0][crc >> 56];
        next += 8;
        len -= 8;
    }

    /* process remaining bytes (can't be larger than 8) */
    while (len) {
        crc = crc64_table[0][(crc ^ *next++) & 0xff] ^ (crc >> 8);
        len--;
    }

    return crc;
}

