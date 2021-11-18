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

typedef enum ipu_msg_opcode_ {

    IPU_MSG_LOAD_XCL_BIN            = 0x1,
    IPU_MSG_CREATE_CONTEXT          = 0x2,
    IPU_MSG_DELETE_CONTEXT          = 0x3,
    IPU_MSG_GET_TELEMETRY           = 0x4,
    IPU_MSG_RESET_PARTITION         = 0x5,
    IPU_MSG_EXECUTE_BUFFER          = 0x6,
    IPU_MSG_SYNC_BO                 = 0x7,
    IPU_MSG_DPU_SELF_TEST           = 0x8,
    IPU_MSG_MAX_XRT_OPCODE,
    IPU_MSG_SUSPEND                 = 0x101,
    IPU_MSG_RESUME                  = 0x102,
    IPU_MSG_ASSIGN_MGMT_PASID       = 0x103,
    IPU_MSG_INVOKE_SELF_TEST        = 0x104,
    IPU_MSG_CHECK_HEADER_HASH       = 0x105,
    IPU_MSG_MAP_HOST_BUFFER         = 0x106,
    IPU_MSG_MAX_DRV_OPCODE,
    IPU_MSG_ASYNC_MESSAGE           = 0x201,
    IPU_MSG_MAX_OPCODE

} ipu_msg_opcode_e;

typedef enum ipu_status_ {
    IPU_STATUS_SUCCESS                      = 0x0,

    IPU_STATUS_INVALID_INPUT_BUFFER         = 0x1,
    IPU_STATUS_INVALID_COMMAND              = 0x2,

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

    IPU_STATUS_MGMT_ERT_FIRST_ERROR         = 0x2000001,    // Replace this with actual error code
    IPU_STATUS_MGMT_ERT_SELF_TEST_FAILURE,                  // Self Test failure
    IPU_STATUS_MGMT_ERT_HASH_MISMATCH,                      // API Header file hash mismatch
    IPU_STATUS_MGMT_ERT_NOAVAIL,                            // Application resource not available
    IPU_STATUS_MGMT_ERT_INVALID_PARAM,                      // Invalid parameter
    IPU_STATUS_MGMT_ERT_ENTER_SUSPEND_FAILURE,              // Failed to enter suspend mode
    IPU_STATUS_MGMT_ERT_BUSY,
    IPU_STATUS_MAX_MGMT_ERT_STATUS_CODE,

    //
    // APP ERT Error Codes
    //

    IPU_STATUS_APP_ERT_FIRST_ERROR          = 0x3000001,    // Replace this with actual error code
    IPU_STATUS_MAX_APP_ERT_STATUS_CODE,

    //
    // RTOS Error Codes
    //

    IPU_STATUS_RTOS_FIRST_ERROR             = 0x4000001,    // Replace this with actual error code
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
