/**
 *  SPDX-License-Identifier: Apache-2.0
 *  Copyright (C) 2015-2022, Xilinx Inc
 *  Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */

/**
 * DOC: Device last / latest error status related structs and defines
 * This file is used by both userspace and kernel driver.
 * This file is used by xbutil, xocl, zocl and xbmgmt.
 */

#ifndef XCLERR_INT_H_
#define XCLERR_INT_H_

#include "xrt_error_code.h"

#define	XCL_ERROR_CAPACITY	32

/**
 * struct xclErrorLast - Container for all last(latest) error records
 * Only one entry in error array per error class xrtErrorClass
 * A xrtErrorModule may produce multiple classes of errors
 * xrtErrorCode (64 bits) = ErrorNum + Driver + Severity + Module + Class
 */
typedef struct xclErrorLast {
	xrtErrorCode	err_code;	/* 64 bits; XRT error code */
	xrtErrorTime	ts;		/* 64 bits; timestamp */
	unsigned        pid;            /* 32 bits; pid associated with error, if available */
} xclErrorLast;

typedef struct xcl_errors {
	int		num_err;	/* number of errors recorded */
	struct xclErrorLast errors[XCL_ERROR_CAPACITY];	/* error array pointer */
} xcl_errors;

#endif /* XCLERR_INT_H_ */
