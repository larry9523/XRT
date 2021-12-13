/******************************************************************************
*
*  Copyright (C) 2021 Advanced Micro Devices, Inc. All rights reserved.
*
******************************************************************************/

/* Copyright (C) 2021 Xilinx, Inc. All rights reserved. */

#ifndef MGMT_MSG_H_
#define MGMT_MSG_H_

#include "ipu_msg.h"

#pragma pack(push, 1)

#define MAX_IPU_CMD_QUEUE_PAIRS           2

//
// Load XCL Bin
//

typedef enum _AieType {
    IPU_NONE = 0,
    IPU_AIE2 = 1,
} AieType;

typedef struct ipu_partition_ {
    ipu_uuid_t uuid;            /** Partition ID */
    AieType aieType;            /** Type of AIE device */
    uint32_t startColumn :8;    /** Starting column of where XCL bin will be loaded */
    uint32_t totalColumn :8;    /** Total number of columns wrt xclBin */
    uint32_t : 16;
} ipu_partition_t;

typedef struct load_xcl_bin_req_ {
    uint64_t XclBinAddress;  // Kernel driver Virtual Address
    uint32_t XclBinSize;     // Size of the xclbin
    ipu_partition_t   part_info;
} load_xcl_bin_req_t;

typedef struct load_xcl_bin_resp_ {
    ipu_status_e status;
} load_xcl_bin_resp_t;

//
// Create Context
//

typedef struct create_context_req_ {
    ipu_uuid_t uuid;
    uint32_t pasid : 16;
    uint32_t num_command_queue_pairs_requested : 8;
    uint32_t :8;
} create_context_req_t;

typedef struct ipu_command_queue_info_t {
    uint32_t mailbox_head_ptr_offset;
    uint32_t mailbox_tail_ptr_offset;
    uint32_t buffer_start_address;
    uint32_t buffer_size;
} ipu_command_queue_info_t;

typedef struct ipu_command_queue_pair_ {
    ipu_command_queue_info_t request_queue_info;
    ipu_command_queue_info_t response_queue_info;
} ipu_command_queue_pair_t;

typedef struct create_context_resp_ {
    ipu_status_e status;
    uint32_t msi_id : 16;
    uint32_t num_command_queue_pairs_allocated : 8;
    uint32_t :8;
    ipu_command_queue_pair_t command_queue_pair[MAX_IPU_CMD_QUEUE_PAIRS];
} create_context_resp_t;

//
// Delete Context
//

typedef struct delete_context_req_ {
    ipu_uuid_t uuid;
    uint32_t pasid : 16;
    uint32_t :16;
} delete_context_req_t;

typedef struct delete_context_resp_ {
    ipu_status_e status;
} delete_context_resp_t;

//
// Get Telemetry
//

typedef enum telemetry_type_ {
    TELEMETRY_TYPE_HEALTH       = 0x0,
    TELEMETRY_TYPE_ERROR_INFO,
    TELEMETRY_TYPE_PROFILING,
    TELEMETRY_TYPE_DEBUG,
    MAX_TELEMETRY_TYPE
} telemetry_type_e;

typedef struct get_telemetry_req_ {
    telemetry_type_e type;
} get_telemetry_req_t;

typedef struct get_telemetry_resp_ {
    ipu_status_e status;
    uint8_t telemetry_data[4]; // Define the format for each type of telemetry data.
} get_telemetry_resp_t;

//
// Reset partition
//

typedef struct reset_partition_req_ {
    ipu_uuid_t uuid;
    uint32_t pasid : 16;
    uint32_t :16;
} reset_partition_req_t;

typedef struct reset_partition_resp_ {
    ipu_status_e status;
} reset_partition_resp_t;

//
// Command to assign mgmt pasid
//

typedef struct assign_mgmt_pasid_req_ {
    uint32_t pasid : 16;
    uint32_t :16;
} assign_mgmt_pasid_req_t;

typedef struct assign_mgmt_pasid_resp_ {
    ipu_status_e status;
} assign_mgmt_pasid_resp_t;

//
// Suspend
//

typedef struct suspend_req_ {
    uint32_t place_holder;

} suspend_req_t;

typedef struct suspend_resp_ {
    ipu_status_e status;
} suspend_resp_t;

//
// Resume
//

typedef struct resume_req_ {
    uint32_t place_holder;
} resume_req_t;

typedef struct resume_resp_ {
    ipu_status_e status;
} resume_resp_t;

//
// Command to invoke self test
//
typedef struct invoke_self_test_req_ {
    uint32_t place_holder;
} invoke_self_test_req_t;

typedef struct invoke_self_test_resp_ {
    ipu_status_e status;
} invoke_self_test_resp_t;

//
// Command to check header hash
//

typedef struct check_header_hash_req_ {
    uint64_t    hash_high;
    uint64_t    hash_low;
} check_header_hash_req_t;

typedef struct check_header_hash_resp_ {
    ipu_status_e status;
} check_header_hash_resp_t;

//
// command to map host buffer virtual address to IPU FW address
//

typedef struct map_host_buffer_req_ {
    ipu_uuid_t uuid;
    uint64_t buffer_address;
    uint64_t buffer_size;
} map_host_buffer_req_t;

typedef struct map_host_buffer_resp_ {
    ipu_status_e status;
} map_host_buffer_resp_t;

//
// Command to query error information
//

typedef struct query_error_info_req_ {
    uint64_t error_info_buff_addr;
    uint32_t error_info_buff_size;
    uint32_t next_row;
    uint32_t next_column;
    uint32_t next_module;
} query_error_info_req_t;

typedef struct query_error_info_resp_ {
    ipu_status_e status;
    uint32_t error_count;
    uint32_t is_next_error_valid : 1;
    uint32_t :31;
    uint32_t next_row;
    uint32_t next_column;
    uint32_t next_module;
} query_error_info_resp_t;

//
// Async message format
//

typedef enum async_event_type_ {
    ASYNC_EVENT_TYPE_ERROR    = 0x1,
    ASYNC_EVENT_TYPE_MAX
} async_event_type_e;

typedef struct async_msg_ {
    async_event_type_e async_event_type;
} async_msg_t;

#pragma pack(pop)

#endif
