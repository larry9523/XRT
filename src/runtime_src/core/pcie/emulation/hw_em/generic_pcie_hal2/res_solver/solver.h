/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#ifndef SOLVER_H
#define SOLVER_H

#include "xrs.h"
#include "xrs_bitmap.h"

struct solver_node {
	uuid_t 		xclbin_uuid;
	uuid_t		cdo_uuid;
	uint32_t	pasid;
	uint32_t	noly;			/* # overlay */
	uint32_t	ncol;			/* # columns */
	uint32_t	*oly;			/* start column array */
	int32_t 	part;			/* selected partition */
						/* -1: this CDO is not used */
	struct solver_node *next;
};

struct solver_state {
	uint32_t		total_col;
	uint32_t		allocated;
	uint32_t		nnode;
	struct xrs_bitmap 	*resbit;
	struct solver_node	*node_head;

	struct xrs_helper_func	*func;
};

#endif
