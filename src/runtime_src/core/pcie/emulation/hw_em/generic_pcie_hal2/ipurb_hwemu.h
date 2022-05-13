/*
 *  Copyright (C) 2021, Xilinx Inc
 *
 *  This file is dual licensed.  It may be redistributed and/or modified
 *  under the terms of the Apache 2.0 License OR version 2 of the GNU
 *  General Public License.
 *
 *  Apache License Verbiage
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  GPL license Verbiage:
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.  This program is
 *  distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 *  License for more details.  You should have received a copy of the
 *  GNU General Public License along with this program; if not, write
 *  to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA 02111-1307 USA
 *
 */

#ifndef IPURB_HWENU_H
#define IPURB_HWENU_H

#include <boost/pool/object_pool.hpp>
#include <cstdint>
#include <list>

#include "ipuhenvring.h"
#include "core/include/xrt/xrt_bo.h"
#include "em_defines.h"
#include "ert.h"

namespace xclhwemhal2 {
  class HwEmShim;
}

namespace hwemu {

  void ipurb_hwemu_mem_write32(uint64_t io_hdl, uint64_t addr, uint32_t val);
  uint32_t ipurb_hwemu_mem_read32(uint64_t io_hdl, uint64_t addr);
  void ipurb_hwemu_reg_write32(uint64_t io_hdl, uint64_t addr, uint32_t val);
  uint32_t ipurb_hwemu_reg_read32(uint64_t io_hdl, uint64_t addr);

  //! Forward declaration
  class ipurb_cmd;
  class xocl_ipurb;

  /**
   * class ipurb_queue: Represent a IPU Generic Ring Buffer queues.
   *
   * Currently, we only support one mgmt task and one user task, where
   * mng_buff and usr_buff point to mgmt ring buffer and user ring
   * buffer. We use m_uuid to represents the XCLBIN downloaded and the
   * context opened upon this XCLBIN.
   */
  class ipurb_queue
  {
    public:
      ipurb_queue(xclhwemhal2::HwEmShim*);
      ~ipurb_queue();

      xclhwemhal2::HwEmShim*   device;
      xocl_ipurb*              ipurbp;

      uint16_t                qid;
      IpuHenvRing*            mng_buff;
      IpuHenvRing*            usr_buff;
      uint32_t                context_id;
  };

  /**
   * class ipurb_cmd: Represent a command in IPU Ring Buffer. It contains
   *                  the execbuf sent from the xclExecBuf (ert_packet BO)
   *                  and IPU Ring Buffer command. Also it contains the
   *                  method to send and receive the command and its
   *                  resonse.
   *
   * @opcode():          ert_packet opcode
   * @set_state():       set ert_packet state
   * @payload_size():    ert_packet payload size in bytes
   *
   * @exec_buf():        send exec_buf command and get its response
   * @load_xclbin():     send load_xclbin command and get its response
   * @open_context():    send open_context command and get its response
   * @sync_bo():         send sync_bo command and get its response
   */
  class ipurb_cmd
  {
    public:
      ipurb_cmd(ipurb_queue *in_qp);

      uint32_t    opcode();
      void        set_state(enum ert_cmd_state state);
      uint32_t    payload_size();

      int         exec_buf(xclemulation::drm_xocl_bo *bo);
      int         load_xclbin(xrt::bo& xbo, char *buf, size_t size, const uuid_t uuid);
      int         unload_xclbin(const uuid_t uuid);
      int         open_context(const uuid_t uuid, uint32_t start_col, uint32_t ncol);
      int         close_context(const uuid_t uuid);
      int         sync_bo(uint64_t dest, uint64_t src, size_t size, size_t seek);

      ipurb_queue*          queuep; // point to the ring buffer to send command

      uint16_t              cmdid;
      struct ert_packet     *ert_pkt;

      //! Static member varibale
      //  to get the unique ID for each command
      static uint64_t next_uid;
  };

  /**
   * class xocl_ipurb:  The main class of Ipu Ring Buffer.
   *
   * @load_xclbin():      Load Xclbin to IPU
   * @open_context():     Create context (create user task)
   * @close_context():    Close context (delete user task)
   * @add_exec_buffer():  Send exec_buf to IPU
   * @alloc_bo():         Alloc shadow BO from DDR for SRAM BO
   * @free_bo():          Free shadw BO from DDR for SRAM BO
   * @sync_bo():          Send sync_bo to IPU
   */
  class xocl_ipurb
  {
    public:
      xocl_ipurb(xclhwemhal2::HwEmShim* dev);
      ~xocl_ipurb();

      int    load_xclbin(char *buf, size_t size, const uuid_t uuid);
      int    unload_xclbin(const uuid_t uuid);
      int    open_context(const uuid_t uuid, unsigned int ip_index, uint32_t start_col, uint32_t ncol);
      int    close_context(const uuid_t uuid, unsigned int ip_index);
      int    add_exec_buffer(xclemulation::drm_xocl_bo *buf);
      int    alloc_bo(size_t size);
      int    free_bo();
      int    sync_bo(uint64_t dest, const void *src, size_t size, size_t seek);

      ipurb_queue queue;

      std::unique_ptr<xrt::bo>      sbo;
      boost::object_pool<ipurb_cmd> cmd_pool;

      xclhwemhal2::HwEmShim*   device;

    private:
      int    nctx;
      pid_t  pid;
      std::list<std::pair<uuid_t, xrt::bo>> xclbin_list;
  };
}  // namespace hwemu

static inline void ipurb_mem_write32(uint64_t io_hdl, uint64_t addr, uint32_t val)
{
  hwemu::ipurb_hwemu_mem_write32(io_hdl, addr, val);
}

static inline uint32_t ipurb_mem_read32(uint64_t io_hdl, uint64_t addr)
{
  return hwemu::ipurb_hwemu_mem_read32(io_hdl, addr);
}

static inline void ipurb_reg_write32(uint64_t io_hdl, uint64_t addr, uint32_t val)
{
  hwemu::ipurb_hwemu_reg_write32(io_hdl, addr, val);
}

static inline uint32_t ipurb_reg_read32(uint64_t io_hdl, uint64_t addr)
{
  return hwemu::ipurb_hwemu_reg_read32(io_hdl, addr);
}
#endif
