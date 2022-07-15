/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#ifndef SOLVER_H
#define SOLVER_H

#include "xrs.h"
#include "xrs_bitmap.h"

struct solver_node {
	uuid_t 		xclbin_uuid;
	uuid_t		cdo_uuid;
	uint32_t	rid;			/* Request ID from consumer */
	uint32_t	noly;			/* # overlay */
	uint32_t	ncol;			/* # columns */
	uint32_t	*oly;			/* start column array */
	int32_t 	part;			/* selected partition */
						/* -1: this CDO is not used */
	struct solver_node *next;
};

struct solver_partition_node {
	uint32_t	nshared;
	uint32_t	start_col;
	uint32_t	ncol;
	struct solver_partition_node *next;
};

struct solver_state {
	uint32_t		total_col;
	uint32_t		allocated;
	uint32_t		nnode;
	uint32_t		npartition_node;
	enum xrs_mode		mode;
	struct xrs_bitmap 	*resbit;
	struct solver_node	*node_head;
	struct solver_partition_node	*partition_node_head;

	struct xrs_helper_func	*func;
};

#endif
