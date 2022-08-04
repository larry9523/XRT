/*
 *  SPDX-License-Identifier: Apache-2.0
 *  Copyright (C) 2021, Xilinx Inc
 *  Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#include "shim.h"
#include "ipu_msg.h"
#include "mgmt_msg.h"
#include "app_msg.h"

#define SCOPED_LOCK true
//#define INVALID_CONTEXT_ID 		(0xFF)
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
  }

  ipurb_queue::~ipurb_queue()
  {
  }

/**
 * cu_mask_to_cu_idx - Convert CU mask to CU index list
 *
 * @xcmd: Command
 * @cus:  CU index list
 *
 * Returns: Number of CUs
 *
 */
int ipurb_queue::cu_mask_to_cu_idx(struct kds_command *xcmd, uint8_t *cus)
{
	int num_cu = 0;
  /*
	uint32_t mask;

	// i for iterate masks, j for iterate bits 
	int i, j;

	for (i = 0; i < xcmd->num_mask; ++i) {
		if (xcmd->cu_mask[i] == 0)
			continue;

		mask = xcmd->cu_mask[i];
		for (j = 0; mask > 0; ++j) {
			if (!(mask & 0x1)) {
				mask >>= 1;
				continue;
			}

			cus[num_cu] = i * 32 + j;
			num_cu++;
			mask >>= 1;
		}
	}
*/
	return num_cu;
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

  int ipurb_cmd::exec_buf(xclemulation::drm_xocl_bo* bo, uint64_t iSlotID)
  {
    this->ert_pkt = (struct ert_packet*)bo->buf;
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



      auto ert_start_cu = reinterpret_cast<ert_start_kernel_cmd*>(ert_pkt);
      memcpy(req.cu_exec.payload, ert_start_cu->data, payload_size() - 4);

      uint32_t cu_idx = 0, mask = 0, mask1 = 0;
      // udpate cu_mask to 128 bit mask
      mask1 = ert_start_cu->cu_mask;
      mask = mask1;
      if (!mask) {
        printf("IPURB: fail to send exec buf, invalid cu_mask\n");
        return -EINVAL;
      }

      cu_idx = 0;
      while (mask) {
        if (mask & 0x1)
          break;
        cu_idx++;
        mask >>= 1;
      }

      req.cu_exec.cu_idx = cu_idx;
      printf("cu_idx %d\n", cu_idx);
      if (SCOPED_LOCK)
      {
        // usr_buff is present in sharable map so protect with a mutex.
        std::lock_guard<std::mutex> lkgd{ queuep->usr_buff_mtx };
        //Ensure all RINGB operations are sequential at this place
        std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
        passed = RINGB_Command(req, &resp, queuep->m_usr_buff_map[iSlotID]->usr_buff.get(), 0xFA5EFADE, IPU_MSG_EXECUTE_BUFFER_CF,
          "IPU_MSG_EXECUTE_BUFFER_CF", __FUNCTION__);
        printf("passed is %d\n", passed);
      }

    }
    break;

    case ERT_EXIT:
      break;

    default:
      std::cerr << "Error: Unknown command." << std::endl;
      return -EINVAL;
    }

    if (passed)
      set_state(ERT_CMD_STATE_COMPLETED);
    else
      set_state(ERT_CMD_STATE_TIMEOUT);

    return 0;
  }

  int ipurb_cmd::load_xclbin(xrt::bo* xbo, char* buf, size_t size, const uuid_t uuid)
  {
    //auto data = xbo.map();
    auto data = xbo->map();
    memcpy(data, buf, size);
    //xbo.sync(XCL_BO_SYNC_BO_TO_DEVICE, size, 0);
    xbo->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, 0);

    register_xcl_bin_req_t req = { 0 };
    register_xcl_bin_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    req.num_xcl_bin_infos = 1;
    req.xcl_bin_info[0].xcl_bin_address = static_cast<uint64_t>(xbo->address());
    req.xcl_bin_info[0].xcl_bin_size = size;

    auto uid64p = const_cast<uint64_t*>(reinterpret_cast<const uint64_t*>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_info[0].xcl_bin_uuid.uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_info[0].xcl_bin_uuid.uuid_high = uid64;
    if (SCOPED_LOCK)
    {
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_REGISTER_XCL_BIN,
        "IPU_MSG_REGISTER_XCL_BIN", __FUNCTION__);
      printf("REGISTER_XCLBIN passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    return 0;
  }

  int ipurb_cmd::unload_xclbin(const uuid_t uuid)
  {
    unregister_xcl_bin_req_t req = { 0 };
    unregister_xcl_bin_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    auto uid64p = const_cast<uint64_t*>(reinterpret_cast<const uint64_t*>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_uuid[0].uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_uuid[0].uuid_high = uid64;
    if (SCOPED_LOCK)
    {
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_UNREGISTER_XCL_BIN,
        "IPU_MSG_UNREGISTER_XCL_BIN", __FUNCTION__);

      printf("UNREGISTER_XCLBIN passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    return 0;
  }

  int ipurb_cmd::open_context(const uuid_t uuid, uint32_t start_col, uint32_t ncol, int slotid)
  {
    create_context_req_t req;
    create_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };


    memset(&req, 0, sizeof(create_context_req_t));

    req.part_info.aie_type = IPU_AIE2;
    req.part_info.start_column = start_col;
    req.part_info.total_columns = ncol;

    req.num_xcl_bin_uuids = 1;

    auto uid64p = const_cast<uint64_t*>(reinterpret_cast<const uint64_t*>(uuid));
    uint64_t uid64 = *uid64p++;
    req.xcl_bin_uuid[0].uuid_low = uid64;
    uid64 = *uid64p;
    req.xcl_bin_uuid[0].uuid_high = uid64;

    req.pasid = 0xFFFF;
    req.num_command_queue_pairs_requested = 0x1;
    if (SCOPED_LOCK)
    {
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_CREATE_CONTEXT,
        "IPU_MSG_CREATE_CONTEXT", __FUNCTION__);
      printf("CREATE_CONTEXT passed is %d\n", passed);

      if (!passed) {
        printf("IPURB: create context fail.\n");
        return -ETIME;
      }
    }
    ipu_command_queue_pair_t* qPair = &resp.command_queue_pair[0];
    //uint32_t slotid = 0;
    if (resp.num_command_queue_pairs_allocated > 0) {
      auto devp = reinterpret_cast<uint64_t>(queuep->device);
      std::shared_ptr<usr_buff_struct> obj = std::make_shared<usr_buff_struct>();
      obj->usr_buff = std::make_shared<IpuHenvRing>(qPair->request_queue_info, qPair->response_queue_info, devp);
      obj->context_id = resp.context_id;
      //slotid = queuep->device->m_SlotID++;
      std::lock_guard<std::mutex> lkgd{ queuep->usr_buff_mtx };
      queuep->m_usr_buff_map[slotid] = obj;
    }

    return 0;  //success
  }

  int ipurb_cmd::close_context(uint32_t slot)
  {
    // Ignore the ctxhdl/slot for now since we only support one hw_context and
    // the default ctxhdl/slot is 0 though we are using real context_id in ipurb
    delete_context_req_t req = { 0 };
    delete_context_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    if (SCOPED_LOCK)
    {
      std::lock_guard<std::mutex> lkgd{ queuep->usr_buff_mtx };
      req.context_id = queuep->m_usr_buff_map[slot]->context_id;
    }
    if (SCOPED_LOCK)
    {
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_DELETE_CONTEXT,
        "IPU_MSG_DELETE_CONTEXT", __FUNCTION__);
      printf("DELETE_CONTEXT passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    return 0;
  }

  int ipurb_cmd::sync_bo(uint64_t dest, uint64_t src, size_t size, size_t seek, int slot)
  {

    auto slot_found = queuep->m_usr_buff_map.find(slot);
    if (slot_found == queuep->m_usr_buff_map.end())
    {
      std::cout << "\n slot id is not found in sbo map \n";
      return -ENODEV;
    }
    if (queuep->m_usr_buff_map[slot]->context_id == INVALID_CONTEXT_ID) {
      printf("IPRRB: can not sync bo, no context created.\n");
      return -ENODEV;
    }
    //std::cout<<"\n the context id "<<queuep->m_usr_buff_map[slot]->context_id <<" with slot id as "<<slot<<"\n";
        // Map host buffer
    map_host_buffer_req_t mreq = { 0 };
    map_host_buffer_resp_t mresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    mreq.context_id = queuep->m_usr_buff_map[slot]->context_id;

    mreq.buffer_address = src;
    mreq.buffer_size = size;
    bool passed = false;
    if (SCOPED_LOCK)
    {
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      passed = RINGB_Command(mreq, &mresp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_MAP_HOST_BUFFER,
        "IPU_MSG_MAP_HOST_BUFFER", __FUNCTION__);
      printf("MAP_HOST_BUFFER passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    // Set up ADMA
    sync_bo_req_t sreq;
    sync_bo_resp_t sresp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    sreq.src_addr = src;
    sreq.dst_addr = dest;
    sreq.size = size;
    sreq.src_type = 2;
    sreq.dst_type = 0;


    if (SCOPED_LOCK)
    {
      std::lock_guard<std::mutex> lkgd{ queuep->usr_buff_mtx };
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      passed = RINGB_Command(sreq, &sresp, queuep->m_usr_buff_map[slot]->usr_buff.get(), 0xFA5EFADE, IPU_MSG_SYNC_BO,
        "IPU_MSG_SYNC_BO", __FUNCTION__);
      printf("SYNC_BO passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    return 0;
  }

  int ipurb_cmd::config(uint32_t num_cus, const void* cfg, int slot)
  {
    cu_config_t* cu_cfg = (cu_config_t*)cfg;
    scheduler_config_buffer_req_t req;
    scheduler_config_buffer_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };
    req.num_cus = num_cus;

    for (uint8_t i = 0; i < num_cus; ++i) {
      req.configs[i].cu_idx = cu_cfg[i].cu_idx;
      req.configs[i].cu_functional = cu_cfg[i].cu_functional;

      printf("req.configs[%d].cu_idx %d, req.configs[%d].cu_functional %d\n", i, req.configs[i].cu_idx, i, req.configs[i].cu_functional);
    }

    if (SCOPED_LOCK)
    {
      std::lock_guard<std::mutex> lkgd{ queuep->usr_buff_mtx };
      //Ensure all RINGB operations are sequential at this place
      std::lock_guard<std::mutex> lk{ queuep->lGlobalMtx };
      int passed = RINGB_Command(req, &resp, queuep->m_usr_buff_map[slot]->usr_buff.get(), 0xFA5EFADE, IPU_MSG_CONFIG_CU,
        "IPU_MSG_CONFIG_CU", __FUNCTION__);
      printf("IPU_MSG_CONFIG_CU passed is %d\n", passed);
      if (!passed)
        return -ETIME;
    }
    return 0;
  }

  int ipurb_cmd::assign_mgmt_pasid(uint32_t mgmt_pasid)
  {
    assign_mgmt_pasid_req_t req = { 0 };
    assign_mgmt_pasid_resp_t resp = { IPU_STATUS_MAX_IPU_STATUS_CODE };

    req.pasid = mgmt_pasid;

    bool passed = RINGB_Command(req, &resp, queuep->mng_buff, 0xFA5EFADE, IPU_MSG_ASSIGN_MGMT_PASID,
    "IPU_MSG_ASSIGN_MGMT_PASID", __FUNCTION__);

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

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      throw std::runtime_error("FAILED to construct ipurb cmd \n");

    int rval = 0;
    if (xcmd->assign_mgmt_pasid(0xFFFF))
      rval = 1;

    cmd_pool.destroy(xcmd);

    if (rval)
      throw std::runtime_error("FAILED to assign_mgmt_pasid \n");
  }

  xocl_ipurb::~xocl_ipurb()
  {
    /*
    for (auto uuid_bo_pair: xclbin_slot_bo_map) {
        xocl_ipurb::unload_xclbin(uuid_bo_pair.first);
        uuid_bo_pair.second.release();
    }
    */
  }

  int xocl_ipurb::load_xclbin(char* buf, size_t size, const uuid_t uuid, int islotid)
  {
#if 0
    // Test code to verify the resource solver interfaces
    struct xrs_actions* act = nullptr;
    void (*action_cb)(xrs_handle_t hdl, struct xrs_actions* acts) = nullptr;

    uint32_t start_col[4] = { 1, 2, 3, 4 };

    struct cdo_parts cp;
    cp.cdo_uuid = const_cast<uuid_t*>(reinterpret_cast<const uuid_t*>(uuid)); // use XCLBIN uuid for now
    cp.nparts = 4;
    cp.ncols = 1;
    cp.start_col_list = start_col;

    struct part_meta pm;
    pm.xclbin_uuid = const_cast<uuid_t*>(reinterpret_cast<const uuid_t*>(uuid));
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
    xrs_query_pids(xrs_hdl, pm.xclbin_uuid, npid, (reinterpret_cast<uint32_t*>(pids)));
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
    auto suuid = device->convert_uuid_to_string(uuid);
    DEBUG_MSGS_COUT(" load_xclbin is started for UUID \t" << suuid);
    xrt::device xdev(device->getMCoreDevice());
    DEBUG_MSGS_COUT(" internal alloc_bo will be called for UUID \t" << suuid << "\t with slot id as "<<islotid);
    std::shared_ptr<xrt::bo> lxbo;
    auto itr = xclbin_slot_bo_map.find(suuid);
    if (itr != xclbin_slot_bo_map.end())
    {
      std::cerr << "\n same xclbin is trying to load again! so do not create XBo object anymore, use existing one!";
      lxbo = itr->second;
      
    }
    else
    {
      lxbo = std::make_shared<xrt::bo>(xdev, size, xrt::bo::flags::host_only, 0);
      DEBUG_MSGS_COUT("\n host_only xrt::bo  is created \n");
      xclbin_slot_bo_map[suuid] = lxbo;
    }

    //auto xbo = std::make_shared<xrt::bo>(xdev, size, xrt::bo::flags::host_only, 0);
    ipurb_cmd* xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int ret = 0;
    auto xbo_ptr = lxbo.get();
    if (xcmd->load_xclbin(xbo_ptr, buf, size, uuid) != 0)
      ret = 1;

    cmd_pool.destroy(xcmd);
    DEBUG_MSGS_COUT("\n load_xclbin is finsihed for UUID \t" << suuid<<" with slot id "<<islotid );
    return ret;
  }

  int xocl_ipurb::unload_xclbin(const uuid_t uuid)
  {

    ipurb_cmd *xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->unload_xclbin(uuid))
      rval = 1;

    cmd_pool.destroy(xcmd);
 
    return rval;
  }

  int xocl_ipurb::open_context(const uuid_t uuid, uint32_t start_col, uint32_t ncol, int slotid)
  {
    //TODO: nctx should have a conditional check where slot index is free.
    DEBUG_MSGS_COUT("\n internal open_context started for UUID \t" << device->convert_uuid_to_string(uuid));
    ipurb_cmd* xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int ret = xcmd->open_context(uuid, start_col, ncol, slotid);

    cmd_pool.destroy(xcmd);
    return ret;
  }

  int xocl_ipurb::close_context(uint32_t ctxhdl)
  {
    ipurb_cmd* xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->close_context(ctxhdl))
      rval = 1;
    else
      nctx--;

    cmd_pool.destroy(xcmd);

    if (rval == 0)
    {
      return rval;

      DEBUG_MSGS_COUT("\n cleaning the memory now for slot id " << ctxhdl);
     
      std::lock_guard<std::mutex> lkgd{ queue.usr_buff_mtx };
      queue.m_usr_buff_map.erase(ctxhdl);
      this->removeBO(ctxhdl);
    }
    DEBUG_MSGS_COUT("\n Finished cleaning memory for slot id " << ctxhdl);
    return rval;
  }
  void xocl_ipurb::removeBO(uint32_t slotid)
  {
    return;
    auto ptr = reinterpret_cast<xclhwemhal2::HwEmShim*>(this->device);
    if (!ptr)
    {
      std::cerr << "\n failed typecast to HwEmShim ptr\n";
      return;
    }

    auto uuid = ptr->get_uuid_as_string(slotid);
    DEBUG_MSGS_COUT("\n removeBO UUID is::" << uuid << "\t for slotid::" << slotid);

    if (!uuid.empty())
    {

      if (xclbin_slot_bo_map.find(uuid) != xclbin_slot_bo_map.end())
      {
        DEBUG_MSGS_COUT("\n found BO and erasing it.");
       // xclbin_slot_bo_map.erase(uuid);
      }
      else
      {
        DEBUG_MSGS_COUT("\n uuid is not matched in BO map\t");
      }
    }
    else
      std::cerr << "\n UUID is not present in shim map::" << slotid;
  }


  int xocl_ipurb::add_exec_buffer(xclemulation::drm_xocl_bo* buf, uint64_t iSlotID)
  {
    DEBUG_MSGS_COUT(" started.");
    ipurb_cmd* xcmd = cmd_pool.construct(&queue);
    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->exec_buf(buf, iSlotID))
      rval = 1;

    cmd_pool.destroy(xcmd);
    DEBUG_MSGS_COUT(" end.");
    return rval;
  }

  /*
   * This function is to allocate a shadow bufer in DDR (HOST_ONLY) when
   * a BO is allocated from SRAM. Currently, we only support one SRAM
   * allocation so only no more than one DDR shadow buffer is allowed.
   */
  int xocl_ipurb::alloc_bo(size_t size, uint64_t iSlotID)
  {
    xrt::device xdev(device->getMCoreDevice());
    // iSlotID <-> contextID   one create context has one sBO
    // Ensure one slotID should have a single SBO only, add that check.
    std::lock_guard<std::mutex> scplk{ Sbo_mtx };
    if (mSlotID_Sbo.find(iSlotID) != mSlotID_Sbo.end()) {
      DEBUG_MSGS_COUT(" got same slotID for creating a shadow buffer, so skipping.");

    }
    else {
      mSlotID_Sbo[iSlotID] = std::make_unique<xrt::bo>(xdev, size, xrt::bo::flags::host_only, 0);
      DEBUG_MSGS_COUT(" After creating Real BO object mSlotID_Sbo size is \t" << mSlotID_Sbo.size());
    }

    return 0;
  }

  int xocl_ipurb::free_bo(uint64_t iSlotID)
  {
    DEBUG_MSGS_COUT( " started for slotID \t" << iSlotID);
    std::lock_guard<std::mutex> scplk{ Sbo_mtx };
    //mSlotID_Sbo.erase(iSlotID);
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
  int xocl_ipurb::sync_bo(uint64_t dest, const void *src, size_t size, size_t seek, uint64_t iSlotID)
  { 
    //Try to reduce the scope of this lock, underneath sync_bo has another lock in it.
    // recursive mutex calls are happening! Take care.
    DEBUG_MSGS_COUT( " started for slotID \t" << iSlotID);

    std::lock_guard<std::mutex> scplk{Sbo_mtx};
    auto itr = mSlotID_Sbo.find(iSlotID);
    if (itr != mSlotID_Sbo.end())
    {
      auto data = itr->second->map(); // sbo->map();
      memcpy(data, src, size);
      itr->second->sync(XCL_BO_SYNC_BO_TO_DEVICE, size, 0);
      auto paddr = static_cast<uint64_t>(itr->second->address());

      ipurb_cmd *xcmd = cmd_pool.construct(&queue);
      if (!xcmd)
        return 1;

      int rval = 0;
      if (xcmd->sync_bo(dest, paddr, size, seek, iSlotID))
      {
        std::cerr<<"\n xcmd-> sync_bo is failed ";
        rval = 1;
      }

      cmd_pool.destroy(xcmd);
      DEBUG_MSGS_COUT( " end for slotID \t" << iSlotID);
      return rval;
    }
    else
    {
      std::cerr << "\n should n't come here! index should identify this.";
      return 1;
    }
  }

  int xocl_ipurb::config(uint32_t num_cus, const void *cfg, int slot)
  {
    ipurb_cmd *xcmd = cmd_pool.construct(&queue);

    if (!xcmd)
      return 1;

    int rval = 0;
    if (xcmd->config(num_cus, cfg, slot))
      rval = 1;

    cmd_pool.destroy(xcmd);
    return rval;
  }

  
}
