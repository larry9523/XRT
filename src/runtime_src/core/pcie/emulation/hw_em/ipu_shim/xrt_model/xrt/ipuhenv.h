#pragma once

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "host_env.h"

//
// IPU SMN Base Addresses from the Phoenix SMN memory map (scf_smn_map.json)
//
#define IPU_SMN_MMIO_BASE_ADDR 0x1F600000ull
#define IPU_SMN_MMIO_MASK      0x000FFFFFull
#define IPU_SMN_SRAM_BASE_ADDR 0x1F700000ull  // TODO: BOZO: FIXME: Not sure if this is needed/correct
#define IPU_SMN_SRAM_MASK      0x000FFFFFull  // TODO: BOZO: FIXME: Not sure if this is needed/correct

#define X2I_SMN_MMIO_ADDR(X) (( X & IPU_SMN_MMIO_MASK ) | IPU_SMN_MMIO_BASE_ADDR)
#define X2I_SMN_SRAM_ADDR(X) (( X & IPU_SMN_SRAM_MASK ) | IPU_SMN_SRAM_BASE_ADDR)

#define RD_SMN_MMIO_ADDR(ADDR)     RD_SMN_ADDR(X2I_SMN_MMIO_ADDR(ADDR))
#define WR_SMN_MMIO_ADDR(ADDR,VAL) WR_SMN_ADDR(X2I_SMN_MMIO_ADDR(ADDR),VAL)

#define RD_SMN_MMIO_ADDR_XRT(HDL,ADDR)     ipurb_mem_read32(HDL, X2I_SMN_MMIO_ADDR(ADDR))
#define WR_SMN_MMIO_ADDR_XRT(HDL,ADDR,VAL) ipurb_mem_write32(HDL, X2I_SMN_MMIO_ADDR(ADDR),VAL)
