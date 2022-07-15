/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#ifndef XRS_BITMAP_H
#define XRS_BITMAP_H

#include "xrs.h"

#if defined(__KERNEL__)
# include <linux/types.h>
#else
# include <errno.h>
#endif /* __KERNEL__ */

struct xrs_bitmap {
	char *bits;
	uint32_t length;
};

struct xrs_bitmap *xrs_bitmap_init(struct xrs_helper_func *func, uint32_t length);
void xrs_bitmap_free(struct xrs_helper_func *func, struct xrs_bitmap *bit);
uint32_t xrs_bitmap_length(struct xrs_bitmap *bit);
int xrs_bitmap_set(struct xrs_helper_func *func, struct xrs_bitmap *bit, uint32_t idx);
int xrs_bitmap_clear(struct xrs_helper_func *func, struct xrs_bitmap *bit, uint32_t idx);
int xrs_bitmap_get(struct xrs_helper_func *func, struct xrs_bitmap *bit, uint32_t idx);

#endif /* XRS_BITMAP_H */
