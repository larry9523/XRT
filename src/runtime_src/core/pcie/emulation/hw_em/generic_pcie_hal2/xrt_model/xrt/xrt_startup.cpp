/*
 ******************************************************************************
 *
 * Copyright 2020 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
 *
 * AMD is granting you permission to use this software and documentation (if
 * any) (collectively, the “Materials”) pursuant to the terms and conditions of
 * the Software License Agreement included with the Materials.  If you do not
 * have a copy of the Software License Agreement, contact your AMD
 * representative for a copy.
 *
 * You agree that you will not reverse engineer or decompile the Materials, in
 * whole or in part, except as allowed by applicable law.
 *
 * WARRANTY DISCLAIMER:  THE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
 * INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE
 * MATERIALS WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM
 * CUSTOM OF TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE
 * OF THE MATERIAL IS ASSUMED BY YOU.  Some jurisdictions do not allow the
 * exclusion of implied warranties, so the above exclusion may not apply to
 * You.
 *
 * LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
 * NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
 * INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
 * THE MATERIALS OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  In no event shall AMD's total
 * liability to You for all damages, losses, and causes of action (whether in
 * contract, tort (including negligence) or otherwise) exceed the amount of
 * $100 USD. You agree to defend, indemnify and hold harmless AMD and its
 * licensors, and any of their directors, officers, employees, affiliates or
 * agents from and against any and all loss, damage, liability and other
 * expenses (including reasonable attorneys' fees), resulting from Your use of
 * the Materials or violation of the terms and conditions of this Agreement.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS:  The Materials are provided with
 * "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is
 * subject to the restrictions as set forth in FAR 52.227-14 and
 * DFAR252.227-7013, et seq., or its successor.  Use of the Materials by the
 * Government constitutes acknowledgment of AMD's proprietary rights in them.
 *
 * EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
 * stated in the Software License Agreement.
 *******************************************************************************
 */

/* Copyright (C) 2021 Xilinx, Inc. All rights reserved. */

/*----------------------------------------------------------------------------------------
 *                            M O D U L E S    U S E D
 *----------------------------------------------------------------------------------------
 */
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include <typeinfo> // for typeid
#include <fstream>

#include "host_env.h"
#include "ipuhenv.h"
#include "os.h"
#include "header.h"
#include "ipu_msg.h"
#include "mgmt_msg.h"
#include "app_msg.h"
#include "ipuhenvring.h"
#include "reg/chip_offset_byte.h"
#include "ipurb_hwemu.h"

using namespace::std;
/*----------------------------------------------------------------------------------------
 *                   D E F I N I T I O N S    A N D    M A C R O S
 *----------------------------------------------------------------------------------------
 */

//
// For SimNow Phase 1 development, where the XRT is abstracted and compiled to a SimNow BFM model,
// we will randomly choose DRAM addresses 0x4000000 - 0x8000000 (64MB region) as shared memory
// accessible by both the XRT and the IPU firmware.
//
static uint64_t TEST_DRAM_BASE_ADDR  = 0x4000000;
static const uint64_t TEST_DRAM_SIZE       = 0x4000000;
static uint64_t TEST_DRAM_LIMIT_ADDR = TEST_DRAM_BASE_ADDR + TEST_DRAM_SIZE - 1;

#define MPIPU_PWAITMODE             (mmMPIPU_LX3_PWAITMODE)
#define MPIPU_PMFW_SUSP_RDY         (mmMPIPU_C2PMSG_112)
#define MPIPU_POST_REG              (mmMPIPU_EXT_SCRATCH31)
#define MPIPU_TEST_DRAM_LO          (mmMPIPU_EXT_SCRATCH1)
#define MPIPU_TEST_DRAM_HI          (mmMPIPU_EXT_SCRATCH2)

#define FATAL(f, ...) { printf(f, ##__VA_ARGS__); abort(); }

/*----------------------------------------------------------------------------------------
 *                  T Y P E D E F S     A N D     S T R U C T U R E S
 *----------------------------------------------------------------------------------------
 */
enum X2I_RESPONSE {
    X2I_RESPONSE_OK   = 0,
    X2I_RESPONSE_FAIL = 1,
};

static const uint32_t ALIVE_PTR = X2I_SMN_MMIO_ADDR(mmMPIPU_SRAM_I2X_MAILBOX_15);

class TestResult
{
public:
    std::string m_TestName;
    bool m_Passed;

    TestResult(const char *testName, bool passed)
        : m_TestName(testName)
        , m_Passed(passed) { }

    void PrintResult()
    {
        printf("%-27s : %s\n", m_TestName.c_str(), (m_Passed) ? "PASS" : "FAIL");
    }
};
std::vector< TestResult > TestResults;

/*----------------------------------------------------------------------------------------
 *           P R O T O T Y P E S     O F     L O C A L     F U N C T I O N S
 *----------------------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------------------
 *                          E X P O R T E D    F U N C T I O N S
 *----------------------------------------------------------------------------------------
 */

#define HENV_TRACE do { cout << "[HENV] "<< __FILE__ << ":" << __LINE__<< endl; } while ( 0 )
#define HENV_C2PMSG_TRACE( MSG, VAL ) do { \
    cout << "[HENV] " << #MSG << ": 0x" << hex << VAL << dec << endl; \
    WR_SMN_ADDR( X2I_SMN_MMIO_ADDR( MSG ), VAL++ ); \
} while ( 0 )
#define HENV_WR_C2PMSG( MSG, VAL ) do { \
    HENV_TRACE; \
    WR_SMN_ADDR( X2I_SMN_MMIO_ADDR( MSG ), VAL ); \
} while ( 0 )

void printMsg(string msg)
{
    std::cout << "XRT_MIMIC::" << msg << endl;
}


#define TEST_FN_HEADER do { printTestFunctionHeader( __FUNCTION__ ); } while( 0 )
void printTestFunctionHeader(const char *name)
{
    printf("[TEST STATUS] [%s] Start\n", name);
}

#define TEST_FN_RESULT(PASSED) do { printTestFunctionResult( __FUNCTION__, (PASSED) ); } while( 0 )
void printTestFunctionResult(const char *name, bool passed)
{
    TestResults.push_back(TestResult(name, passed));
    printf("[TEST STATUS] [%s] <<< %s >>>\n", name, ((passed) ? "PASS" : "FAIL"));
}

void printHeader(const char *name, const ipu_msg_header_t&header)
{
    printf("[HENV] %s.msg_id=0x%X\n", name, header.msg_id);
    printf("[HENV] %s.msg_opcode=0x%X\n", name, header.msg_opcode);
    printf("[HENV] %s.msg_size=0x%X\n", name, header.msg_size);
    printf("[HENV] %s.total_msg_size=0x%X\n", name, header.total_msg_size);
    printf("[HENV] %s.protocol_version=0x%X\n", name, header.protocol_version);
    printf("[HENV] %s.msg_sequence_num=0x%X\n", name, header.msg_sequence_num);
}

#define CASE_RESP( N,R ) case R: printf( "[HENV] %s Command Status: %s\n", N, #R ); break;
void printRespStatus(const char *name, const ipu_status_e status)
{
    switch(status) {
        CASE_RESP(name,IPU_STATUS_SUCCESS);

        CASE_RESP(name,IPU_STATUS_INVALID_INPUT_BUFFER);
        CASE_RESP(name,IPU_STATUS_INVALID_COMMAND);

        //
        // AIE Error codes
        //

        CASE_RESP(name,IPU_STATUS_AIE_SATURATION_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_FP_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_STREAM_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_ACCESS_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_BUS_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_INSTRUCTION_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_ECC_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_LOCK_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_DMA_ERROR);
        CASE_RESP(name,IPU_STATUS_AIE_MEM_PARITY_ERROR);
        CASE_RESP(name,IPU_STATUS_MAX_AIE_STATUS_CODE);

        //
        // MGMT ERT Error Codes
        //

        CASE_RESP(name,IPU_STATUS_MGMT_ERT_SELF_TEST_FAILURE);
        CASE_RESP(name,IPU_STATUS_MGMT_ERT_HASH_MISMATCH);
        CASE_RESP(name,IPU_STATUS_MGMT_ERT_NOAVAIL);
        CASE_RESP(name,IPU_STATUS_MGMT_ERT_INVALID_PARAM);
        CASE_RESP(name,IPU_STATUS_MGMT_ERT_ENTER_SUSPEND_FAILURE);
        CASE_RESP(name, IPU_STATUS_MGMT_ERT_BUSY);
        CASE_RESP(name,IPU_STATUS_MAX_MGMT_ERT_STATUS_CODE);

        //
        // APP ERT Error Codes
        //

        CASE_RESP(name,IPU_STATUS_APP_ERT_FIRST_ERROR);
        CASE_RESP(name,IPU_STATUS_MAX_APP_ERT_STATUS_CODE);

        //
        // RTOS Error Codes
        //

        CASE_RESP(name,IPU_STATUS_MAX_RTOS_STATUS_CODE);

        CASE_RESP(name,IPU_STATUS_MAX_IPU_STATUS_CODE);

    default:
        printf("[HENV] %s: %s (0x%X)\n", name, "UNKNOWN RESPONSE!", status);
        break;
    }
}

#define PADDING_UINT32( X ) ((X % sizeof(uint32_t)) ? (4 - (X % sizeof(uint32_t))):0)
/**
 * Wrapper function to execute an IPU command utilizing the ring buffer implementation in the IPU
 *
 * @param <COMMAND>       command type
 * @param <RESPONSE>      response type
 * @param command       The COMMAND type command to execute
 * @param response      The RESPONSE type response object
 * @param pRing         The ring buffer to use
 * @param msg_id        The message ID
 * @param opcode        The op code
 * @param cmdStr        String of the command to execute for reporting
 * @param fnStr         The test function name, for reporting
 * @param expectSuccess Normally true, means the test should get a successful result, set to false to test for failure
 *
 * @return bool true if the command received the desired result (success if expectSuccess is true, !success if
 *         expectSuccess is false)
 */
template < typename COMMAND, typename RESPONSE >
bool RINGB_Command(COMMAND command, RESPONSE  *response, IpuHenvRing *pRing,
                   uint32_t msg_id, ipu_msg_opcode_e opcode, const char *cmdStr, const char *fnStr,
                   bool expectSuccess = true)
{
    ipu_msg_header_t txHeader = { 0 };
    ipu_msg_header_t rxHeader = { 0 };

    txHeader.msg_id = msg_id;
    txHeader.msg_opcode = opcode;
    txHeader.msg_size = sizeof(COMMAND);
    txHeader.total_msg_size = sizeof(COMMAND);
    txHeader.protocol_version = IPU_MSG_PROTOCOL_VERSION;

    response->status = IPU_STATUS_MAX_IPU_STATUS_CODE;

    if(pRing->Rx().GetAvailable()) {
        printf("[HENV] !!! %s: Clearing stale data !!! \n", fnStr);
        fprintf(stderr, "[HENV] !!! %s: Clearing stale data !!! \n", fnStr);
        pRing->Rx().PopHead(pRing->Rx().GetAvailable());
    }

    const uint32_t needed = sizeof(ipu_msg_header_t) + sizeof(COMMAND);
    if(pRing->Tx().CheckTail(needed)) {
        uint32_t txSize = pRing->WriteBuffer(0, &txHeader, sizeof(ipu_msg_header_t));
        txSize += pRing->WriteBuffer(txSize, &command, sizeof(COMMAND));
        printf("txSize=0x%X, needed=0x%X\n", txSize, needed);
        if(needed == txSize) {
            printf("[HENV] All data transmitted\n");
            txSize += PADDING_UINT32(txSize);
            pRing->Tx().PushTail(txSize);

            if(pRing->Rx().WaitForData(sizeof(ipu_msg_header_t) + sizeof(RESPONSE))) {
                uint32_t dataAvailable = pRing->Rx().GetAvailable();
                printf("[HENV] dataAvailable=0x%X\n", dataAvailable);
                if(dataAvailable >= sizeof(ipu_msg_header_t) + sizeof(RESPONSE)) {
                    uint32_t rxSize = pRing->ReadBuffer(0, &rxHeader, sizeof(ipu_msg_header_t));
                    printf("[HENV] Rx Message Size: 0x%X, expected: 0x%lX\n"
                           , rxHeader.total_msg_size, sizeof(RESPONSE));
                    rxSize += pRing->ReadBuffer(rxSize, response, sizeof(RESPONSE));

                    printHeader("rxHeader", rxHeader);
                    if(rxSize < dataAvailable) {
                        printf("[HENV] !!! More data available (0x%X) than expected (0x%lX)\n", dataAvailable
                               , (sizeof(ipu_msg_header_t) + sizeof(RESPONSE)));
                        // BOZO: TODO: FIXME: we are just blinding skipping data here, we should figure out how to
                        // properly read all of it
                        rxSize = dataAvailable;
                    } else {
                        rxSize += PADDING_UINT32(rxSize);
                    }

                    pRing->Rx().PopHead(rxSize);
                    printRespStatus(cmdStr, response->status);
                } else {
                    printf("\n\t[HENV] >>> %s: Not enough data - FAIL\n", fnStr);
                }
            } else {
                printf("\n\t[HENV] >>> %s: No data received - FAIL\n", fnStr);
            }
        } else {
            printf("\n\t[HENV] >>> %s: Not enough space - FAIL\n", fnStr);
        }
    } else {
        printf("\n\t[HENV] >>> %s: No Space at tail - FAIL\n", fnStr);
    }

    return (expectSuccess)? (IPU_STATUS_SUCCESS == response->status) : (IPU_STATUS_SUCCESS != response->status);
}

void TEST_IpuInvokeSelfTest(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    invoke_self_test_req_t self_test_req = { 0 };
    invoke_self_test_resp_t self_test_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    uint32_t /*__attribute__((aligned(32)))*/ fill_data[4096];

    for(size_t i=0; i<4096; i++)
        fill_data[i] = 0xfeedbeef;
    WR_SYSHUB(TEST_DRAM_BASE_ADDR, fill_data, sizeof(fill_data));


    // self_test_req.place_holder = 0xBA5EFACE;

    bool passed = RINGB_Command(self_test_req, &self_test_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_INVOKE_SELF_TEST
                                , "IPU_MSG_INVOKE_SELF_TEST", __FUNCTION__);

    TEST_FN_RESULT(passed);
}

// Create dummy xclbin (algined for mpIPU dma purposes)
uint8_t __attribute__((aligned(32))) xcl_bin_data[256];

// #define XILINX_DPU_SELF_TEST
void TEST_DPUSelfTest(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    dpu_self_test_req_t stest_req = { 0 };
    dpu_self_test_resp_t stest_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

#define IFM_SIZE 5120
#define WGT_SIZE 148480
#define BIAS_SIZE 512
#define OFM_SIZE 2048
#define INSTR_SIZE 4096

    int *buffer;
    uint32_t size;
    size  = IFM_SIZE + OFM_SIZE + WGT_SIZE + BIAS_SIZE + INSTR_SIZE;
    buffer = (int *)malloc(size);
    FILE* stream = fopen("./buffer.dat", "rb");
    size_t result = fread( buffer, sizeof(int32_t), (IFM_SIZE + OFM_SIZE + WGT_SIZE + BIAS_SIZE + INSTR_SIZE)/sizeof(int32_t), stream);
    if (result != size) {
      printf("DPU user self test: map host buffer failed\n");
      TEST_FN_RESULT(0);
    }

    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, buffer, size);

    // XCLBIN has to be in DRAM (model) not in this fsdl libarary address space
    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, buffer, size);
    stest_req.data[0] = (uint64_t)TEST_DRAM_BASE_ADDR + 0x4000;
    stest_req.data[1] = 0;
    stest_req.data[2] = size;

    bool passed = RINGB_Command(stest_req, &stest_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_DPU_SELF_TEST
        , "IPU_MSG_DPU_SELF_TEST", __FUNCTION__);

    TEST_FN_RESULT(passed);
}

void TEST_DPUUserSelfTest(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;

/* create context */
    create_context_req_t create_context_req;
    create_context_resp_t create_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    create_context_req.part_info.aie_type = IPU_AIE2;
    create_context_req.part_info.start_column = 0x0;
    create_context_req.part_info.total_columns = 0x5;

    create_context_req.num_xcl_bin_uuids = 0x1;
    create_context_req.xcl_bin_uuid[0].uuid_low = 0x1234567800ABCDEF;
    create_context_req.xcl_bin_uuid[0].uuid_high = 0xFEDCBA9876543210;

    create_context_req.pasid = 0xFFFF;
    create_context_req.num_command_queue_pairs_requested = 0x1;

    bool passed = RINGB_Command(create_context_req, &create_context_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_CREATE_CONTEXT
                                , "IPU_MSG_CREATE_CONTEXT", __FUNCTION__);
    if (!passed) {
        printf("DPU user self test: create context failed\n");
        TEST_FN_RESULT(passed);
    }

    ipu_command_queue_pair_t *qPair = &create_context_resp.command_queue_pair[0];
    uint32_t context_id = create_context_resp.context_id;
    printf("create_context_resp.msi_id=0x%X\n", create_context_resp.msi_id);
    printf("create_context_resp.num_command_queue_pairs_allocated=0x%X\n"
           , create_context_resp.num_command_queue_pairs_allocated);

    printf("XRT-MB REQ HD INFO 0x%x\n", qPair->request_queue_info.mailbox_head_ptr_offset);
    printf("XRT-MB REQ TL INFO 0x%x\n", qPair->request_queue_info.mailbox_tail_ptr_offset);
    printf("XRT-MB REQ BA INFO 0x%x\n", qPair->request_queue_info.buffer_start_address);
    printf("XRT-MB REQ BL INFO 0x%x\n", qPair->request_queue_info.buffer_size);

    printf("XRT-MB RSP HD INFO 0x%x\n", qPair->response_queue_info.mailbox_head_ptr_offset);
    printf("XRT-MB RSP TL INFO 0x%x\n", qPair->response_queue_info.mailbox_tail_ptr_offset);
    printf("XRT-MB RSP BA INFO 0x%x\n", qPair->response_queue_info.buffer_start_address);
    printf("XRT-MB RSP BL INFO 0x%x\n", qPair->response_queue_info.buffer_size);

    /* end create context */

/* map host buffer */
    map_host_buffer_req_t map_host_buffer_req = { 0 };
    map_host_buffer_resp_t map_host_buffer_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    int *buffer;
    uint32_t size;
    size  = IFM_SIZE + OFM_SIZE + WGT_SIZE + BIAS_SIZE + INSTR_SIZE;
    buffer = (int *)malloc(size);
    FILE* stream = fopen("./buffer.dat", "rb");
    size_t result = fread( buffer, sizeof(int32_t), (IFM_SIZE + OFM_SIZE + WGT_SIZE + BIAS_SIZE + INSTR_SIZE)/sizeof(int32_t), stream);
    if (result != size) {
      printf("DPU user self test: map host buffer failed\n");
      TEST_FN_RESULT(0);
    }

    // Dummy test data for the buffer access test
    WR_SYSHUB(TEST_DRAM_BASE_ADDR, buffer, size);

    for (int i = 0; i < 16; i ++)
        printf("%x: %x\n", 150 * 1024 + i, ((char *)(buffer))[150 * 1024 + i]);

    map_host_buffer_req.context_id = context_id;
    map_host_buffer_req.buffer_address = TEST_DRAM_BASE_ADDR;
    map_host_buffer_req.buffer_size = size;

    passed = RINGB_Command(map_host_buffer_req, &map_host_buffer_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_MAP_HOST_BUFFER
                                , "IPU_MSG_MAP_HOST_BUFFER", __FUNCTION__);

    if (!passed) {
        printf("DPU user self test: map host buffer failed\n");
        TEST_FN_RESULT(passed);
    }

/* end map host buffer */

/* sync bo */

    if(create_context_resp.num_command_queue_pairs_allocated > 0) {
        IpuHenvRing rb(qPair->request_queue_info, qPair->response_queue_info, 0);

        sync_bo_req_t sync_bo_req;
        sync_bo_resp_t sync_bo_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

        sync_bo_req.src_addr = TEST_DRAM_BASE_ADDR;
        sync_bo_req.size = size;

        passed = RINGB_Command(sync_bo_req, & sync_bo_resp, &rb, 0xFA5EFADE, IPU_MSG_SYNC_BO
                                , "IPU_MSG_SYNC_BO", __FUNCTION__);
    } else {
        printf("[HENV] !!! Failed to create any command pairs!\n");
    }
    TEST_FN_RESULT(passed);
}


void TEST_DPULoadXclBin(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    register_xcl_bin_req_t xcl_req = { 0 };
    register_xcl_bin_resp_t xcl_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    std::string binaryFile = "./4cmt_simnow_interpreter_0916.pdi";
    const char* bit = binaryFile.c_str();
    ifstream stream(bit);
    stream.seekg(0, stream.end);
    int size = stream.tellg();
    stream.seekg(0, stream.beg);
    char* header = new char[size];
    stream.read(header, size);

    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, header, size);

    // XCLBIN has to be in DRAM (model) not in this fsdl libarary address space
    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, header, size);

    xcl_req.num_xcl_bin_infos = 1;
    xcl_req.xcl_bin_info[0].xcl_bin_address = (uint64_t)(TEST_DRAM_BASE_ADDR + 0x4000);
    xcl_req.xcl_bin_info[0].xcl_bin_size = size;
    xcl_req.xcl_bin_info[0].xcl_bin_uuid.uuid_low = 0x1234567800ABCDEF;
    xcl_req.xcl_bin_info[0].xcl_bin_uuid.uuid_high = 0xFEDCBA9876543210;


/*
    xcl_req.XclBinAddress = (uint64_t)TEST_DRAM_BASE_ADDR + 0x4000;
    xcl_req.XclBinSize = size;
    xcl_req.part_info.startColumn = 0;
    xcl_req.part_info.totalColumn = 5;
    xcl_req.part_info.aieType = IPU_AIE2;
*/
    bool passed = RINGB_Command(xcl_req, &xcl_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_REGISTER_XCL_BIN
        , "IPU_MSG_REGISTER_XCL_BIN", __FUNCTION__);

    TEST_FN_RESULT(passed);
}

void TEST_IpuLoadXclBin(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    register_xcl_bin_req_t xcl_req = { 0 };
    register_xcl_bin_resp_t xcl_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    // simple pattern for sanity checks
    for(size_t i=0; i<sizeof(xcl_bin_data); i++)
        xcl_bin_data[i] = i;
    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, xcl_bin_data, sizeof(xcl_bin_data));

    // XCLBIN has to be in DRAM (model) not in this fsdl libarary address space
    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x4000, xcl_bin_data, sizeof(xcl_bin_data));

    xcl_req.num_xcl_bin_infos = 1;
    xcl_req.xcl_bin_info[0].xcl_bin_address = (uint64_t)(TEST_DRAM_BASE_ADDR + 0x4000);
    xcl_req.xcl_bin_info[0].xcl_bin_size = sizeof(xcl_bin_data);
    xcl_req.xcl_bin_info[0].xcl_bin_uuid.uuid_low = 0x1234567800ABCDEF;
    xcl_req.xcl_bin_info[0].xcl_bin_uuid.uuid_high = 0xFEDCBA9876543210;

/*

    xcl_req.XclBinAddress = (uint64_t)TEST_DRAM_BASE_ADDR + 0x4000;
    xcl_req.XclBinSize = sizeof(xcl_bin_data);

    xcl_req.part_info.uuid.uuid_low = 0x1234567800ABCDEF;
    xcl_req.part_info.uuid.uuid_high = 0xFEDCBA9876543210;
    xcl_req.part_info.startColumn = 1;
    xcl_req.part_info.totalColumn = 3;
    xcl_req.part_info.aieType = IPU_AIE2;
*/
    bool passed = RINGB_Command(xcl_req, &xcl_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_REGISTER_XCL_BIN
                                , "IPU_MSG_REGISTER_XCL_BIN", __FUNCTION__);

    TEST_FN_RESULT(passed);
}

void TEST_IpuUnregisterXclBin(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    unregister_xcl_bin_req_t unreg_xcl_bin_req = { 0 };
    unregister_xcl_bin_resp_t unreg_xcl_bin_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    unreg_xcl_bin_req.xcl_bin_uuid[0].uuid_low = 0x1234567800ABCDEF;
    unreg_xcl_bin_req.xcl_bin_uuid[0].uuid_high = 0xFEDCBA9876543210;

    bool passed = RINGB_Command(unreg_xcl_bin_req, &unreg_xcl_bin_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_UNREGISTER_XCL_BIN
                                , "IPU_MSG_UNREGISTER_XCL_BIN", __FUNCTION__);

    TEST_FN_RESULT(passed);
}


void TEST_IpuExecuteBuffer(IpuHenvRing *pAppCtxBuff)
{
    static uint64_t ba = 0xABCDFEADADFE;
    TEST_FN_HEADER;
    execute_buffer_req_t exec_buf_req;
    execute_buffer_resp_t exec_buf_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    //exec_buf_req.buffer_address = ba;
    //exec_buf_req.buffer_size = 0;

    bool passed = RINGB_Command(exec_buf_req, & exec_buf_resp, pAppCtxBuff, 0xFA5EFADE, IPU_MSG_EXECUTE_BUFFER
                                , "IPU_MSG_EXECUTE_BUFFER", __FUNCTION__);

    ba+=0xefaabcdef;
    TEST_FN_RESULT(passed);
}

static void TEST_IpuCreateContext(IpuHenvRing *pMngBuff, uint32_t *context_id)
{
    TEST_FN_HEADER;
    create_context_req_t create_context_req;
    create_context_resp_t create_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };



    create_context_req.part_info.aie_type = IPU_AIE2;
    create_context_req.part_info.start_column = 0x0;
    create_context_req.part_info.total_columns = 0x5;

    create_context_req.num_xcl_bin_uuids = 0x1;
    create_context_req.xcl_bin_uuid[0].uuid_low = 0x1234567800ABCDEF;
    create_context_req.xcl_bin_uuid[0].uuid_high = 0xFEDCBA9876543210;

    //create_context_req.uuid.uuid_low = 0x1234567800ABCDEF;
    //create_context_req.uuid.uuid_high = 0xFEDCBA9876543210;
    create_context_req.pasid = 0xFFFF;
    create_context_req.num_command_queue_pairs_requested = 0x1;

    bool passed = RINGB_Command(create_context_req, &create_context_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_CREATE_CONTEXT
                                , "IPU_MSG_CREATE_CONTEXT", __FUNCTION__);

    ipu_command_queue_pair_t *qPair = &create_context_resp.command_queue_pair[0];

    printf("create_context_resp.msi_id=0x%X\n", create_context_resp.msi_id);
    printf("create_context_resp.num_command_queue_pairs_allocated=0x%X\n"
           , create_context_resp.num_command_queue_pairs_allocated);

    printf("XRT-MB REQ HD INFO 0x%x\n", qPair->request_queue_info.mailbox_head_ptr_offset);
    printf("XRT-MB REQ TL INFO 0x%x\n", qPair->request_queue_info.mailbox_tail_ptr_offset);
    printf("XRT-MB REQ BA INFO 0x%x\n", qPair->request_queue_info.buffer_start_address);
    printf("XRT-MB REQ BL INFO 0x%x\n", qPair->request_queue_info.buffer_size);

    printf("XRT-MB RSP HD INFO 0x%x\n", qPair->response_queue_info.mailbox_head_ptr_offset);
    printf("XRT-MB RSP TL INFO 0x%x\n", qPair->response_queue_info.mailbox_tail_ptr_offset);
    printf("XRT-MB RSP BA INFO 0x%x\n", qPair->response_queue_info.buffer_start_address);
    printf("XRT-MB RSP BL INFO 0x%x\n", qPair->response_queue_info.buffer_size);

    if(passed && create_context_resp.num_command_queue_pairs_allocated > 0) {
        IpuHenvRing rb(qPair->request_queue_info, qPair->response_queue_info, 0);

        TEST_IpuExecuteBuffer(&rb);
        TEST_IpuExecuteBuffer(&rb);
        TEST_IpuExecuteBuffer(&rb);
        TEST_IpuExecuteBuffer(&rb);
        TEST_IpuExecuteBuffer(&rb);
    } else {
        printf("[HENV] !!! Failed to create any command pairs!\n");
    }
    TEST_FN_RESULT(passed);
}

void TEST_IpuMapHostBuffer(IpuHenvRing *pMngBuff, uint32_t context_id)
{
    TEST_FN_HEADER;
    map_host_buffer_req_t map_host_buffer_req = { 0 };
    map_host_buffer_resp_t map_host_buffer_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    // simple pattern for sanity checks
    for(size_t i=0; i<sizeof(xcl_bin_data); i++)
        xcl_bin_data[i] = ~i;

    // Dummy test data for the buffer access test
    WR_SYSHUB(TEST_DRAM_BASE_ADDR + 0x8000, xcl_bin_data, sizeof(xcl_bin_data));

    map_host_buffer_req.context_id = context_id;
    map_host_buffer_req.buffer_address = TEST_DRAM_BASE_ADDR + 0x8000;
    map_host_buffer_req.buffer_size = sizeof(xcl_bin_data);


    bool passed = RINGB_Command(map_host_buffer_req, &map_host_buffer_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_MAP_HOST_BUFFER
                                , "IPU_MSG_MAP_HOST_BUFFER", __FUNCTION__);
    TEST_FN_RESULT(passed);
}

void TEST_IpuDeleteContextNegative(IpuHenvRing *pMngBuff, uint32_t context_id)
{
    TEST_FN_HEADER;
    delete_context_req_t delete_context_req;
    delete_context_resp_t delete_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    delete_context_req.context_id = 8;

    bool passed = RINGB_Command(delete_context_req, &delete_context_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT
                                , "IPU_MSG_DELETE_CONTEXT", __FUNCTION__, false);
    TEST_FN_RESULT(passed);
}

void TEST_IpuDeleteContext(IpuHenvRing *pMngBuff, uint32_t context_id)
{
    TEST_FN_HEADER;
    delete_context_req_t delete_context_req = { 0 };
    delete_context_resp_t delete_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    delete_context_req.context_id = context_id;

    bool passed = RINGB_Command(delete_context_req, &delete_context_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT
                                , "IPU_MSG_DELETE_CONTEXT", __FUNCTION__);
    TEST_FN_RESULT(passed);
}

void TEST_IpuGetTelemetry(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    get_telemetry_req_t get_telemetry_req = { TELEMETRY_TYPE_DEBUG };
    get_telemetry_resp_t get_telemetry_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    RINGB_Command(get_telemetry_req, &get_telemetry_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_GET_TELEMETRY
                                , "IPU_MSG_GET_LOG_PAGE", __FUNCTION__);
}

void TEST_IpuResetPartition(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    reset_partition_req_t reset_partition_req;
    reset_partition_resp_t reset_partition_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    reset_partition_req.part_info.start_column = 0;
    reset_partition_req.part_info.total_columns = 2;
    reset_partition_req.part_info.aie_type = IPU_AIE2;

    bool passed = RINGB_Command(reset_partition_req, &reset_partition_resp, pMngBuff, 0xFA5EFADE
                                , IPU_MSG_RESET_PARTITION, "IPU_MSG_RESET_PARTITION", __FUNCTION__);
    TEST_FN_RESULT(passed);
}

#define WAIT_FOR_WAITI_TOGGLE 10000
void TEST_IpuSuspend(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    suspend_req_t suspend_req = { 0 };
    suspend_resp_t suspend_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    uint32_t suspRdy = RD_SMN_MMIO_ADDR(MPIPU_PMFW_SUSP_RDY);
    uint32_t waitRdy = RD_SMN_MMIO_ADDR(MPIPU_PWAITMODE);
    int32_t waitForReadyFlag = WAIT_FOR_WAITI_TOGGLE;
    printf("[SUSPEND] PRE suspRdy=0x%X waitRdy=0x%X\n", suspRdy, waitRdy);

    // Check that the IPU is operational, both regs cannot be 1
    bool ready = !(1 == (suspRdy & 1) && 1 == (waitRdy & 1));
    bool passed = false;

    if(ready) {
        passed = RINGB_Command(suspend_req, &suspend_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_SUSPEND
                               , "IPU_MSG_SUSPEND", __FUNCTION__);
    }

    if(ready && passed) {
        while((0 == (suspRdy = RD_SMN_MMIO_ADDR(MPIPU_PMFW_SUSP_RDY)) ||
                0 == (waitRdy = RD_SMN_MMIO_ADDR(MPIPU_PWAITMODE))) &&
                waitForReadyFlag-- > 0);
        printf("[SUSPEND] POST suspRdy=0x%X waitRdy=0x%X waited=%i\n", suspRdy, waitRdy
               , (WAIT_FOR_WAITI_TOGGLE - waitForReadyFlag));
        // We passed if both regs are non-zero
        passed = (suspRdy && waitRdy);
    }

    TEST_FN_RESULT(passed);
}

void TEST_IpuResume(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    resume_req_t resume_req = { 0 };
    resume_resp_t resume_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    uint32_t suspRdy = RD_SMN_MMIO_ADDR(MPIPU_PMFW_SUSP_RDY);
    uint32_t waitRdy = RD_SMN_MMIO_ADDR(MPIPU_PWAITMODE);
    int32_t  waitForReadyFlag = WAIT_FOR_WAITI_TOGGLE;
    printf("[RESUME] PRE suspRdy=0x%X waitRdy=0x%X\n", suspRdy, waitRdy);

    // Check that the IPU is suspended, both regs should be 1
    bool ready = (1 == (suspRdy & 1) && 1 == (waitRdy & 1));
    bool passed = false;

    if(ready) {
        passed = RINGB_Command(resume_req, &resume_resp, pMngBuff, 0xFA5EFADE, IPU_MSG_RESUME
                               , "IPU_MSG_RESUME", __FUNCTION__);
    }

    if(ready && passed) {
        while((0 != (suspRdy = RD_SMN_MMIO_ADDR(MPIPU_PMFW_SUSP_RDY)) ||
                0 != (waitRdy = RD_SMN_MMIO_ADDR(MPIPU_PWAITMODE))) &&
                waitForReadyFlag-- > 0);
        printf("[RESUME] POST suspRdy=0x%X waitRdy=0x%X waited=%i\n", suspRdy, waitRdy
               , (WAIT_FOR_WAITI_TOGGLE - waitForReadyFlag));
        // We passed if both regs are 0
        passed = (!suspRdy && !waitRdy);
    }

    TEST_FN_RESULT(passed);
}

void TEST_IpuAssignMgmtPasid(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    assign_mgmt_pasid_req_t assign_mgmt_pasid_req = { 0 };
    assign_mgmt_pasid_resp_t assign_mgmt_pasid_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    bool passed = RINGB_Command(assign_mgmt_pasid_req, &assign_mgmt_pasid_resp, pMngBuff, 0xFA5EFADE
                                , IPU_MSG_ASSIGN_MGMT_PASID, "IPU_MSG_ASSIGN_MGMT_PASID", __FUNCTION__);
    TEST_FN_RESULT(passed);
}

void TEST_IpuCheckHeaderHash(IpuHenvRing *pMngBuff)
{
    TEST_FN_HEADER;
    check_header_hash_req_t check_header_hash_req = { 0 };
    check_header_hash_resp_t check_header_hash_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    check_header_hash_req.hash_high = 0;
    check_header_hash_req.hash_low = 0;

    {
        uint8_t *p =(uint8_t*) &check_header_hash_req.hash_high;
        printf("TEST_HashCheck\n");
        for(int i=0; i<16; i++) {
            printf("%02d -- %02X\n", i, p[i]);
        }
    }

    bool passed = RINGB_Command(check_header_hash_req, &check_header_hash_resp, pMngBuff, 0xFA5EFADE
                                , IPU_MSG_CHECK_HEADER_HASH, "IPU_MSG_CHECK_HEADER_HASH", __FUNCTION__);
    TEST_FN_RESULT(passed);
}

IpuHenvRing *XRT_WaitForERT(uint64_t io_hdl)
{
    os_ipu_mnmg_u mngInfo;
    printf("[XRT] XRT_WaitForERT() ALIVE_PTR == 0x%08X\n", ALIVE_PTR);

    // uint32_t rv = RD_SMN_ADDR(ALIVE_PTR);
    uint32_t rv = ipurb_mem_read32(io_hdl, ALIVE_PTR);
    printMsg("[XRT] Waiting on IPU Alive indicator");
    while(rv==0x0) {
        // rv = RD_SMN_ADDR(ALIVE_PTR);
        rv = ipurb_mem_read32(io_hdl, ALIVE_PTR);
    }
    printMsg("[XRT] IPU Alive indicator showed up");
    cout << "[XRT] RV=" << hex << rv << endl;
    printMsg("[XRT] Ack alive message");
    for(uint8_t i=0; i< sizeof(os_ipu_mnmg_t)/4; i++) {
        // mngInfo.d[i]=RD_SMN_ADDR(X2I_SMN_MMIO_ADDR(rv)+(i*4));
        mngInfo.d[i]=ipurb_mem_read32(io_hdl, (X2I_SMN_MMIO_ADDR(rv)+(i*4)));

        cout << "[XRT] Data = " << hex << mngInfo.d[i] << endl;
    }
    // WR_SMN_ADDR(ALIVE_PTR,0x0);
    ipurb_mem_write32(io_hdl, ALIVE_PTR, 0x0);
    cout << "[XRT] (X2E)Tail Ptr = " << hex << mngInfo.f.os_to_ipu_ch.tail_ptr << endl;
    cout << "[XRT] (X2E)Head Ptr = " << hex << mngInfo.f.os_to_ipu_ch.head_ptr << endl;
    cout << "[XRT] (X2E)Buffer Ptr = " << hex << mngInfo.f.os_to_ipu_ch.buffer_ptr<< endl;
    cout << "[XRT] (X2E)Buffer Size = " << hex << mngInfo.f.os_to_ipu_ch.buffer_size << endl;
    cout << "[XRT] (E2X)Tail Ptr = " << hex << mngInfo.f.ipu_to_os_ch.tail_ptr << endl;
    cout << "[XRT] (E2X)Head Ptr = " << hex << mngInfo.f.ipu_to_os_ch.head_ptr << endl;
    cout << "[XRT] (E2E)Buffer Ptr = " << hex << mngInfo.f.ipu_to_os_ch.buffer_ptr<< endl;
    cout << "[XRT] (E2X)Buffer Size = " << hex << mngInfo.f.ipu_to_os_ch.buffer_size << endl;
    printf("[XRT] XRT_WaitForERT() ALIVE_PTR == 0x%08X\n", ALIVE_PTR);

    IpuHenvRing *pIpuMngInfoRing = new IpuHenvRing(mngInfo.f.os_to_ipu_ch, mngInfo.f.ipu_to_os_ch, io_hdl);
    return pIpuMngInfoRing;
}

static void PrintFinalTestSummary()
{
    uint32_t totalFail = 0;
    printf("Final Test Summary:\n");
    printf("----------------------------------\n");
    for(auto testResult : TestResults) {
        if(!testResult.m_Passed) {
            ++totalFail;
            testResult.PrintResult();
        }
    }
    printf("----------------------------------\n");
    printf("%4u Tests Failed\n", totalFail);
    // printf("%4u Tests Passed\n", TestResults.size()-totalFail);
    printf("----------------------------------\n");
}

/*---------------------------------------------------------------------------------------*/
/**
 * Primary entry point for SimNow FSDL host environment.
 */

void FsdlMain()
{
    uint32_t context_id = 0;
    // make sure all template functions used by Xilinx XRT are instantiated.
    execute_buffer_req_t exec_buf_req = { 0 };
    execute_buffer_resp_t exec_buf_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    bool lpassed = RINGB_Command(exec_buf_req, &exec_buf_resp, nullptr, 0xFA5EFADE, IPU_MSG_EXECUTE_BUFFER
                                , "IPU_MSG_EXECUTE_BUFFER", __FUNCTION__);
    delete_context_req_t delete_context_req = { 0 };
    delete_context_resp_t delete_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(delete_context_req, &delete_context_resp, nullptr, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT
                                , "IPU_MSG_DELETE_CONTEXT", __FUNCTION__, false);
 
    register_xcl_bin_req_t load_xclbin_req = { 0 };
    register_xcl_bin_resp_t load_xclbin_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(load_xclbin_req, &load_xclbin_resp, nullptr, 0xFA5EFADE, IPU_MSG_REGISTER_XCL_BIN,
		"IPU_MSG_REGISTER_XCL_BIN", __FUNCTION__);

    create_context_req_t create_context_req;
    create_context_resp_t create_context_resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(create_context_req, &create_context_resp, nullptr, 0xFA5EFADE, IPU_MSG_CREATE_CONTEXT,
		"IPU_MSG_CREATE_CONTEXT", __FUNCTION__);
 
    map_host_buffer_req_t mreq = { 0 };
    map_host_buffer_resp_t mresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(mreq, &mresp, nullptr, 0xFA5EFADE, IPU_MSG_MAP_HOST_BUFFER,
		"IPU_MSG_MAP_HOST_BUFFER", __FUNCTION__);
 
    sync_bo_req_t sreq;
    sync_bo_resp_t sresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(sreq, &sresp, nullptr, 0xFA5EFADE, IPU_MSG_SYNC_BO,
		"IPU_MSG_MAP_SYNC_BO", __FUNCTION__);

    unregister_xcl_bin_req_t ureq;
    unregister_xcl_bin_resp_t uresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    lpassed = RINGB_Command(ureq, &uresp, nullptr, 0xFA5EFADE, IPU_MSG_UNREGISTER_XCL_BIN,
                "IPU_MSG_UNREGISTER_XCL_BIN", __FUNCTION__);

    printf("lpassed is %d\n", lpassed);
    // End of the instantiation

    IpuHenvRing *pMngBuff = nullptr;
    cout << "[HENV] FsdlMain start" << endl;
    pMngBuff = XRT_WaitForERT(0);

    uint64_t dram_lo = RD_SMN_MMIO_ADDR(MPIPU_TEST_DRAM_LO);
    uint64_t dram_hi = RD_SMN_MMIO_ADDR(MPIPU_TEST_DRAM_HI);
    TEST_DRAM_BASE_ADDR  = (dram_lo) | (dram_hi << 32);
    TEST_DRAM_LIMIT_ADDR = TEST_DRAM_BASE_ADDR + TEST_DRAM_SIZE - 1;

    if(pMngBuff) {
#ifdef XILINX_DPU_SELF_TEST
        TEST_DPULoadXclBin(pMngBuff);
        // TEST_DPUSelfTest(pMngBuff);
        TEST_DPUUserSelfTest(pMngBuff);
#else
        TEST_IpuLoadXclBin(pMngBuff);
        TEST_IpuDeleteContextNegative(pMngBuff, context_id);
        TEST_IpuCreateContext(pMngBuff, &context_id);
        TEST_IpuMapHostBuffer(pMngBuff, context_id);

        TEST_IpuDeleteContext(pMngBuff, context_id);
        TEST_IpuUnregisterXclBin(pMngBuff);
        TEST_IpuGetTelemetry(pMngBuff);
        TEST_IpuResetPartition(pMngBuff);

        // Emulation is reporting a problem on second test run, after suspend/resume so add the first test here
        TEST_IpuInvokeSelfTest(pMngBuff);

        TEST_IpuSuspend(pMngBuff);
        TEST_IpuResume(pMngBuff);
        TEST_IpuCheckHeaderHash(pMngBuff);
        // Emulation is reporting a problem on second test run, after suspend/resume so add extra self test here for
        // consistency
        TEST_IpuInvokeSelfTest(pMngBuff);

        // Resurrect applciation
        TEST_IpuLoadXclBin(pMngBuff);
        TEST_IpuCreateContext(pMngBuff, &context_id);

#if 1
        // Test ring buffer wrap behavior
        uint32_t test = 0;
        while(pMngBuff->Rx().GetTailIndex() > 0x100) {
            printf("[RING] Wrap Test %u\n", ++test);
            TEST_IpuAssignMgmtPasid(pMngBuff);
        }
        const uint32_t totalTests = test + 5;
        for(uint32_t x = test; x < totalTests; ++x) {
            printf("[RING] Wrap Test %u/%u\n", ++test, totalTests);
            TEST_IpuAssignMgmtPasid(pMngBuff);
            pMngBuff->Rx().DebugPrint("[RING] wrap");
            pMngBuff->Tx().DebugPrint("[RING] wrap");
        }
#endif

#endif
    }

    printf("Debug Reg: 0x%08X\n", RD_SMN_MMIO_ADDR(MPIPU_POST_REG));
    cout << "[HENV] FsdlMain finish" << endl;

    PrintFinalTestSummary();
}

