/******************************************************************************
  Copyright 2021 ADVANCED MICRODEVICES, INC. All rights reserved.

  AMD is granting You permission to use this software and documentation (if
  any) (collectively, the "Software") pursuant to the terms and conditions of
  the Software License Agreement included with the Software. If You do not have
  a copy of the Software License Agreement, contact Your AMD representative for
  a copy.

  You agree that You will not reverse engineer or decompile the Software, in
  whole or in part, except as allowed by applicable law.

  WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND. AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, QUALITY,
  FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT AND WARRANTIES
  ARISING FROM CUSTOM OF TRADE OR COURSE OF USAGE WITH RESPECT TO THE SOFTWARE,
  INCLUDING WITHOUT LIMITATION, THAT THE SOFTWARE WILL RUN UNINTERRUPTED OR
  ERROR-FREE. THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE SOFTWARE IS
  ASSUMED BY YOU. Some jurisdictions do not allow the exclusion of implied
  warranties, so the above exclusion may not apply to You, but only to the
  extent required by law.

  LIMITATION OF LIABILITY AND INDEMNIFICATION: TO THE EXTENT NOT PROHIBITED BY
  APPLICABLE LAW, AMD AND ITS LICENSORS WILL NOT, UNDER ANY CIRCUMSTANCES BE
  LIABLE TO YOU FOR ANY PUNITIVE, DIRECT, INCIDENTAL, INDIRECT, SPECIAL OR
  CONSEQUENTIAL DAMAGES ARISING FROM POSSESSION OR USE OF THE SOFTWARE OR
  OTHERWISE IN CONNECTION WITH ANY PROVISION OF THIS AGREEMENT EVEN IF AMD AND
  ITS LICENSORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. THIS
  INCLUDES, WITHOUT LIMITATION, DAMAGES DUE TO LOST OR MISAPPROPRIATED DATA,
  LOST PROFITS OR CONFIDENTIAL OR OTHER INFORMATION, FOR BUSINESS INTERRUPTION,
  FOR PERSONAL INJURY, FOR LOSS OF PRIVACY, FOR FAILURE TO MEET ANY DUTY
  INCLUDING OF GOOD FAITH OR REASONABLE CARE, FOR NEGLIGENCE AND FOR ANY OTHER
  PECUNIARY OR OTHER LOSS WHTSOEVER. In no event shall AMD's total liability to
  You for all damages, losses, and causes of action (whether in contract, tort
  (including negligence) or otherwise) exceed the amount of $50 USD. You agree
  to defend, indemnify and hold harmless AMD, its subsidiaries and affiliates
  and their respective licensors, directors, officers, employees, affiliates or
  agents from and against any and all loss, damage, liability and other
  expenses (including reasonable attorneys' fees), resulting from Your
  possession or use of the Software or violation of the terms and conditions of
  this Agreement.

  U.S. GOVERNMENT RESTRICTED RIGHTS: Notice to U.S. Government End Users. The
  Software and related documentation are "commercial items", as that term is
  defined at 48 C.F.R. Section 2.101, consisting of "commercial computer
  software" and "commercial computer software documentation", as such terms are
  used in 48 C.F.R. Section 12.212 and 48 C.F.R. Section 227.7202,
  respectively. Consistent with 48 C.F.R. Section 12.212 or 48 C.F.R. Sections
  227.7202-1 through 227.7202-4, as applicable, the commercial computer
  software and commercial computer software documentation are being licensed to
  U.S. Government end users: (a) only as commercial items, and (b) with only
  those rights as are granted to all other end users pursuant to the terms and
  conditions set forth in this Agreement. Unpublished rights are reserved under
  the copyright laws of the United States.

  EXPORT RESTRICTIONS:  You shall adhere to all applicable U.S. import/export
  laws and regulations, as well as the import/export control laws and
  regulations of other countries as applicable. You further agree You will not
  export, re-export, or transfer, directly or indirectly, any product,
  technical data, software or source code received from AMD under this license,
  or the direct product of such technical data or software to any country for
  which the United States or any other applicable government requires an export
  license or other governmental approval without first obtaining such licenses
  or approvals, or in violation of any applicable laws or regulations of the
  United States or the country where the technical data or software was
  obtained. You acknowledges the technical data and software received will not,
  in the absence of authorization from U.S. or local law and regulations as
  applicable, be used by or exported, re-exported or transferred to: (i) any
  sanctioned or embargoed country, or to nationals or residents of such
  countries; (ii) any restricted end-user as identified on any applicable
  government end-user list; or (iii) any party where the end-use involves
  nuclear, chemical/biological weapons, rocket systems, or unmanned air
  vehicles.  For the most current Country Group listings, or for additional
  information about the EAR or Your obligations under those regulations, please
  refer to the website of the U.S. Bureau of Industry and Security at
  http://www.bis.doc.gov/.
*/

/* Copyright (C) 2021 Xilinx, Inc. All rights reserved. */

#ifndef MGMT_MSG_H_
#define MGMT_MSG_H_

#include <ipu_msg.h>

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
    uint64_t uuid;              /** Partition ID */
    AieType aieType;            /** Type of AIE device */
    uint32_t startColumn :8;    /** Starting column of where XCL bin will be loaded */
    uint32_t totalColumn :8;    /** Total number of columns wrt xclBin */
    uint32_t : 16;
} ipu_partition_t;

typedef struct load_xcl_bin_req_ {
    uint64_t XclBinAddress;  // Kernel driver Virtual Address
    uint32_t XclBinSize;     // Size of the xclbin
    // uint32_t pad;            // TODO Xilinx temp solution for the current drop breaks load xclbin
    ipu_partition_t   part_info;
} load_xcl_bin_req_t;

typedef struct load_xcl_bin_resp_ {
    ipu_status_e status;
} load_xcl_bin_resp_t;

//
// Create Context
//

typedef struct create_context_req_ {
    uint64_t uuid;
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
    uint64_t uuid;
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
    uint64_t uuid;
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

typedef enum SelfTestStatus_ {
    SANITY_TEST_SUCCESS                 = 0x00000000,
    SANITY_TEST_NPI_ACCESS_FAIL         = 0x00000001,
    SANITY_TEST_CORE_MODULE_ACCESS_FAIL = 0x00000002,
    SANITY_TEST_TILE_ACCESS_FAIL        = 0x00000004,
    SANITY_TEST_MEM_MODULE_ACCESS_FAIL  = 0x00000008,
    SANITY_TEST_STREAM_BUFFER_1_FAIL    = 0x00000010,
    SANITY_TEST_STREAM_BUFFER_2_FAIL    = 0x00000020,
    SANITY_TEST_STREAM_BUFFER_3_FAIL    = 0x00000040,
    SANITY_TEST_STREAM_BUFFER_4_FAIL    = 0x00000080,
    SANITY_TEST_AIE2_INTERRUPT_FAIL     = 0x00000100,
    SANITY_TEST_TMR_FAIL                = 0x00000200,
    SANITY_TEST_ADMA_DRAM_FAIL          = 0x00000400,
    SANITY_TEST_ADMA_AIE2_FAIL          = 0x00000800,
} SelfTestStatus_t;

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
    uint64_t uuid;
    uint64_t buffer_address;
    uint64_t buffer_size;
} map_host_buffer_req_t;

typedef struct map_host_buffer_resp_ {
    ipu_status_e status;
} map_host_buffer_resp_t;

//
// command to start DPU self test in mgmt thread
//

typedef struct dpu_self_test_req_ {
    uint32_t cu_index;
    uint32_t data[3];
} dpu_self_test_req_t;

typedef struct dpu_self_test_resp_ {
    ipu_status_e status;
} dpu_self_test_resp_t;

#pragma pack(pop)

#endif
