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
#include "solver.h"

#ifdef __cplusplus
extern "C" {
#endif

static void insert_node(struct solver_state *xrs, struct solver_node *node)
{
	struct solver_node *cnode = xrs->node_head;
	struct solver_node *pnode = xrs->node_head;
	int i;

	if (!xrs->node_head) {
		xrs->node_head = node;
		goto done;
	}

	while (cnode != NULL) {
		pnode = cnode;
		cnode = cnode->next;
	}

	pnode->next = node;

done:
	xrs->nnode++;
	if (node->part < 0) {
		/* This CDO is not allocated */
		return;
	}
	xrs->allocated += node->ncol;
	for (i = 0; i < node->ncol; i ++)
		xrs_bitmap_set(xrs->func, xrs->resbit, node->oly[node->part]
		    + i);
}

static void remove_node(struct solver_state *xrs, struct solver_node *node)
{
	struct solver_node *cnode = xrs->node_head;
	struct solver_node *pnode = NULL;

	while (cnode != NULL && cnode != node) {
		pnode = cnode;
		cnode = cnode->next;
	}

	if (cnode != NULL) {
		if (pnode != NULL)
			pnode->next = cnode->next;
		else
			xrs->node_head = cnode->next;
		if (node->part < 0)
			return;

		xrs->nnode--;
		xrs->allocated -= node->ncol;
		for (int i = 0; i < node->ncol; i ++)
			xrs_bitmap_clear(xrs->func, xrs->resbit,
			    node->oly[node->part] + i);

	}
}

static struct solver_node *search_node_by_pasid(struct solver_state *xrs,
		uint32_t pasid)
{
	struct solver_node *cnode = xrs->node_head;

	while (cnode != NULL) {
		if (cnode->pasid == pasid)
			break;
		cnode = cnode->next;
	}

	return cnode;
}

static int get_nnodes_by_xclbin_uuid(struct solver_state *xrs,
		uuid_t *xclbin_uuid)
{
	struct solver_node *cnode = xrs->node_head;
	int n = 0;

	while (cnode != NULL) {
		if (!uuid_compare(cnode->xclbin_uuid, *xclbin_uuid) &&
		    cnode->part >=0)
			n++;
		cnode = cnode->next;
	}

	return n;

}

/*
 * The caller needs to guarantee
 *     1. npasid is not 0
 *     2. enough memory is allocated for pasids (array)
 */
static void get_pasids_by_xclbin_uuid(struct solver_state *xrs,
		uuid_t *xclbin_uuid, uint32_t npasid, uint32_t *pasids)
{
	struct solver_node *cnode = xrs->node_head;
	int n = 0;

	while (cnode != NULL && n < npasid) {
		if (!uuid_compare(cnode->xclbin_uuid, *xclbin_uuid) &&
		    cnode->part >=0) {
			pasids[n] = cnode->pasid;
			n++;
		}
		cnode = cnode->next;
	}
}

static void xrs_action_callback(xrs_handle_t hdl, struct xrs_actions *acts)
{
	struct solver_state *xrs;

	if (!hdl || !acts)
		return;

	xrs = (struct solver_state *)hdl;

	xrs->func->xrs_mem_free(acts);
}

/**
 * allocate_partition() - Allocate partition
 *
 * @xrs:   	Softstate of xrs
 * @pmp:	Input partition metadata (for Fat-XCLBIN)
 * @cdo:	Output of which CDO in Fat-XCLBIN is selected
 * @part:	Output of which overlay in the CDO is selected
 *
 * Return:	0 when successful or standard error number when failing
 */
static int allocate_partition(struct solver_state *xrs, struct part_meta *pmp,
	uint32_t *cdo, uint32_t *part)
{
	int i;

	for (i = 0; i < pmp->ncdos; i++) {
		struct cdo_parts *cdop = pmp->cdo + i;
		int j, k;

		if (xrs->total_col - xrs->allocated < cdop->ncols)
			continue;

		for (j = 0; j < cdop->nparts; j++) {
			for (k = 0; k < cdop->ncols; k++) {
				uint32_t start_col = cdop->start_col[j];
				if (xrs_bitmap_get(xrs->func, xrs->resbit,
				    start_col + k))
					break;
			}
			if (k == cdop->ncols) {
				*cdo = i;
				*part = j;
				return 0;
			}
		}
	}

	if (i == pmp->ncdos) {
		xrs->func->xrs_log("Solver: available columns %d less than requested\n", xrs->total_col - xrs->allocated);
		return -EBUSY;
	}

	return 0;
}

xrs_handle_t xrs_init(uint32_t ncol, enum xrs_mode mode, struct xrs_helper_func *func)
{
	struct solver_state *xrs;

	xrs = func->xrs_mem_alloc(sizeof (struct solver_state));
	if (!xrs) {
		func->xrs_log("Sovler: in %s: allocate xrs state failed\n",
		    __func__);
		return NULL;
	}
	memset(xrs, 0, sizeof (struct solver_state));

	xrs->func = func;
	xrs->total_col = ncol;
	xrs->resbit = xrs_bitmap_init(func, ncol);
	if (!xrs->resbit) {
		func->xrs_log("Solver: in %s: allocate xrs bitmap failed\n",
		    __func__);
		func->xrs_mem_free(xrs);
		return NULL;
	}

	return xrs;
}

int xrs_fini(xrs_handle_t hdl)
{
	struct solver_state *xrs;

	if (!hdl)
		return -ENODEV;

	xrs = (struct solver_state *)hdl;
	xrs_bitmap_free(xrs->func, xrs->resbit);
	xrs->func->xrs_mem_free(xrs);

	return 0;
}

int xrs_load_xclbin(xrs_handle_t hdl, uint32_t pasid, struct part_meta *pmp,
		struct xrs_actions **actions,
		void (**action_cb)(xrs_handle_t hdl, struct xrs_actions *acts))
{
	struct solver_state *xrs;
	struct solver_node *node;
	uint32_t cdo = 0, part = 0, nact;
	int i;
	int rval;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	/*
	 * Currently, one thread can only load one xclbin.
	 * TODO add support for one thread to load multiple xclbins.
	 */
	if (search_node_by_pasid(xrs, pasid)) {
		xrs->func->xrs_log("Solver: pasid %d exists\n", pasid);
		return -EEXIST;
	}

	rval = allocate_partition(xrs, pmp, &cdo, &part);
	if (rval) {
		xrs->func->xrs_log("Solver: no available partition\n");
		actions = NULL;
		action_cb = NULL;
		return rval;
	}

	for (i = 0; i < pmp->ncdos; i++) {
		struct cdo_parts *cpart = &pmp->cdo[i];
		node = (struct solver_node *)xrs->func->xrs_mem_alloc(
		    sizeof (struct solver_node));
		if (!node) {
			xrs->func->xrs_log("Solver: in %s, fail to allocate node memory\n", __func__);
			/* TODO release previous allocated node */
			return -ENOMEM;
		}
		memset(node, 0, sizeof (struct solver_node));
		node->oly = (uint32_t *)xrs->func->xrs_mem_alloc(cpart->nparts *
		    sizeof (uint32_t));
		if (!node->oly) {
			xrs->func->xrs_log("Solver: in %s, fail to allocate node overlay memory\n", __func__);
			xrs->func->xrs_mem_free(node);
			/* TODO release previous allocated node */
			return -ENOMEM;
		}
		memset(node->oly, 0,
		    sizeof (cpart->nparts * sizeof (uint32_t)));

		uuid_copy(node->xclbin_uuid, *(pmp->xclbin_uuid));
		uuid_copy(node->cdo_uuid, *(cpart->cdo_uuid));
		node->pasid = pasid;
		node->noly = cpart->nparts;
		node->ncol = cpart->ncols;
		memcpy(node->oly, cpart->start_col, cpart->nparts *
		    sizeof(uint32_t));
		if (i == cdo)
			node->part = part;
		else
			node->part = -1; /* this CDO is not allocated */

		insert_node(xrs, node);

		if (i != cdo)
			continue;

		/*
		 * Fill up action list.
		 *
		 * TODO for static allocation, we will only have one action.
		 * Need to add more actions when we support dynamic allocation,
		 * e.g. unload/reload to different partition.
		 */
		nact = 1;
		*actions = xrs->func->xrs_mem_alloc(sizeof (uint32_t) +
		    nact * sizeof (struct xrs_action));
		if (!(*actions)) {
			xrs->func->xrs_log("Solver: in %s, fail to allocate action memory\n", __func__);
			/* TODO release previous allocated node */
			return -ENOMEM;
		}

		(*actions)->nactions = nact;
		(*actions)->actions[0].pasid = node->pasid;
		(*actions)->actions[0].xclbin_uuid = &node->xclbin_uuid;
		(*actions)->actions[0].cdo_uuid = &node->cdo_uuid;
		(*actions)->actions[0].pasid = node->pasid;
		(*actions)->actions[0].part.start_col = node->oly[part];
		(*actions)->actions[0].part.ncol = node->ncol;
		(*actions)->actions[0].action = XRS_LOAD_ACTION_LOAD;

		*action_cb = xrs_action_callback;
	}

	return rval;
}

int xrs_unload_xclbin(xrs_handle_t hdl, uint32_t pasid)
{
	struct solver_state *xrs;
	struct solver_node *node;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	while (1) {
		node = search_node_by_pasid(xrs, pasid);
		if (!node)
			break;
		remove_node(xrs, node);
		xrs->func->xrs_mem_free(node->oly);
		xrs->func->xrs_mem_free(node);
	}

	return 0;
}

int xrs_query_npasid(xrs_handle_t hdl, uuid_t *xclbin_uuid)
{
	struct solver_state *xrs;
	int npasid;

	if (!hdl)
		return -ENODEV;

	xrs = (struct solver_state *)hdl;
	npasid = get_nnodes_by_xclbin_uuid(xrs, xclbin_uuid);

	return npasid;
}

int xrs_query_pasids(xrs_handle_t hdl, uuid_t *xclbin_uuid, uint32_t npasid,
		uint32_t *pasids)
{
	struct solver_state *xrs;

	if (npasid == 0)
		return -EINVAL;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	get_pasids_by_xclbin_uuid(xrs, xclbin_uuid, npasid, pasids);

	return 0;
}

#ifdef __cplusplus
}
#endif
