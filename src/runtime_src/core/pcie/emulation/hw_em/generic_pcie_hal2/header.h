/*
* Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
*/

#ifndef HEADER_H
#define HEADER_H
typedef struct {
    uint32_t alive_ptr;
    uint32_t major;
    uint32_t minor;
    uint32_t sub;
    uint32_t build;
    uint64_t api_hash_high;
    uint64_t api_hash_low;
    char     desc[216];
} IPU_Firmware_Header;

extern  IPU_Firmware_Header volatile firmware_header  __attribute__((section(".firmwareheader")));

#endif

