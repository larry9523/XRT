/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2021, Xilinx Inc
 * Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#ifndef __XGQ_HWEMU_H__
#define __XGQ_HWEMU_H__

#include <boost/pool/object_pool.hpp>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#include "core/include/xrt/xrt_bo.h"
#include "em_defines.h"
#include "ert.h"
#include "xgq_cmd_common.h"
#include "xgq_hwemu_plat.h"

namespace xclhwemhal2 {
  class HwEmShim;
}

constexpr uint64_t XRT_QUEUE1_RING_BASE = 0x7B000;
constexpr uint32_t XRT_QUEUE1_RING_LENGTH = 0x5000; // hard code for now 20K

constexpr uint64_t XRT_XGQ_SUB_BASE = 0x1040000;
constexpr uint64_t XRT_XGQ_COM_BASE = 0x1030000;

namespace hwemu {

  //! Forward declaration
  class xgq_cmd;
  class xocl_xgq;

  /**
   * class xgq_queue: Represent a XRT Generic Queue pair.
   *
   * @submit_worker():          submission queue worker thread
   * @complete_worker():        completion queue worker thread
   * @update_doorbell():        update sub_tail to submission XGQ doorbell
   * @check_doorbell():         check com_tail in completion XGQ doorbell
   * @submit_cmd():             put a command into submission queue entry
   * @read_completion():        read a completion entry from completion queue
   * @iowrite32_ctrl():         write 32 bits to an IO CTRL address
   * @iowrite32_mem():          write 32 bits to an IO MEM address
   * @ioread32_ctrl():          read 32 bits value from an IO CTRL address
   * @ioread32_mem():           read 32 bits value from an IO MEM address
   *
   * @pending_cmds:             a list of commands waiting to be submitted
   * @submitted_cmds:           a hash map of commands sent but not yet
   *                            completed
   */
  class xgq_queue
  {
    public:
      xgq_queue(xclhwemhal2::HwEmShim*, xocl_xgq*, uint16_t, uint32_t, uint64_t, uint64_t);
      ~xgq_queue();

      xclhwemhal2::HwEmShim*   device;
      xocl_xgq*                xgqp;

      int      submit_worker();
      int      complete_worker();
      void     update_doorbell();
      int      submit_cmd(xgq_cmd *xcmd);
      void     read_completion(xgq_com_queue_entry& ccmd, uint64_t addr);
      void     iowrite32_ctrl(uint32_t addr, uint32_t data);
      void     iowrite32_mem(uint32_t addr, uint32_t data);
      uint32_t ioread32_ctrl(uint32_t addr);
      uint32_t ioread32_mem(uint32_t addr);

      uint16_t        qid;
      uint16_t        nslot;
      uint32_t        slot_size;

      uint64_t        xgq_sub_base;
      uint64_t        xgq_com_base;

      std::list<xgq_cmd*>          pending_cmds;
      std::map<uint64_t, xgq_cmd*> submitted_cmds;
      std::mutex                   queue_mutex;
      bool                         stop;

      std::thread*            sub_thread;
      std::condition_variable sub_cv;
      std::thread*            com_thread;
      std::condition_variable com_cv;

      struct xgq       queue;
  };

  /**
   * class xgq_cmd: Represent a command in XGQ. It contains the execbuf sent
   *                from the xclExecBuf (ert_packet BO) and XGQ command.
   *
   * @opcode():          ert_packet opcode
   * @set_state():       set ert_packet state
   * @convert_bo():      convert ert_packet to XGQ command packet
   * @payload_size():    ert_packet payload size in bytes
   * @xcmd_size():       XGQ command total size in bytes
   */
  class xgq_cmd
  {
    public:
      xgq_cmd();

      uint32_t    opcode();
      void        set_state(enum ert_cmd_state state);
      uint32_t    payload_size();
      bool        is_ertpkt();

      int         convert_bo(xclemulation::drm_xocl_bo *bo);
      int         load_xclbin(xrt::bo& xbo, char *buf, size_t size);

      uint32_t    xcmd_size();

      uint16_t              cmdid;
      std::vector<uint32_t> sq_buf;
      struct ert_packet     *ert_pkt;
      int                   rval;

      std::mutex              cmd_mutex;
      std::condition_variable cmd_cv;

      //! Static member varibale
      //  to get the unique ID for each command
      static uint64_t next_uid;
  };

  /**
   * class xocl_xgq:    The main class of XGQ.
   *
   * @add_exec_buffer:  Convert an exec buf to XGQ command, add it to XGQ
   *                    pending command list and notify XGQ thread.
   */
  class xocl_xgq
  {
    public:
      xocl_xgq(xclhwemhal2::HwEmShim* dev);
      ~xocl_xgq();

      int    add_exec_buffer(xclemulation::drm_xocl_bo *buf);
      int    load_xclbin(char *buf, size_t size);

      // TODO support multiple queues
      xgq_queue queue;

      boost::object_pool<xgq_cmd> cmd_pool;

      xclhwemhal2::HwEmShim*   device;
  };

}  // namespace hwemu

#endif
