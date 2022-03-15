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

#include "shim.h"
#include "ipu_msg.h"
#include "mgmt_msg.h"
#include "app_msg.h"
#include "xrs.h"

extern IpuHenvRing *XRT_WaitForERT(uint64_t io_hdl);
template < typename COMMAND, typename RESPONSE >
bool RINGB_Command(COMMAND command, RESPONSE  *response, IpuHenvRing *pRing,
                   uint32_t msg_id, ipu_msg_opcode_e opcode, const char *cmdStr,
                   const char *fnStr, bool expectSuccess = true);

static int log_helper(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int ret = vprintf(format, ap);
  va_end(ap);

  return ret;
}

struct xrs_helper_func ipurb_xrs_func = {
	.xrs_mem_alloc	= malloc,
	.xrs_mem_free	= free,
	.xrs_log	= log_helper,
};

using namespace xclhwemhal2;

namespace hwemu {

  void ipurb_hwemu_mem_write32(uint64_t io_hdl, uint64_t addr, uint32_t val)
  {
    auto device = reinterpret_cast<xclhwemhal2::HwEmShim *>(io_hdl);
    device->xclWrite(XCL_ADDR_KERNEL_CTRL, addr, (void*)(&val), 4);
  }

  uint32_t ipurb_hwemu_mem_read32(uint64_t io_hdl, uint64_t addr)
  {
    uint32_t value;
    auto device = reinterpret_cast<xclhwemhal2::HwEmShim *>(io_hdl);
    device->xclRead(XCL_ADDR_KERNEL_CTRL, addr, (void*)(&value), 4);

    return value;
  }

  void ipurb_hwemu_reg_write32(uint64_t io_hdl, uint64_t addr, uint32_t val)
  {
    auto device = reinterpret_cast<xclhwemhal2::HwEmShim *>(io_hdl);
    device->xclWrite(XCL_ADDR_KERNEL_CTRL, addr, (void*)(&val), 4);
  }

  uint32_t ipurb_hwemu_reg_read32(uint64_t io_hdl, uint64_t addr)
  {
    uint32_t value;
    auto device = reinterpret_cast<xclhwemhal2::HwEmShim *>(io_hdl);
    device->xclRead(XCL_ADDR_KERNEL_CTRL, addr, (void *)(&value), 4);

    return value;
  }

  ipurb_queue::ipurb_queue(HwEmShim* in_dev)
    : device(in_dev)
  {
    qid = 0;

    auto devp = reinterpret_cast<uint64_t>(device);
    mng_buff = XRT_WaitForERT(devp);
  }

  ipurb_queue::~ipurb_queue()
  {
  }


  //! initialize static variable, which gives unique ID for each cmd
  uint64_t ipurb_cmd::next_uid = 0;

  ipurb_cmd::ipurb_cmd(ipurb_queue* in_qp)
    : queuep(in_qp)
  {
    cmdid = ++next_uid; //! Assign unique ID for each new command
    ert_pkt = nullptr;
  }

  uint32_t ipurb_cmd::opcode()
  {
    return ert_pkt->opcode;
  }

  void ipurb_cmd::set_state(enum ert_cmd_state state)
  {
    ert_pkt->state = state;
  }

  uint32_t ipurb_cmd::payload_size()
  {
    return ert_pkt->count * sizeof(uint32_t);
  }

  int ipurb_cmd::exec_buf(xclemulation::drm_xocl_bo *bo)
  {
    this->ert_pkt = (struct ert_packet *)bo->buf;

    bool passed = true;
    switch (opcode()) {
      case ERT_CONFIGURE:
        break;

      case ERT_START_CU:
        {
          if (payload_size() > 20 * sizeof(uint32_t)) {
            printf("IPURB: fail to send exec buf, payload is too big\n");
            return -EINVAL;
          }

          execute_buffer_req_t req;
          execute_buffer_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

          auto ert_start_cu = reinterpret_cast<ert_start_kernel_cmd *>(ert_pkt);
          memcpy(req.data, ert_start_cu->data, payload_size() - 4);

          passed = RINGB_Command(req, &resp, queuep->usr_buff, 0xFA5EFADE, IPU_MSG_EXECUTE_BUFFER,
		"IPU_MSG_EXECUTE_BUFFER", __FUNCTION__);
          printf("passed is %d\n", passed);

        }
        break;

      case ERT_EXIT:
        break;

      default:
        std::cout << "Error: Unknown command." << std::endl;
        return -EINVAL;
    }

    if (passed)
      set_state(ERT_CMD_STATE_COMPLETED);
    else
      set_state(ERT_CMD_STATE_TIMEOUT);

    return 0;
  }

  int ipurb_cmd::load_xclbin(xrt::bo& xbo, char *buf, size_t size, const uuid_t uuid)
  {
    auto data = xbo.map();
    memcpy(data, buf, size);
    xbo.sync(XCL_BO_SYNC_BO_TO_DEVICE, size, 0);

    load_xcl_bin_req_t req = { 0 };
    load_xcl_bin_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    req.XclBinAddress = static_cast<uint64_t>(xbo.address());
    req.XclBinSize = size;

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.part_info.uuid.uuid_low = uid64;
    uid64 = *uid64p;
    req.part_info.uuid.uuid_high = uid64;

    // Hard code for now
    req.part_info.startColumn = 0;
    req.part_info.totalColumn = 5;
    req.part_info.aieType = IPU_AIE2;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_LOAD_XCL_BIN,
		"IPU_MSG_LOAD_XCL_BIN", __FUNCTION__);
    printf("LOAD_XCLBIN passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    return 0;
  }

  int ipurb_cmd::open_context(const uuid_t uuid)
  {
    create_context_req_t req = { 0 };
    create_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.uuid.uuid_low = uid64;
    uid64 = *uid64p;
    req.uuid.uuid_high = uid64;

    req.pasid = 0xFFFF;
    req.num_command_queue_pairs_requested = 0x1;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_CREATE_CONTEXT,
		"IPU_MSG_CREATE_CONTEXT", __FUNCTION__);
    printf("CREATE_CONTEXT passed is %d\n", passed);

    if (!passed) {
      printf("IPURB: create context fail.\n");
      return -ETIME;
    }

    ipu_command_queue_pair_t *qPair = &resp.command_queue_pair[0];
    if (resp.num_command_queue_pairs_allocated > 0) {
      auto devp = reinterpret_cast<uint64_t>(queuep->device);
      queuep->usr_buff = new IpuHenvRing(qPair->request_queue_info, qPair->response_queue_info, devp);
      uuid_copy(queuep->m_uuid, uuid);
    }

    return 0;
  }

  int ipurb_cmd::close_context(const uuid_t uuid)
  {
    delete_context_req_t req = { 0 };
    delete_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    // TODO use higher 64 bits uuid for now
    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.uuid.uuid_low = uid64;
    uid64 = *uid64p;
    req.uuid.uuid_high = uid64;

    req.pasid = 0xFFFF;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT,
		"IPU_MSG_DELETE_CONTEXT", __FUNCTION__);
    printf("DELETE_CONTEXT passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    delete queuep->usr_buff;
    queuep->usr_buff = nullptr;
    uuid_clear(queuep->m_uuid);

    return 0;
  }

  int ipurb_cmd::sync_bo(uint64_t dest, uint64_t src, size_t size, size_t seek)
  {
    if (seek) {
      printf("IPRRB: can not sync bo, offset is not supported.\n");
      return -EINVAL;
    }

    if (uuid_is_null(queuep->m_uuid)) {
      printf("IPRRB: can not sync bo, no context created.\n");
      return -ENODEV;
    }

    // Map host buffer
    map_host_buffer_req_t mreq = { 0 };
    map_host_buffer_resp_t mresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(queuep->m_uuid));
    uint64_t uid64 = *uid64p++;
    mreq.uuid.uuid_low = uid64;
    uid64 = *uid64p;
    mreq.uuid.uuid_high = uid64;

    mreq.buffer_address = src;
    mreq.buffer_size = size;

    bool passed = RINGB_Command(mreq, &mresp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_MAP_HOST_BUFFER,
		"IPU_MSG_MAP_HOST_BUFFER", __FUNCTION__);
    printf("MAP_HOST_BUFFER passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    // Set up ADMA
    sync_bo_req_t sreq;
    sync_bo_resp_t sresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    sreq.src_addr = src;
    sreq.dst_addr = dest;
    sreq.size = size;

    passed = RINGB_Command(sreq, &sresp, queuep->usr_buff, 0xFA5EFADE, IPU_MSG_SYNC_BO,
		"IPU_MSG_SYNC_BO", __FUNCTION__);
    printf("SYNC_BO passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    return 0;
  }


  xocl_ipurb::xocl_ipurb(HwEmShim* dev)
    : queue(dev)
  {
    device = dev;
    nctx = 0;
    pid = getpid();

    xrs_hdl = xrs_init(5, XRS_MODE_SPACIAL_STATIC, &ipurb_xrs_func);
  }

  xocl_ipurb::~xocl_ipurb()
  {
    xrs_fini(xrs_hdl);
  }

  int xocl_ipurb::load_xclbin(char *buf, size_t size, const uuid_t uuid)
  {
    // Test code to verify the resource solver interfaces
    struct xrs_actions *act = nullptr;
    void (*action_cb)(xrs_handle_t hdl, struct xrs_actions *acts) = nullptr;

    uint32_t start_col = 0;

    struct cdo_parts cp;
    cp.cdo_uuid = const_cast<uuid_t *>(reinterpret_cast<const uuid_t *>(uuid)); // use XCLBIN uuid for now
    cp.nparts = 1;
    cp.ncols = 5;
    cp.start_col = &start_col;

    struct part_meta pm;
    pm.xclbin_uuid = const_cast<uuid_t *>(reinterpret_cast<const uuid_t *>(uuid));
    pm.ncdos = 1;
    pm.cdo = &cp;

    int rval = xrs_load_xclbin(xrs_hdl, pid, &pm, &act, &action_cb);
    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);

    int npasid = xrs_query_npasid(xrs_hdl, pm.xclbin_uuid);
    uint32_t pasids[npasid];
    xrs_query_pasids(xrs_hdl, pm.xclbin_uuid, npasid, (reinterpret_cast<uint32_t *>(pasids)));
    for (int i = 0; i < npasid; i++)
      printf("In %s, npasid[%d]: %d\n", __func__, i, pasids[i]);

    xrs_unload_xclbin(xrs_hdl, pid);

    // End of resource solver test code

    xrt::device xdev(device->getMCoreDevice());
    xrt::bo xbo(xdev, size, xrt::bo::flags::host_only, 0);

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    rval = 0;
    if (xcmd->load_xclbin(xbo, buf, size, uuid))
      rval = 1;

    cmd_pool.destroy(xcmd);

    return rval;
  }

  int xocl_ipurb::open_context(const uuid_t uuid, unsigned int ip_index)
  {
    if (nctx != 0) {
      printf("IPURB: can not open multiple contexts\n");
      return -EBUSY;
    }

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->open_context(uuid))
      rval = 1;
    else
      nctx++;

    cmd_pool.destroy(xcmd);
    return rval;
  }

  int xocl_ipurb::close_context(const uuid_t uuid, unsigned int ip_index)
  {
    if (nctx != 1) {
      printf("IPURB: can not close contexts, no opened context\n");
      return -ENODEV;
    }

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->close_context(uuid))
      rval = 1;
    else
      nctx--;

    cmd_pool.destroy(xcmd);
    return rval;
  }

  int xocl_ipurb::add_exec_buffer(xclemulation::drm_xocl_bo *buf)
  {
    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->exec_buf(buf))
      rval = 1;

    cmd_pool.destroy(xcmd);
    return rval;
  }

  /*
   * This function is to allocate a shadow bufer in DDR (HOST_ONLY) when
   * a BO is allocated from SRAM. Currently, we only support one SRAM
   * allocation so only no more than one DDR shadow buffer is allowed.
   */
  int xocl_ipurb::alloc_bo(size_t size)
  {
    if (sbo != nullptr) {
      printf("IPURB: fail to alloc BO, BO already allocated.\n");
      return -EINVAL;
    }

    xrt::device xdev(device->getMCoreDevice());
    sbo = std::make_unique<xrt::bo>(xdev, size, xrt::bo::flags::host_only, 0);

    return 0;
  }

  int xocl_ipurb::free_bo()
  {
    if (sbo == nullptr) {
      printf("IPURB: fail to free BO, no BO allocated.");
      return -ENODEV;
    }

    sbo.reset();
    return 0;
  }

  /*
   * This sync_bo will only be called for the BO allocated in SRAM.
   * To sync BO in SRAM, we will
   *   1. Map in the SRAM shadow buffer in DDR
   *   2. Copy the content to shadow buffer
   *   3. Send a sync_bo command to set up ADMA between shadow buffer
   *      and SRAM BO. (SRAM is actually on the user task's heap.
   */
  int xocl_ipurb::sync_bo(uint64_t dest, const void *src, size_t size, size_t seek)
  {
    auto data = sbo->map();
    memcpy(data, src, size);
    sbo->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, 0);
    auto paddr = static_cast<uint64_t>(sbo->address());

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->sync_bo(dest, paddr, size, seek))
      rval = 1;

    cmd_pool.destroy(xcmd);
    return rval;
  }

}
