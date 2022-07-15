/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
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

static void insert_partition_node(struct solver_state *xrs,
	struct solver_partition_node *pt_node)
{
	struct solver_partition_node *cnode = xrs->partition_node_head;
	struct solver_partition_node *pnode = xrs->partition_node_head;
	uint32_t i;

	if (!xrs->partition_node_head) {
		xrs->partition_node_head = pt_node;
		goto done;
	}

	while (cnode != NULL) {
		pnode = cnode;
		cnode = cnode->next;
	}

	pnode->next = pt_node;

done:
	xrs->npartition_node++;
	xrs->allocated += pt_node->ncol;
	for (i = 0; i < pt_node->ncol; i ++)
		xrs_bitmap_set(xrs->func, xrs->resbit, pt_node->start_col
		    + i);
}

static void insert_node(struct solver_state *xrs, struct solver_node *node)
{
	struct solver_node *cnode = xrs->node_head;
	struct solver_node *pnode = xrs->node_head;

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
}

static void remove_partition_node(struct solver_state *xrs,
	struct solver_partition_node *pt_node)
{
	struct solver_partition_node *cnode = xrs->partition_node_head;
	struct solver_partition_node *pnode = NULL;
	uint32_t i;

	while (cnode != NULL && cnode != pt_node) {
		pnode = cnode;
		cnode = cnode->next;
	}

	if (cnode != NULL) {
		if (pnode != NULL)
			pnode->next = cnode->next;
		else
			xrs->partition_node_head = cnode->next;

		xrs->npartition_node--;
		xrs->allocated -= pt_node->ncol;
		for (i = 0; i < pt_node->ncol; i++) {
			xrs_bitmap_clear(xrs->func, xrs->resbit,
			    pt_node->start_col + i);
		}
	}
}

static struct solver_partition_node *search_partition_node(
	struct solver_state *xrs, uint32_t start_col, uint32_t ncol)
{
	struct solver_partition_node *cnode = xrs->partition_node_head;

	while (cnode != NULL) {
		if (cnode->start_col == start_col &&
		    cnode->ncol == ncol)
			break;
		cnode = cnode->next;
	}

	return cnode;
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

		xrs->nnode--;
	}
}

static struct solver_node *search_node_by_rid(struct solver_state *xrs,
		uint32_t rid)
{
	struct solver_node *cnode = xrs->node_head;

	while (cnode != NULL) {
		if (cnode->rid == rid)
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
		    cnode->part >= 0)
			n++;
		cnode = cnode->next;
	}

	return n;

}

/*
 * The caller needs to guarantee
 *     1. nrid is not 0
 *     2. enough memory is allocated for rids (array)
 */
static void get_rids_by_xclbin_uuid(struct solver_state *xrs,
		uuid_t *xclbin_uuid, uint32_t nrid, uint32_t *rids)
{
	struct solver_node *cnode = xrs->node_head;
	uint32_t n = 0;

	while (cnode != NULL && n < nrid) {
		if (!uuid_compare(cnode->xclbin_uuid, *xclbin_uuid) &&
		    cnode->part >=0) {
			rids[n] = cnode->rid;
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
 * allocate_partition_share() - Allocate partition shared with other processes
 *
 * @xrs:   	Softstate of xrs
 * @pmp:	Input partition metadata (for Fat-XCLBIN)
 * @cdo:	Output of which CDO in Fat-XCLBIN is selected
 * @part:	Output of which overlay in the CDO is selected
 *
 * Return:	partition node successful or NULL if not found
 */
static struct solver_partition_node *allocate_partition_share(
	struct solver_state *xrs, struct part_meta *pmp,
	uint32_t *cdo, uint32_t *part)
{
	struct solver_partition_node *rpt_node = NULL;
	uint32_t i;

	for (i = 0; i < pmp->ncdos; i++) {
		struct cdo_parts *cdop = pmp->cdo + i;
		uint32_t j;

		for (j = 0; j < cdop->nparts; j++) {
			struct solver_partition_node *pt_node =
			    xrs->partition_node_head;
			uint32_t start_col = cdop->start_col_list[j];

			while (pt_node != NULL) {
				if (start_col != pt_node->start_col ||
				    cdop->ncols != pt_node->ncol) {
					pt_node = pt_node->next;
					continue;
				}

				if (rpt_node && rpt_node->nshared <=
				    pt_node->nshared) {
					pt_node = pt_node->next;
					continue;
				}

				rpt_node = pt_node;
				*cdo = i;
				*part = j;

				pt_node = pt_node->next;
			}
		}
	}

	return rpt_node;
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
	uint32_t i;

	for (i = 0; i < pmp->ncdos; i++) {
		struct cdo_parts *cdop = pmp->cdo + i;
		uint32_t j, k;

		if (xrs->total_col - xrs->allocated < cdop->ncols)
			continue;

		for (j = 0; j < cdop->nparts; j++) {
			for (k = 0; k < cdop->ncols; k++) {
				uint32_t start_col = cdop->start_col_list[j];
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

	if (i == pmp->ncdos)
		return -EBUSY;

	return 0;
}

xrs_handle_t xrs_init(uint32_t ncol, enum xrs_mode mode,
		struct xrs_helper_func *func)
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
	xrs->mode = mode;
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

int xrs_allocate_resource(xrs_handle_t hdl, struct alloc_requests *req,
		struct xrs_actions **actions,
		void (**action_cb)(xrs_handle_t hdl, struct xrs_actions *acts))
{
	struct solver_state *xrs;
	struct solver_node *node;
	struct solver_partition_node *pt_node = NULL;
	uint32_t cdo = 0, part = 0, nact;
	uint32_t rid = req->rid;
	struct part_meta *pmp = req->pmp;
	uint32_t i;
	int rval;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	/*
	 * The request ID already exists.
	 */
	if (search_node_by_rid(xrs, rid)) {
		xrs->func->xrs_log("Solver: rid %d exists\n", rid);
		return -EEXIST;
	}

	rval = allocate_partition(xrs, pmp, &cdo, &part);

	if (rval) {
		if (xrs->mode != XRS_MODE_TEMPORAL_BEST) {
			xrs->func->xrs_log("Solver: no available partition\n");
			actions = NULL;
			action_cb = NULL;
			return rval;
		}
		xrs->func->xrs_log("Solver: no unused cols available, try sharing...\n");
		pt_node = allocate_partition_share(xrs, pmp, &cdo, &part);
		if (!pt_node) {
			xrs->func->xrs_log("Solver: no available partition\n");
			actions = NULL;
			action_cb = NULL;
			return -EBUSY;
		}
		rval = 0;
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
		memset(node->oly, 0, cpart->nparts * sizeof (uint32_t));

		uuid_copy(node->xclbin_uuid, *(pmp->xclbin_uuid));
		uuid_copy(node->cdo_uuid, *(cpart->cdo_uuid));
		node->rid = rid;
		node->noly = cpart->nparts;
		node->ncol = cpart->ncols;
		memcpy(node->oly, cpart->start_col_list, cpart->nparts *
		    sizeof(uint32_t));
		if (i == cdo)
			node->part = part;
		else
			node->part = -1; /* this CDO is not allocated */

		insert_node(xrs, node);

		if (i != cdo)
			continue;

		if (pt_node == NULL) {
			pt_node = xrs->func->xrs_mem_alloc(sizeof
			    (struct solver_partition_node));
			if (!pt_node) {
				xrs->func->xrs_log("Solver: in %s, fail to allocate partition node\n", __func__);
				/* TODO release previous allocated node */
				return -ENOMEM;
			}
			memset(pt_node, 0,
			    sizeof (struct solver_partition_node));
			pt_node->nshared = 1;
			pt_node->start_col = node->oly[part];
			pt_node->ncol = node->ncol;
			insert_partition_node(xrs, pt_node);
		} else
			pt_node->nshared++;

		/*
		 * Fill up action list.
		 *
		 * TODO for static allocation, we will only have one action.
		 * Need to add more actions when we support dynamic allocation,
		 * e.g. unload/reload to different partition.
		 */
		nact = 1;
		*actions = xrs->func->xrs_mem_alloc(sizeof(struct xrs_actions) +
		    (nact - 1) * sizeof (struct xrs_action));
		if (!(*actions)) {
			xrs->func->xrs_log("Solver: in %s, fail to allocate action memory\n", __func__);
			/* TODO release previous allocated node */
			return -ENOMEM;
		}

		(*actions)->nactions = nact;
		(*actions)->actions[0].rid = node->rid;
		(*actions)->actions[0].xclbin_uuid = &node->xclbin_uuid;
		(*actions)->actions[0].cdo_uuid = &node->cdo_uuid;
		(*actions)->actions[0].rid = node->rid;
		(*actions)->actions[0].part.start_col = node->oly[part];
		(*actions)->actions[0].part.ncol = node->ncol;
		(*actions)->actions[0].action = pt_node->nshared == 1 ?
		    XRS_LOAD_ACTION_LOAD : XRS_LOAD_ACTION_NONE;
		*action_cb = xrs_action_callback;
	}

	return rval;
}

int xrs_release_resource(xrs_handle_t hdl, uint32_t rid)
{
	struct solver_partition_node *pt_node;
	struct solver_state *xrs;
	struct solver_node *node;
	int found = 0;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	while (1) {
		node = search_node_by_rid(xrs, rid);
		if (!node)
			break;

		found = 1;
		if (node->part >= 0) {
			pt_node = search_partition_node(xrs,
			    node->oly[node->part],
			    node->ncol);
			if (!pt_node) {
				xrs->func->xrs_log("Solver: in %s fatal error, can not find partition node!",
				    __func__);
				return -ENODEV;
			}

			pt_node->nshared--;
			if (pt_node->nshared == 0) {
				remove_partition_node(xrs, pt_node);
				xrs->func->xrs_mem_free(pt_node);
			}
		}

		remove_node(xrs, node);
		xrs->func->xrs_mem_free(node->oly);
		xrs->func->xrs_mem_free(node);
	}

	return found ? 0 : -ENODEV;
}

int xrs_query_nrid(xrs_handle_t hdl, uuid_t *xclbin_uuid)
{
	struct solver_state *xrs;
	int nrid;

	if (!hdl)
		return -ENODEV;

	xrs = (struct solver_state *)hdl;
	nrid = get_nnodes_by_xclbin_uuid(xrs, xclbin_uuid);

	return nrid;
}

int xrs_query_rids(xrs_handle_t hdl, uuid_t *xclbin_uuid, uint32_t nrid,
		uint32_t *rids)
{
	struct solver_state *xrs;

	if (nrid == 0)
		return -EINVAL;

	if (!hdl)
		return -ENODEV;
	xrs = (struct solver_state *)hdl;

	get_rids_by_xclbin_uuid(xrs, xclbin_uuid, nrid, rids);

	return 0;
}

#ifdef __cplusplus
}
#endif
