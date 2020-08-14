/**
 * Copyright (C) 2020 Xilinx, Inc
 * Author(s): Larry Liu
 * ZNYQ XRT Library layered on top of ZYNQ zocl kernel driver
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef xrt_core_edge_user_aie_h
#define xrt_core_edge_user_aie_h

#include <vector>
#include <queue>

#include "core/common/device.h"
#include "core/edge/common/aie_parser.h"
extern "C" {
#include <xaiengine.h>
#ifdef AIE_V2
#include "aie_v1.h"
#endif

}

#define HW_GEN                   XAIE_DEV_GEN_AIE
#define XAIE_NUM_ROWS            9
#define XAIE_NUM_COLS            50
#define XAIE_BASE_ADDR           0x20000000000
#define XAIE_COL_SHIFT           23
#define XAIE_ROW_SHIFT           18
#define XAIE_SHIM_ROW            0
#define XAIE_MEM_TILE_ROW_START  0
#define XAIE_MEM_TILE_NUM_ROWS   0
#define XAIE_AIE_TILE_ROW_START  1
#define XAIE_AIE_TILE_NUM_ROWS   8

#define XAIEGBL_NOC_DMASTA_STARTQ_MAX 4
#define CONVERT_LCHANL_TO_PCHANL(l_ch) (l_ch > 1 ? l_ch - 2 : l_ch)

namespace zynqaie {

struct BD {
    uint16_t bd_num;
    uint16_t addr_high;
    uint32_t addr_low;
};

struct DMAChannel {
    std::queue<BD> idle_bds;
    std::queue<BD> pend_bds;
};

struct ShimDMA {
    XAie_DmaDesc desc;
    XAieDma_Shim handle;
    DMAChannel dma_chan[XAIEDMA_SHIM_MAX_NUM_CHANNELS];
    bool configured;
};


class Aie {
public:
    using gmio_type = xrt_core::edge::aie::gmio_type;

    ~Aie();
    Aie(std::shared_ptr<xrt_core::device> device);

    std::vector<XAieGbl_Tile> tileArray;  // Tile Array
    std::vector<ShimDMA> shim_dma;   // shim DMA

    /* This is the collections of gmios that are used. */
    std::vector<gmio_type> gmios;

    int getTilePos(int col, int row);

    XAie_DevInst *getDevInst();

    XAieGbl *getAieInst();

    static XAieGbl_ErrorHandleStatus
    error_cb(struct XAieGbl *aie_inst, XAie_LocType loc, u8 module, u8 error, void *arg);

private:
    int numRows;
    int numCols;
    int fd;
    uint64_t aieAddrArrayOff;

    XAie_DevInst devInst;         // AIE Device Instance
    XAieGbl_Config *aieConfigPtr; // AIE configuration pointer
    XAieGbl aieInst;              // AIE global instance
    XAieGbl_HwCfg aieConfig;      // AIE configuration pointer
};

}

#endif
