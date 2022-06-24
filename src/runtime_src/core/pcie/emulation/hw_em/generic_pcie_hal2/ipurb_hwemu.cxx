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

#define INVALID_CONTEXT_ID 		(0xFF)
extern IpuHenvRing *XRT_WaitForERT(uint64_t io_hdl);
template < typename COMMAND, typename RESPONSE >
bool RINGB_Command(COMMAND command, RESPONSE  *response, IpuHenvRing *pRing,
                   uint32_t msg_id, ipu_msg_opcode_e opcode, const char *cmdStr,
                   const char *fnStr, bool expectSuccess = true);

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
    context_id = INVALID_CONTEXT_ID;
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
          memcpy(req.cu_exec.payload, ert_start_cu->data, payload_size() - 4);

          uint32_t cu_idx = 0, mask = 0;
          mask = ert_start_cu->cu_mask;

          if (!mask) {
            printf("IPURB: fail to send exec buf, invalid cu_mask\n");
            return -EINVAL;
          }

          cu_idx = 0;
          while (mask) {
            if (mask & 0x1)
              break;
            cu_idx++;
            mask >>=1;
          }

          req.cu_exec.cu_idx = cu_idx;
          printf("cu_idx %d\n", cu_idx);

          passed = RINGB_Command(req, &resp, queuep->usr_buff, 0xFA5EFADE, IPU_MSG_EXECUTE_BUFFER_CF,
               "IPU_MSG_EXECUTE_BUFFER_CF", __FUNCTION__);
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

    register_xcl_bin_req_t req = { 0 };
    register_xcl_bin_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    req.num_xcl_bin_infos = 1;
    req.xcl_bin_info[0].xcl_bin_address = static_cast<uint64_t>(xbo.address());
    req.xcl_bin_info[0].xcl_bin_size = size;

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_info[0].xcl_bin_uuid.uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_info[0].xcl_bin_uuid.uuid_high = uid64;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_REGISTER_XCL_BIN,
		"IPU_MSG_REGISTER_XCL_BIN", __FUNCTION__);
    printf("REGISTER_XCLBIN passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    return 0;
  }

  int ipurb_cmd::unload_xclbin(const uuid_t uuid)
  {
    unregister_xcl_bin_req_t req = { 0 };
    unregister_xcl_bin_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_uuid[0].uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_uuid[0].uuid_high = uid64;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_UNREGISTER_XCL_BIN,
                "IPU_MSG_UNREGISTER_XCL_BIN", __FUNCTION__);

    printf("UNREGISTER_XCLBIN passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    return 0;
  }

  int ipurb_cmd::open_context(const uuid_t uuid, uint32_t start_col, uint32_t ncol)
  {
    create_context_req_t req;
    create_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };


    memset(&req, 0, sizeof(create_context_req_t));

    req.part_info.aie_type = IPU_AIE2;
    req.part_info.start_column = start_col;
    req.part_info.total_columns = ncol;

    req.num_xcl_bin_uuids = 1;

    auto uid64p = const_cast<uint64_t *>(reinterpret_cast<const uint64_t *>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_uuid[0].uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_uuid[0].uuid_high = uid64;

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
      queuep->context_id = resp.context_id;
    }

    return queuep->context_id;
  }

  int ipurb_cmd::close_context(uint32_t ctxhdl)
  {
    // Ignore the ctxhdl for now since we only support one hw_context and
    // the default ctxhdl is 0 though we are using real context_id in ipurb
    delete_context_req_t req = { 0 };
    delete_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    req.context_id = queuep->context_id;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT,
		"IPU_MSG_DELETE_CONTEXT", __FUNCTION__);
    printf("DELETE_CONTEXT passed is %d\n", passed);
    if (!passed)
      return -ETIME;

    delete queuep->usr_buff;
    queuep->usr_buff = nullptr;
    queuep->context_id = INVALID_CONTEXT_ID;

    return 0;
  }

  int ipurb_cmd::sync_bo(uint64_t dest, uint64_t src, size_t size, size_t seek)
  {
    if (seek) {
      printf("IPRRB: can not sync bo, offset is not supported.\n");
      return -EINVAL;
    }

    if (queuep->context_id == INVALID_CONTEXT_ID) {
      printf("IPRRB: can not sync bo, no context created.\n");
      return -ENODEV;
    }

    // Map host buffer
    map_host_buffer_req_t mreq = { 0 };
    map_host_buffer_resp_t mresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    mreq.context_id = queuep->context_id;

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

  int ipurb_cmd::config(uint32_t num_cus, const void *cfg)
  {
    cu_config_t *cu_cfg = (cu_config_t *)cfg;
    scheduler_config_buffer_req_t req;
    scheduler_config_buffer_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    req.num_cus = num_cus;

    for (uint8_t i=0; i<num_cus; ++i) {
      req.configs[i].cu_idx = cu_cfg[i].cu_idx;
      req.configs[i].cu_functional = cu_cfg[i].cu_functional;

      printf("req.configs[%d].cu_idx %d, req.configs[%d].cu_functional %d\n", i, req.configs[i].cu_idx, i, req.configs[i].cu_functional);
    }

    bool passed = RINGB_Command(req, &resp, queuep->usr_buff, 0xFA5EFADE, IPU_MSG_CONFIG_CU,
           "IPU_MSG_CONFIG_CU", __FUNCTION__);

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
  }

  xocl_ipurb::~xocl_ipurb()
  {
    for (auto uuid_bo_pair: xclbin_list) {
        xocl_ipurb::unload_xclbin(uuid_bo_pair.first);
        xclbin_list.remove(uuid_bo_pair);
    }
  }

  int xocl_ipurb::load_xclbin(char *buf, size_t size, const uuid_t uuid)
  {
#if 0
    // Test code to verify the resource solver interfaces
    struct xrs_actions *act = nullptr;
    void (*action_cb)(xrs_handle_t hdl, struct xrs_actions *acts) = nullptr;

    uint32_t start_col[4] = {1, 2, 3, 4};

    struct cdo_parts cp;
    cp.cdo_uuid = const_cast<uuid_t *>(reinterpret_cast<const uuid_t *>(uuid)); // use XCLBIN uuid for now
    cp.nparts = 4;
    cp.ncols = 1;
    cp.start_col_list = start_col;

    struct part_meta pm;
    pm.xclbin_uuid = const_cast<uuid_t *>(reinterpret_cast<const uuid_t *>(uuid));
    pm.ncdos = 1;
    pm.cdo = &cp;

    struct alloc_requests req;
    req.pid = 1;
    req.pmp = &pm;
    int rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 1 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    req.pid = 2;
    rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 2 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    req.pid = 3;
    rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 3 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    req.pid = 4;
    rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 4 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    printf("\nIn %s, unload xclbin 3\n", __func__);
    xrs_release_resource(xrs_hdl, 3);

    req.pid = 5;
    rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 5 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    req.pid = 6;
    rval = xrs_allocate_resource(xrs_hdl, &req, &act, &action_cb);
    printf("\nIn %s, xrs_load_xclbin 6 returns: %d, act is %p\n", __func__, rval, act);
    if (act) {
      printf("In %s, action naction is %d\n", __func__, act->nactions);
      printf("In %s, action pid is %d\n", __func__, act->actions[0].pid);
      printf("In %s, action action is %d\n", __func__, act->actions[0].action);
      printf("In %s, action start_col is %d\n", __func__, act->actions[0].part.start_col);
      printf("In %s, action ncol is %d\n", __func__, act->actions[0].part.ncol);
    }

    if (action_cb != nullptr)
      action_cb(xrs_hdl, act);
    act = nullptr;

    int npid = xrs_query_npid(xrs_hdl, pm.xclbin_uuid);
    uint32_t pids[npid];
    xrs_query_pids(xrs_hdl, pm.xclbin_uuid, npid, (reinterpret_cast<uint32_t *>(pids)));
    printf("\n");
    for (int i = 0; i < npid; i++)
      printf("In %s, npid[%d]: %d\n", __func__, i, pids[i]);

    xrs_release_resource(xrs_hdl, 1);
    xrs_release_resource(xrs_hdl, 2);
    xrs_release_resource(xrs_hdl, 4);
    xrs_release_resource(xrs_hdl, 5);
    xrs_release_resource(xrs_hdl, 6);

    // End of resource solver test code
#endif

    xrt::device xdev(device->getMCoreDevice());
    xrt::bo xbo(xdev, size, xrt::bo::flags::host_only, 0);

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int ret = 0;
    if (xcmd->load_xclbin(xbo, buf, size, uuid))
      ret = 1;

    cmd_pool.destroy(xcmd);

    if (!ret) {
        std::pair<uuid_t, xrt::bo> p;

        uuid_copy(p.first, uuid);
        p.second = std::move(xbo);

        xclbin_list.emplace_back(p);
    }
    return ret;
  }

  int xocl_ipurb::unload_xclbin(const uuid_t uuid)
  {
    if (nctx) {
      printf("IPURB: opened context, can't unload xclbin\n");
      return -ENODEV;
    }

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->unload_xclbin(uuid))
      rval = 1;

    cmd_pool.destroy(xcmd);

    return rval;
  }

  int xocl_ipurb::open_context(const uuid_t uuid, uint32_t start_col, uint32_t ncol)
  {
    if (nctx != 0) {
      printf("IPURB: can not open multiple contexts\n");
      return -EBUSY;
    }

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int context_id = xcmd->open_context(uuid, start_col, ncol);
    if (context_id >= 0)
      nctx++;

    cmd_pool.destroy(xcmd);
    return context_id;
  }

  int xocl_ipurb::close_context(uint32_t ctxhdl)
  {
    if (nctx != 1) {
      printf("IPURB: can not close contexts, no opened context\n");
      return -ENODEV;
    }

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->close_context(ctxhdl))
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

  int xocl_ipurb::config(uint32_t num_cus, const void *cfg)
  {
    ipurb_cmd *xcmd = cmd_pool.construct(&queue);

    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->config(num_cus, cfg))
      rval = 1;

    cmd_pool.destroy(xcmd);
    return rval;
  }
}
