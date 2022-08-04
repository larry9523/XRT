/******************************************************************************
*  SPDX-License-Identifier: Apache-2.0
*  Copyright (C) 2021-2022 Xilinx, Inc. All rights reserved. 
*  Copyright (C) 2021-2022 Advanced Micro Devices, Inc. All rights reserved.
*
******************************************************************************/

#ifndef IPU_MSG_H_
#define IPU_MSG_H_

#include <stdint.h>

#pragma pack(push, 1)

#define IPU_MSG_PROTOCOL_VERSION        0x1
#define IPU_API_HASH_SIZE_BITS          128

#define IPU_MSG_TOMBSTONE               0xDEADFACE

/*
 * The following macro should generate a warning if !0; Assuming -Wall -Werror
 * this will cause the build to fail
 */
#define STRUCT_SIZE_CHECK_16(s) { char * p = (sizeof(s) % 2); (void)p; }
#define STRUCT_SIZE_CHECK_32(s) { char * p = (sizeof(s) % 4); (void)p; }

typedef struct ipu_uuid_ {
    uint64_t uuid_low;
    uint64_t uuid_high;
} ipu_uuid_t;

typedef enum ipu_msg_opcode_ {

    IPU_MSG_REGISTER_XCL_BIN        = 0x1,
    IPU_MSG_CREATE_CONTEXT          = 0x2,
    IPU_MSG_DELETE_CONTEXT          = 0x3,
    IPU_MSG_GET_TELEMETRY           = 0x4,
    IPU_MSG_RESET_PARTITION         = 0x5,
    IPU_MSG_EXECUTE_BUFFER          = 0x6,
    IPU_MSG_SYNC_BO                 = 0x7,
    IPU_MSG_DPU_SELF_TEST           = 0x8,
    IPU_MSG_QUERY_ERROR_INFO        = 0x9,
    IPU_MSG_UNREGISTER_XCL_BIN      = 0xA,
    IPU_MSG_CONFIG_CU               = 0xB,
    IPU_MSG_EXECUTE_BUFFER_CF       = 0xC,
    IPU_MSG_MAX_XRT_OPCODE,
    IPU_MSG_SUSPEND                 = 0x101,
    IPU_MSG_RESUME                  = 0x102,
    IPU_MSG_ASSIGN_MGMT_PASID       = 0x103,
    IPU_MSG_INVOKE_SELF_TEST        = 0x104,
    IPU_MSG_CHECK_HEADER_HASH       = 0x105,
    IPU_MSG_MAP_HOST_BUFFER         = 0x106,
    IPU_MSG_AIE_ERROR_INJECT        = 0x107,
    IPU_MSG_MAX_DRV_OPCODE,
    IPU_MSG_ASYNC_MSG_AIE_ERROR         = 0x201,
    IPU_MSG_ASYNC_MSG_WATCHDOG_TIMEOUT  = 0x202,
    IPU_MSG_MAX_OPCODE

} ipu_msg_opcode_e;

typedef enum ipu_status_ {
    IPU_STATUS_SUCCESS                      = 0x0,

    //
    // AIE Error codes
    //

    IPU_STATUS_AIE_SATURATION_ERROR         = 0x1000001,
    IPU_STATUS_AIE_FP_ERROR                 = 0x1000002,
    IPU_STATUS_AIE_STREAM_ERROR             = 0x1000003,
    IPU_STATUS_AIE_ACCESS_ERROR             = 0x1000004,
    IPU_STATUS_AIE_BUS_ERROR                = 0x1000005,
    IPU_STATUS_AIE_INSTRUCTION_ERROR        = 0x1000006,
    IPU_STATUS_AIE_ECC_ERROR                = 0x1000007,
    IPU_STATUS_AIE_LOCK_ERROR               = 0x1000008,
    IPU_STATUS_AIE_DMA_ERROR                = 0x1000009,
    IPU_STATUS_AIE_MEM_PARITY_ERROR         = 0x100000a,
    IPU_STATUS_MAX_AIE_STATUS_CODE,

    //
    // MGMT ERT Error Codes
    //

    IPU_STATUS_MGMT_ERT_SELF_TEST_FAILURE       = 0x2000001,// Self Test failure
    IPU_STATUS_MGMT_ERT_HASH_MISMATCH,                      // API Header file hash mismatch
    IPU_STATUS_MGMT_ERT_NOAVAIL,                            // Application resource not available
    IPU_STATUS_MGMT_ERT_INVALID_PARAM,                      // Invalid parameter
    IPU_STATUS_MGMT_ERT_ENTER_SUSPEND_FAILURE,              // Failed to enter suspend mode
    IPU_STATUS_MGMT_ERT_BUSY,                               // Busy performing operation
    IPU_STATUS_MGMT_ERT_APPLICATION_ACTIVE,                 // The selected application is active
    IPU_STATUS_MAX_MGMT_ERT_STATUS_CODE,

    //
    // APP ERT Error Codes
    //

    IPU_STATUS_APP_ERT_FIRST_ERROR              = 0x3000001,    // Replace this with actual error code
    IPU_STATUS_MAX_APP_ERT_STATUS_CODE,

    //
    // IPU RTOS Error Codes
    //

    IPU_STATUS_INVALID_INPUT_BUFFER             = 0x4000001,
    IPU_STATUS_INVALID_COMMAND,
    IPU_STATUS_INVALID_PARAM,
    IPU_STATUS_XCL_BIN_REG_FAILED,
    IPU_STATUS_XCL_BIN_UNREG_FAILED,
    IPU_STATUS_MAX_RTOS_STATUS_CODE,

    IPU_STATUS_MAX_IPU_STATUS_CODE

} ipu_status_e;

typedef struct ipu_msg_header_ {
    uint32_t total_msg_size;
    uint32_t msg_size           : 11;
    uint32_t rsvd0              : 5;
    uint32_t protocol_version   : 8;
    uint32_t msg_sequence_num   : 8;
    uint32_t msg_id;
    uint32_t msg_opcode;

} ipu_msg_header_t;

#pragma pack(pop)

#endif
