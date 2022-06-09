/*
* Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.
*/

#ifndef IPU_PLATFORM_H
#define IPU_PLATFORM_H


/* Have up to 16 kernels and 8 bytes to store index, slot, functional information for each
 * First 4 bytes as size of this chunk of stack memory, and configuration flag
 * Initiate the buffer with its size, and 12 bytes are reserved for the future
 * 16*8 = 128, 128 + 4 + 12 = 144
 */
#define IPU_KERNEL_MAX_NUM            (16)
#define IPU_ERT_ARGS_SIZE             (IPU_KERNEL_MAX_NUM*sizeof(uint32_t)*2+16)

#endif
