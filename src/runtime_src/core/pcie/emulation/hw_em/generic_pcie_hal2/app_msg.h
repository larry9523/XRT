/******************************************************************************
*
*  Copyright (C) 2021 Advanced Micro Devices, Inc. All rights reserved.
*
******************************************************************************/

/* Copyright (C) 2021 Xilinx, Inc. All rights reserved. */

#ifndef APP_MSG_H_
#define APP_MSG_H_

#include "ipu_msg.h"
#include "ipu_plat.h"
#pragma pack(push, 1)

typedef struct cu_config_ {
    uint32_t cu_idx : 24;
    uint32_t cu_functional : 8;
    uint32_t reserved;
} cu_config_t;

typedef struct scheduler_config_buffer_req_ {
    uint32_t num_cus;
    cu_config_t configs[IPU_KERNEL_MAX_NUM];
} scheduler_config_buffer_req_t;

typedef struct scheduler_config_buffer_resp_ {
    ipu_status_e status;
} scheduler_config_buffer_resp_t;

typedef struct cu_execute_buffer_ {
    uint32_t cu_idx;
    uint32_t payload[1];
} cu_execute_buffer_t;

typedef struct execute_buffer_req_ {
    union {
        cu_execute_buffer_t cu_exec;
        uint32_t data[20];
    };
} execute_buffer_req_t;

typedef struct execute_buffer_resp_ {
    ipu_status_e status;
} execute_buffer_resp_t;

typedef struct sync_bo_req_ {
    uint64_t src_addr;      /* 64 bit of src addr */
    uint64_t dst_addr;      /* 64 bit of dst addr */
    uint32_t size;             /*The size of the sync operation*/
    uint32_t src_type : 4;     /*Source Address Type: */
    uint32_t dst_type : 4;     /*Destination Address Type:*/
    uint32_t reserved : 24;    /*Reserved*/
} sync_bo_req_t;

typedef struct sync_bo_resp_ {
    ipu_status_e status;
} sync_bo_resp_t;

typedef struct dpu_self_test_req_ {
    uint32_t cu_index;
    uint32_t data[3];
} dpu_self_test_req_t;

typedef struct dpu_self_test_resp_ {
    ipu_status_e status;
} dpu_self_test_resp_t;

#pragma pack(pop)

#endif
