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

//
// Constant string for the symbolic link associated with the device
//
#define XRT_CORE_SYMBOLIC_LINK_NAME L"\\Global??\\XRT-USER-0"

#define XRT_USER_DEVICE_BUFFER_OBJECT_NAMESPACE L"\\Buffer"
#define XRT_USER_DEVICE_DEVICE_NAMESPACE        L"\\Device"

#define XOCL_DEFAULT_ERROR_CAPACITY 32

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

enum xocl_err_ops {
    XOCL_ERROR_OP_INJECT = 1,
    XOCL_ERROR_OP_CLEAR_ALL
};

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

typedef struct _XRT_HW_CTX_ARGS {
    XRT_CTX_OPERATION  Operation;   // IN: Alloc or free context
    GUID               XclBinUuid;  // IN: XCLBIN to acquire a context on
    ULONG              SlotIdx;     // IN: Slot index
    ULONG              QoS;         // IN: Qos Value (TBD)
} XRT_HW_CTX_ARGS, * PXRT_HW_CTX_ARGS;

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
    uint64_t    offset;
    uint64_t    size;
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
    uint64_t                range;
    uint64_t                anums;
    struct argument_info    args[1];
};

/**
 * struct aie_info - AIE partition info
 *
 * @npart: number of partition overlays
 * @ncol: number of columns in this partition
 * @start_col_list: Array of start column for partition relocation
 * @name:  partition name
 */
struct aie_info {
    char                    name[64];
    uint32_t                npart;
    uint32_t                ncol;
    uint32_t                start_col_list[1];
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
    struct xrt_kds      kds_cfg;
    uint64_t            ksize;
    uint64_t            asize;
    CHAR                data[1]; //data section will have both kernel_info and aie_metadata
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
    XrtStatXclinSlots,
    XrtStatSlotInfo
} XRT_STAT_CLASS, * PXRT_STAT_CLASS;

typedef struct _XRT_STAT_CLASS_ARGS {

    XRT_STAT_CLASS StatClass;

} XRT_STAT_CLASS_ARGS, * PXRT_STAT_CLASS_ARGS;


//
// XrtKdsIoctlHwCtx
//
typedef struct _XRT_SLOT_INFORMATION {
    ULONG        SlotIdx;
} XRT_SLOT_INFORMATION, * PXRT_SLOT_INFORMATION;

//
// XrtStatXclbinSlots
//
typedef struct _XRT_SLOT_COUNT {
    ULONG     CuCount;
    ULONG     SlotCount;
} XRT_SLOT_COUNT, * PXRT_SLOT_COUNT;

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
 * struct _XOCL_ERROR_INJECT_ARGS - Aie error injection
 */
typedef struct _XOCL_ERROR_INJECT_ARGS {
    uint16_t     err_ops;
    uint16_t     err_num;
    uint16_t     err_driver;
    uint16_t     err_severity;
    uint16_t     err_module;
    uint16_t     err_class;
}XOCL_ERROR_INJECT_ARGS, * PXOCL_ERROR_INJECT_ARGS;

struct xocl_err_record {
    xrtErrorCode    xer_err_code;   /* XRT error code */
    uint64_t        xer_ts;         /* timestamp */
    uint32_t        pid;            /* 32 bits; pid associated with error, if available */
};

/**
 * struct _XOCL_ERRORS_USER_ARGS - Pass Aie error to user space
 */
typedef struct _XOCL_ERRORS_USER_ARGS {
    int     num_err;    /* number of errors recorded */
    struct xocl_err_record errors[XOCL_DEFAULT_ERROR_CAPACITY];  /* error array pointer */
} XOCL_ERRORS_USER_ARGS, * PXOCL_ERRORS_USER_ARGS;


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

