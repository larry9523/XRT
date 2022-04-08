/******************************************************************************
 *  SPDX-License-Identifier: Apache-2.0
 *  (C) Copyright 2019-2021 Xilinx, Inc.
 *  (C) Copyright 2019-2021 OSR Open Systems Resources, Inc.
 *  All Rights Reserved
 *
 *  This notice is intended as a precaution against inadvertent publication
 *  and does not imply publication or any waiver of confidentiality.
 *  The year included in the foregoing notice is the year of creation of the work.
 *
 *
 *  Module Name    xrt-intf.hpp
 *  Project        IPU Kernel Mode Driver
 *
 *  Description    IOCTL interface definitions for the  XRT header
 *
 *
 ******************************************************************************/
#pragma once

#include <initguid.h>
#include <stdint.h>

#define FILE_DEVICE_XRT_USER   ((ULONG)0x8879)   // "XO"

//
// Constant string for the symbolic link associated with the device
//
#define XRT_CORE_SYMBOLIC_LINK_NAME L"\\Global??\\XRT-USER-0"

#define XRT_USER_DEVICE_BUFFER_OBJECT_NAMESPACE L"\\Buffer"
#define XRT_USER_DEVICE_DEVICE_NAMESPACE        L"\\Device"

typedef enum _XRT_BUFFER_SYNC_DIRECTION {

    XRT_BUFFER_DIRECTION_TO_DEVICE = 0,
    XRT_BUFFER_DIRECTION_FROM_DEVICE

} XRT_BUFFER_SYNC_DIRECTION, * PXRT_BUFFER_SYNC_DIRECTION;

typedef enum _XRT_BUFFER_TYPE {

    XRT_BUFFER_TYPE_NONE = 0,
    XRT_BUFFER_TYPE_NORMAL = 0x3323,
    XRT_BUFFER_TYPE_USERPTR,
    XRT_BUFFER_TYPE_IMPORT,
    XRT_BUFFER_TYPE_CMA,   // NOT USED
    XRT_BUFFER_TYPE_P2P,
    XRT_BUFFER_TYPE_EXECBUF,
    XRT_BUFFER_TYPE_HOST_ONLY,
    XRT_BUFFER_TYPE_DEVICE_ONLY

} XRT_BUFFER_TYPE, * PXRT_BUFFER_TYPE;

#define XRT_MAX_DDR_BANKS    4

typedef enum _XRT_CTX_OPERATION {

    XRT_CTX_OP_ALLOC_CTX,
    XRT_CTX_OP_FREE_CTX

}XRT_CTX_OPERATION, * PXRT_CTX_OPERATION;

/* Context properties */
#define XRT_CTX_PROP_MASK	0x0F
#define XRT_CTX_SHARED		0x00
#define XRT_CTX_EXCLUSIVE	0x01

/* Virtual CU index
 * This is useful when there is no need to open a context on hardware CU,
 * but still need to lockdown the xclbin.
 */
#define XRT_CTX_VIRT_CU		0xffffffff

typedef struct _XRT_CTX_ARGS {
    XRT_CTX_OPERATION Operation;   // IN: Alloc or free context
    GUID             XclBinUuid;  // IN: XCLBIN to acquire a context on
    ULONG              CuIndex;     // IN: Compute unit for the request
    ULONG              Flags;       // IN: XOCL_CTX_FLAG_XXX values
    ULONG              SlotIdx;     //IN: Slot index
} XRT_CTX_ARGS, * PXRT_CTX_ARGS;

//
typedef struct _XRT_CREATE_BO_ARGS {
    ULONGLONG               Size;           // IN: Size in bytes of Buffer
    ULONG                   BankNumber;     // IN: SRAM offset
    XRT_BUFFER_TYPE         BufferType;     // IN: Which type of Buffer Object is being created
                                            // Must be "NORMAL" or "EXECBUF"
    ULONG              Flags;               // IN: XOCL  FLAGS
} XRT_CREATE_BO_ARGS, * PXRT_CREATE_BO_ARGS;

typedef struct _XRT_MAP_BO_RESULT {
    PVOID       MappedUserVirtualAddress;         // OUT: User VA of mapped buffer
} XRT_MAP_BO_RESULT, * PXRT_MAP_BO_RESULT;


typedef struct _XRT_SYNC_BO_ARGS {
    ULONGLONG   Size;           // IN: Bytes to read or write
    ULONGLONG   Offset;         // IN: DDR offset, in bytes, for sync operation
    XRT_BUFFER_SYNC_DIRECTION Direction;  // IN: Sync direction (FROM device or TO device)
} XRT_SYNC_BO_ARGS, * PXRT_SYNC_BO_ARGS;

typedef struct _XRT_USERPTR_BO_ARGS {
    PVOID                   Address;        // IN: User VA of buffer for driver to use
    ULONGLONG               Size;           // IN: Size in bytes of buffer
    ULONG                   BankNumber;     // IN: Zero-based DDR bank number to use
    XRT_BUFFER_TYPE         BufferType;     // IN: Which type of Buffer Object is being created
                                            //     Must be "USERPTR"
    ULONG                   Flags;          // IN: XOCL  FLAGS
} XRT_USERPTR_BO_ARGS, * PXRT_USERPTR_BO_ARGS;


typedef struct _XRT_INFO_BO_RESULT {
    ULONGLONG           Size;           // OUT: Size in bytes of the buffer
    ULONGLONG           Paddr;          // OUT: Physical address of associated DDR
    XRT_BUFFER_TYPE    BufferType;      // OUT: Buffer Type
    ULONG              Flags;           // OUT: XOCL FLAGS
} XRT_INFO_BO_RESULT, * PXRT_INFO_BO_RESULT;

#define ICAP_XCLBIN_V2      "xclbin2"

/**
 * struct argument_info - Kernel argument information
 *
 * @name:   argument name
 * @offset: argument offset in CU
 * @size:   argument size in bytes
 * @dir:    input or output argument for a CU
 */
struct argument_info {
    char        name[64];
    size_t      offset;
    size_t      size;
    uint32_t    dir;
};

/**
 * struct kernel_info - Kernel information
 *
 * @name:   kernel name
 * @range:  kernel register range
 * @anums:  number of argument
 * @args:   argument array
 */
struct kernel_info {
    char                    name[64];
    size_t                  range;
    size_t                  anums;
    struct argument_info    args[1];
};

struct xrt_kds {
    uint32_t slot_size;
    uint32_t ert : 1;
    uint32_t polling : 1;
    uint32_t cu_dma : 1;
    uint32_t cu_isr : 1;
    uint32_t cq_int : 1;
    uint32_t dataflow : 1;
    uint32_t rw_shared : 1;
    uint32_t unused : 25;
};

typedef struct _XRT_READ_AXLF_ARGS {
    struct xrt_kds     kds_cfg;
    size_t              ksize;
    CHAR                kernels[1];
} XRT_READ_AXLF_ARGS, * PXRT_READ_AXLF_ARGS;


typedef enum _XRT_STAT_CLASS {

    XrtStatDevice = 0xCC,
    XrtStatMemTopology,
    XrtStatMemRaw,
    XrtStatIpLayout,
    XrtStatKds,
    XrtStatKdsCU,
    XrtStatRomInfo,
    XrtStatDebugIpLayout,
    XrtStatTempByMemTopology,
    XrtStatGroupTopology,
    XrtStatMemStatRaw,
    XrtStatMemStat,
    XrtStatXclinSlots
} XRT_STAT_CLASS, * PXRT_STAT_CLASS;

typedef struct _XRT_STAT_CLASS_ARGS {

    XRT_STAT_CLASS StatClass;

} XRT_STAT_CLASS_ARGS, * PXRT_STAT_CLASS_ARGS;

//
// XrtStatXclbinSlots
//
typedef struct _XRT_SLOT_INFORMATION {
    ULONG     CuCount;
    ULONG     SlotCount;
} XRT_SLOT_INFORMATION, * PXRT_SLOT_INFORMATION;

//
// XoclStatKdsCU
//
typedef struct _XRT_KDS_CU {
    GUID      XclBinUuid;
    ULONGLONG BaseAddress;
    ULONGLONG Usage;
    ULONG     SlotIdx;
    ULONG     CuIdx;
    char      kname[64];
} XRT_KDS_CU, * PXRT_KDS_CU;

typedef struct _XRT_KDS_CU_INFORMATION {
    ULONG       CuCount;
    XRT_KDS_CU  CuInfo[1];
} XRT_KDS_CU_INFORMATION, * PXRT_KDS_CU_INFORMATION;

typedef struct _XRT_EXECBUF_ARGS {
    HANDLE      ExecBO;
} XRT_EXECBUF_ARGS, * PXRT_EXECBUF_ARGS;


typedef struct _XRT_EXECPOLL_ARGS {
    ULONG DelayInMS;        // IN: Poll delay in microseconds
} XRT_EXECPOLL_ARGS, * PXRT_EXECPOLL_ARGS;


typedef struct _XRT_ALLOC_HOST_MEM_ARGS {
    ULONGLONG   SizeToAllocate;     // IN: Size, in bytes, to allocate
} XRT_ALLOC_HOST_MEM_ARGS, * PXRT_ALLOC_HOST_MEM_ARGS;

/**
 * struct xcl_sensor - Data structure used to fetch SENSOR group
 */
struct xcl_sensor {
    uint32_t vol_12v_pex;
    uint32_t vol_12v_aux;
    uint32_t cur_12v_pex;
    uint32_t cur_12v_aux;
    uint32_t vol_3v3_pex;
    uint32_t vol_3v3_aux;
    uint32_t cur_3v3_aux;
    uint32_t ddr_vpp_btm;
    uint32_t sys_5v5;
    uint32_t top_1v2;
    uint32_t vol_1v8;
    uint32_t vol_0v85;
    uint32_t ddr_vpp_top;
    uint32_t mgt0v9avcc;
    uint32_t vol_12v_sw;
    uint32_t mgtavtt;
    uint32_t vcc1v2_btm;
    uint32_t fpga_temp;
    uint32_t fan_temp;
    uint32_t fan_rpm;
    uint32_t dimm_temp0;
    uint32_t dimm_temp1;
    uint32_t dimm_temp2;
    uint32_t dimm_temp3;
    uint32_t vccint_vol;
    uint32_t vccint_curr;
    uint32_t se98_temp0;
    uint32_t se98_temp1;
    uint32_t se98_temp2;
    uint32_t cage_temp0;
    uint32_t cage_temp1;
    uint32_t cage_temp2;
    uint32_t cage_temp3;
    uint32_t hbm_temp0;
    uint32_t cur_3v3_pex;
    uint32_t cur_0v85;
    uint32_t vol_3v3_vcc;
    uint32_t vol_1v2_hbm;
    uint32_t vol_2v5_vpp;
    uint32_t vccint_bram;
    uint32_t version;
    uint32_t oem_id;
    uint32_t vccint_temp;
    uint32_t vol_12v_aux1;
    uint32_t vol_vcc1v2_i;
    uint32_t vol_v12_in_i;
    uint32_t vol_v12_in_aux0_i;
    uint32_t vol_v12_in_aux1_i;
    uint32_t vol_vccaux;
    uint32_t vol_vccaux_pmc;
    uint32_t vol_vccram;
    uint32_t power_warn;
    uint32_t qspi_status;
    uint32_t vccint_vcu_0v9;
    uint32_t heartbeat_count;
    uint64_t heartbeat_err_time;
    uint32_t heartbeat_err_code;
    uint32_t heartbeat_stall;
};


/**
 * struct xcl_board_info - Data structure used to fetch BDINFO group
 */
struct xcl_board_info {
    char    serial_num[256];
    char    mac_addr0[32];
    char    mac_addr1[32];
    char    mac_addr2[32];
    char    mac_addr3[32];
    char    revision[256];
    char    bd_name[256];
    char    bmc_ver[256];
    uint32_t max_power;
    uint32_t fan_presence;
    uint32_t config_mode;
    char exp_bmc_ver[256];
    uint32_t mac_contiguous_num;
    char     mac_addr_first[6];
};

