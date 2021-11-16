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


#define XCL_UUID_SZ     16

//
// XoclStatDevice
//
typedef struct _XOCL_DEVICE_INFORMATION {
    ULONG  DeviceNumber;
    USHORT Vendor;
    USHORT Device;
    USHORT SubsystemVendor;
    USHORT SubsystemDevice;
    ULONG  DmaEngineVersion;
    ULONG  DriverVersion;
    ULONG  PciSlot;
    ULONG  MaximumLinkWidth;
    ULONG  LinkWidth;
    ULONG  MaximumLinkSpeed;
    ULONG  LinkSpeed;

} XOCL_DEVICE_INFORMATION, *PXOCL_DEVICE_INFORMATION;

/**
 * struct xcl_pr_region - Data structure used to fetch ICAP group
 */
struct xcl_pr_region {
    uint64_t freq_0;
    uint64_t freq_1;
    uint64_t freq_2;
    uint64_t freq_3;
    uint64_t freq_cntr_0;
    uint64_t freq_cntr_1;
    uint64_t freq_cntr_2;
    uint64_t freq_cntr_3;
    uint64_t idcode;
    uint8_t uuid[XCL_UUID_SZ];
    uint64_t mig_calib;
    uint64_t data_retention;
};

/**
 * struct xcl_mig_ecc -  Data structure used to fetch MIG_ECC group
 */
struct xcl_mig_ecc {
    uint64_t mem_type;
    uint64_t mem_idx;
    uint64_t ecc_enabled;
    uint64_t ecc_status;
    uint64_t ecc_ce_cnt;
    uint64_t ecc_ue_cnt;
    uint64_t ecc_ce_ffa;
    uint64_t ecc_ue_ffa;
};

/**
 * struct xcl_firewall -  Data structure used to fetch FIREWALL group
 */
struct xcl_firewall {
    uint64_t max_level;
    uint64_t curr_status;
    uint64_t curr_level;
    uint64_t err_detected_status;
    uint64_t err_detected_level;
    uint64_t err_detected_time;
};

/**
 * struct xcl_dna -  Data structure used to fetch DNA group
 */
struct xcl_dna {
    uint64_t status;
    uint32_t dna[4];
    uint64_t capability;
    uint64_t dna_version;
    uint64_t revision;
};

/**
 * struct xcl_mailbox -  Data structure used to fetch mailbox group
 */
struct xcl_mailbox {
    /* recv metrics */
    uint64_t          mbx_recv_raw_bytes;
    uint64_t          mbx_recv_req[16];
};

struct drm_xocl_mm_stat {
    size_t memory_usage;
    unsigned int bo_count;
};
