/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#if defined(__KERNEL__)
#else
  #include <string.h>
#endif

#include "xrs.h"
#include "xrs_bitmap.h"

struct xrs_bitmap *xrs_bitmap_init(struct xrs_helper_func *func, unsigned int length)
{
	struct xrs_bitmap *newbits = (struct xrs_bitmap *)func->xrs_mem_alloc(
	    sizeof(struct xrs_bitmap));
	if (!newbits)
		return NULL;

	int char_nums = sizeof (char) * (length >> 3) + 1;
	newbits->bits = (char *)func->xrs_mem_alloc(char_nums);
	if (!newbits->bits) {
		func->xrs_mem_free(newbits);
		return NULL;
	}
	memset(newbits->bits, 0, char_nums);
	newbits->length = length;

	return newbits;
}

void xrs_bitmap_free(struct xrs_helper_func *func, struct xrs_bitmap *bit)
{
	func->xrs_mem_free(bit->bits);
	func->xrs_mem_free(bit);
}

unsigned int xrs_bitmap_length(struct xrs_bitmap *bit)
{
	return bit->length;
}

int xrs_bitmap_set(struct xrs_helper_func *func, struct xrs_bitmap *bit, unsigned int idx)
{
	if (idx + 1 > bit->length) {
		func->xrs_log("Set bit map fail: idx %d exceeds bitmap size %d\n",
		    idx, bit->length);
		return -EINVAL;
	}

	int quo = idx / 8;
	int remainder = idx % 8;
	unsigned char x = (1 << remainder);
	bit->bits[quo] |= x;
	return 0;
}

int xrs_bitmap_clear(struct xrs_helper_func *func, struct xrs_bitmap *bit, uint32_t idx)
{
	if (idx + 1 > bit->length) {
		func->xrs_log("Set bit map fail: idx %d exceeds bitmap size %d\n",
		    idx, bit->length);
		return -EINVAL;
	}

	int quo = idx / 8;
	int remainder = idx % 8;
	unsigned char x = (1 << remainder);
	bit->bits[quo] &= ~x;

	return 0;
}

int xrs_bitmap_get(struct xrs_helper_func *func, struct xrs_bitmap *bit, uint32_t idx)
{
	if (idx + 1 > bit->length) {
		func->xrs_log("Get bit map fail: idx %d exceeds bitmap size %d\n",
		    idx, bit->length);
		return -EINVAL;
	}

	int quo = idx / 8;
	int remainder = idx % 8;
	unsigned char x = (1 << remainder);
	unsigned char res = bit->bits[quo] & x;
	return res ? 1 : 0;
}
