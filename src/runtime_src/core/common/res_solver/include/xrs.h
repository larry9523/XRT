/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#ifndef XRS_H
#define XRS_H

#ifdef _WIN32

#include <ntddk.h>
#include <stdint.h>

typedef unsigned char uuid_t[16];

inline int
uuid_compare(const uuid_t uuid1, const uuid_t uuid2)
{
	if (RtlCompareMemory(&uuid1, &uuid2, sizeof(uuid_t)) == sizeof(uuid_t))
		return 0;

	return 1;
}

inline void
uuid_copy(uuid_t dst, const uuid_t src)
{
	RtlCopyMemory(dst, src, sizeof(uuid_t));
}

#else
#if defined(__KERNEL__)
  #include <linux/types.h>
  #include <linux/uuid.h>
#else
  #include <errno.h>
  #include <stdint.h>
  #include <uuid/uuid.h>
#endif /* __KERNEL__ */
#endif /* _WIN32 */

/**
 * typedef xrs_handle_t - opaque XRT Resource Solver handle
 *
 * A handle of xrs is obtained by calling xrs_init.
 * XRS clients pass this handle to functions exported by XRS to
 * refer to the initialized XRS module.
 */
typedef void * xrs_handle_t;

/**
 * Define the resource management mode
 *
 * XRS_MODE_SPATIAL_STATIC:
 *     Partitions are shared spatially. They are allocated based on the
 *     column availability. If no available columns meet the requested
 *     overlays, allocation will fail.
 *
 * XRS_MODE_SPATIAL_DYNAMIC:
 *     Partitions are shared spatially. They are allocated based on the
 *     best effort of the current request and allocated requests. Allocated
 *     partions can be moved around to fit all requests. If no overlays meet
 *     the requests, allocation will fail.
 *
 * XRS_MODE_TEMPORAL_BEST:
 *     Partitions can be shared temporally. Firstly, we try to allocate
 *     partition on unused columns. If no available columns meet the
 *     requested overlays, we will share the already allocated partition
 *     with other request. We will try our best to load balance of requests
 *     on partitions.
 */
enum xrs_mode {
	XRS_MODE_SPATIAL_STATIC		= 0x0,
	XRS_MODE_SPATIAL_DYNAMIC	= 0x1,
	XRS_MODE_TEMPORAL_BEST		= 0x2,
};

/**
 * Define the actions after allocation
 *
 * XRS_LOAD_ACTION_NONE:
 *     Partition allocated successfully but no action is required. This is
 *     normally used in temporal sharing mode that same CDO has been loaded
 *     by other client.
 *
 * XRS_LOAD_ACTION_LOAD
 *     Partition allocated successfully. The client needs to load the CDO
 *     specified in the action payload.
 *
 * XRS_LOAD_ACTION_UNLOAD
 *     Partition allocated successfully. The client needs to unload the CDO
 *     specified in the action payload.
 */
enum xrs_load_actions {
	XRS_LOAD_ACTION_NONE		= 0x0,
	XRS_LOAD_ACTION_LOAD		= 0x1,
	XRS_LOAD_ACTION_UNLOAD		= 0x2,
};

/**
 * Structure used to describe a partition. A partition is column based
 * allocation unit described by its start column and number of columns.
 */
struct aie_part {
	uint32_t 	start_col;
	uint32_t	ncol;
};

/**
 * QoS structure. This includes factors that define the QoS which will be
 * used to describe
 *   1) the QoS capabilities of a given AIE partition
 *   2) the QoS requirement of a resource allocation
 *
 * Note:
 *       1) The original data got from XCLBIN is operations per AIE cycle. It
 *          is the Resource Solver's consumers responsibility to convert it to
 *          Tera Operations per second based on the AIE frequence at run time.
 *       2) We will only hornor tops for now. Others are just a place holder
 *          for future use.
 */
struct aie_qos {
	uint32_t	tops;		/* Tera operations per second */
	uint32_t	fps;		/* Frames per second */
	uint32_t	dma_bw;		/* DMA bandwidth */
	uint32_t	latency;	/* Frame response latency */
	uint32_t	exec_time;	/* Frame execution time */
	uint32_t	priority;	/* Request priority */
};

/**
 * Structure used to describe a relocatable CDO. A relocatable CDO is
 * identified by its CDO UUID. This CDO can be loaded on multiple
 * partition overlays.
 */
struct cdo_parts {
	uuid_t		*cdo_uuid;
	uint32_t	nparts;			/* # of partition overlays */
	uint32_t	ncols;			/* # of columns */
	uint32_t	*start_col_list;	/* Start column array */
	struct aie_qos	*cqos;			/* CDO QoS capabilities */
};

/**
 * Structure used to describe the partition metadata as part of the
 * allocation requests.This is one of the inputs to resource resolver
 * with a given XCLBIN identified by XCLBIN UUID. It also contains the
 * number of relocatable CDOs and the pointer to relocatable CDO array.
 */
struct part_meta {
	uuid_t			*xclbin_uuid;
	uint32_t		ncdos;
	struct cdo_parts	*cdo;
};

/**
 * Structure used to describe a request to allocate. This is the
 * input to resource resolver for a allocation request. It contains
 * request id and partition metadata. And this can be extended to include
 * other inputs for the allocation like QoS and Priority.
 */
struct alloc_requests {
	uint32_t		rid;
	struct part_meta	*pmp;
	struct aie_qos		*rqos;		/* Requested QoS */
};

/**
 * Structure used to describe an action after allocation. The action
 * is identified by XCLBIN UUID, CDO UUID, rid, partition and the
 * action (Load/Unload/None)
 */
struct xrs_action {
	uuid_t			*xclbin_uuid;
	uuid_t			*cdo_uuid;
	uint32_t		rid;
	struct aie_part		part;
	enum xrs_load_actions	action;
};

/**
 * Structure used to describe an action list after allocation. This
 * is the output from the resource solver after allocation is
 * successful.
 *
 * In some cases, to allocate a CDO, we may need to rearrange the
 * resourced already allocated with the consideration of requested
 * QoS. This action list is used to describe the rearrangements with
 * number of actions and a pointer to the action list array.
 */
struct xrs_actions {
	uint32_t		nactions;
	uint8_t			rsvd[4];
	struct xrs_action	actions[1];
};

/**
 * Helper functions that need to be registered when initialize
 * resource solver.
 */
struct xrs_helper_func {
	/**
	 * @xrs_mem_alloc:
	 *     Allocate memory
	 */
	void *(*xrs_mem_alloc)(size_t size);

	/**
	 * @xrs_mem_free:
	 *     Free memory
	 */
	void (*xrs_mem_free)(void *ptr);

	/**
	 * @xrs_log:
	 * 	Log message
	 */
	int (*xrs_log)(const char *fmt, ...);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * xrs_init() - Register resource solver. Resource solver client needs
 *              to call this function to register itself.
 *
 * @ncol:   	Number of columns that are managed by resource solver
 * @mode:	The allocation strategy. (See enum xrs_mode)
 * @func:	Helper functions registered for resource solver to use
 *
 * Return:	A resource solver handle
 *
 * Note: We should only create one handle per AIE array to be managed.
 */
xrs_handle_t xrs_init(uint32_t ncol, enum xrs_mode mode, struct xrs_helper_func *func);

/**
 * xrs_fini() - Unregister resource solver.
 *
 * @hdl:	Resource solver handle obtained from xrs_init()
 */
int xrs_fini(xrs_handle_t hdl);

/**
 * xrs_allocate_resource() - Request to allocate resources for a given context
 *                           and a partition metadata. (See struct part_meta)
 *
 * @hdl:	Resource solver handle obtained from xrs_init()
 * @req:	Input to the Resource solver including request id
 * 		and partition metadata.
 * @actions:	Pointer to the actions list (output)
 * @action_cb:	Callback function when the actions are done. The arg for
 *		this callback function should be the pointer of actions.
 *		TODO the arg needs to be extended to have
 *			1. result of each actiion
 *
 * Return:	0 when successful. Caller should check the actions and
 * 		proceed accordingly;
 * 		Or standard error number when failing
 *
 * Note:
 *     1. The memory of action list is allocated by resource solver
 *        and caller should call the action_cb to release it.
 *     2. There is no lock mechanism inside resource solver. So it is
 *        the caller's responsiblity to lock down XCLBINs and grab
 *        necessary lock.
 *     3. TODO Recover requests if any action is failed.
 *     4. TODO QoS is missing in this interface, which can be added
 *             to alloc_requests structure.
 */
int xrs_allocate_resource(xrs_handle_t hdl, struct alloc_requests *req,
		struct xrs_actions **actions,
		void (**action_cb)(xrs_handle_t hdl, struct xrs_actions *acts));

/**
 * xrs_release_resource() - Request to free resources for a given context.
 *
 * @hdl:	Resource solver handle obtained from xrs_init()
 * @rid:	The Request ID to identify the requesting context
 *
 * Return:	0 when successful
 * 		Or standard error number when failing
 */
int xrs_release_resource(xrs_handle_t hdl, uint32_t rid);

/**
 * xrs_query_nrid() - Query the number of request id that uses a given xclbin.
 *
 * @hdl:		Resource solver handle obtained from xrs_init()
 * @xclbin_uuid:	The XCLBIN UUID which is used
 *
 * Return:		Number of request ids when successful
 * 			Or standard error number when failing
 */
int xrs_query_nrid(xrs_handle_t hdl, uuid_t *xclbin_uuid);

/**
 * xrs_query_rids() - Query the request ids that uses a given xclbin.
 *
 * @hdl:		Resource solver handle obtained from xrs_init()
 * @xclbin_uuid:	The XCLBIN UUID which is used
 * @nrid:		Max number of request ids to fill into passid array
 * @rids:		Request id array
 *
 * Return:		0 when successful
 * 			Or standard error number when failing
 *
 * Note:
 *     1. Caller of this function needs to make sure enough memory is allocated
 *        to fill in nrid.
 *     2. If there are only m rids are using this uuid, soolver will fill the
 *        first m elements of n passids array.
 */
int xrs_query_rids(xrs_handle_t hdl, uuid_t *xclbin_uuid, uint32_t nrid,
		uint32_t *rids);


#ifdef __cplusplus
}
#endif
#endif
