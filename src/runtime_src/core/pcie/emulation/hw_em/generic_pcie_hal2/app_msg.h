/*
* Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
*/
#ifndef APP_MSG_H_
#define APP_MSG_H_

#include "ipu_msg.h"

#pragma pack(push, 1)

typedef struct execute_buffer_req_ {
//    uint64_t buffer_address;
//    uint64_t buffer_size;
    uint32_t data[20];
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

#pragma pack(pop)

#endif
