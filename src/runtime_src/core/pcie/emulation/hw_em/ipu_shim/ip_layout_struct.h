// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

#ifndef _IP_LAYOUT_STRUCT_H_
#define _IP_LAYOUT_STRUCT_H_

#include "config.h"
#include "mem_model.h"
#include "memorymanager.h"
#include "xclbin.h"
#include "core/common/xclbin_parser.h"
#include "core/include/experimental/xrt_xclbin.h"

#include <exception>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace xclhwemhal2 {
  class HwEmShim;
}

namespace xclhwemhal2 {
  typedef struct loadBitStream  bitStreamArg;
}



namespace xclipu
{

  struct membank
  {
    uint64_t base_addr; // base address of bank
    std::string tag;     // bank tag in lowercase
    uint64_t size;       // size of this bank in bytes
    int32_t index;       // bank id
  };

  struct kds_info
  {
    uint32_t cuindex;   // cu index
    std::string name;   // kernel name
    uint64_t base_addr; // 0 -dummy  ( IPU specific)
    uint32_t status;    // 0 -dummy ( IPU specific)
    uint64_t usages;    // 0-dummy ( IPU specific)
  };



  // This Pool manager mimics a dequeue with a limited size of reserve_capacity
  struct IndexPool
  {
    // vector<bool> is very optimized version for non-contiguous bool variables where each bool index can be used
    // as a slot to represent column free nature.
    std::vector<bool> index_pool;
    std::mutex mtx;
    short current_index = 0;
    int m_size;
    IndexPool() = default;

    void reserve_capacity(const int iSize)
    {
      m_size = iSize;
      std::lock_guard<std::mutex> lk{ mtx };
      index_pool = std::vector<bool>(iSize);
      index_pool.reserve(iSize);
    }

    int get_available_index()
    {
      std::lock_guard<std::mutex> lk{ mtx };
      auto count = 0;
      for (const auto val : index_pool)
      {
        if (val == false)
        {
          index_pool.at(count) = true;
          break;
        }
        ++count;
      }
      // Index always start with 1, so decrement yourself whoever calling.
      return (count < m_size) ? (current_index = ++count) : (throw std::logic_error{ "invalid index found or there is not free slot available" });
    }

    void reset(int Index)
    {
      if (Index > m_size)
        throw std::logic_error{ " index is out of range" };
      else
      {
        std::lock_guard<std::mutex> lk{ mtx };
        index_pool.at(Index - 1) = false;
      }
    }

    int operator++()
    {
      return get_available_index();
    }

    int operator++(int dummy)
    {
      return get_available_index();
    }
    short get_current_index()
    {
      std::lock_guard<std::mutex> lk{ mtx };
      return current_index;

    }
  };
  struct saie_partition_unique_metadata
  {
    std::pair<int, std::string> mslot_uuid;
    bool operator == ( const saie_partition_unique_metadata& lhs) const
    {
      return (mslot_uuid.first == lhs.mslot_uuid.first) && (mslot_uuid.second == lhs.mslot_uuid.second);
    }
    bool operator < ( const saie_partition_unique_metadata& lhs) const
    {
      return (mslot_uuid.first < lhs.mslot_uuid.first); // && (mslot_uuid.second < lhs.mslot_uuid.second);
    }
  };
  // A structure to store dependent hw context  information with slotid as key distinguisher
  struct hw_context
  {
    //std::unique_ptr<xrt::bo> xbo;
    std::vector< xclemulation::MemoryManager* > mDDRMemoryManager;
    std::list<xclemulation::DDRBank> mDdrBanks;
    std::shared_ptr<xrt::xclbin>  mXclBin; 
    std::vector<membank> mMembanks;
    // Very specific structure to store CU information that gets parsed from xclbin metadata
    struct CUinfo
    {
      xrt_core::xclbin::aie_partition_obj maie_partition_obj;
      std::map<uint32_t, uint32_t> mCUFunctionalMap;
      uint32_t mCUs = 0;
      std::vector<kds_info> mCU_kds_info_list;
      std::map<std::string, uint64_t> mCURangeMap;
    }sCUinfo;

    int32_t mSlotID = -1;       // default slotID value.
    unsigned int host_sptag_idx = -1;
    int64_t mrid = -1;

  };

  // A class which maintains the list of unique xclbin's and their metadata
  // DDRMemoryManager which gets used by Ipurb to perform
  // alloc_bo, add_exec_buf, sync_bo operations.
  class cIpuManager
  {

  public:
    // Rule of 5
    cIpuManager(xclhwemhal2::HwEmShim*);
    ~cIpuManager() = default;
    cIpuManager(const cIpuManager&) = default;
    cIpuManager(cIpuManager&&) = default;
    cIpuManager& operator=(const cIpuManager&) = default;
    cIpuManager& operator=(cIpuManager&&) = default;

    //utility API
    std::string uuid_to_string(const uuid_t iUUID);
    bool xclbin_not_initiated(const uuid_t);

    //public API's
    void add_xclbin(const xrt::xclbin&);
    std::shared_ptr<xrt::xclbin> get_xclbin(std::string& iUUID);
    void init_xclbin_metadata(const uuid_t iUUID, int islotid);
    void update_slotid(const uuid_t, uint32_t);
    void reset_xclbin(const uint32_t) noexcept;

    //std::shared_ptr<hw_context> get_hwctx(const uuid_t);
    //std::shared_ptr<hw_context> get_hwctx(const std::string uuid_as_string);
    std::shared_ptr<hw_context> get_hwctx(const uint32_t slotid);
    std::shared_ptr<hw_context> get_hw_context(int islotid, std::string suuid);
    saie_partition_unique_metadata get_pair(int islotid);
    using uuid_string = std::string;
  private:
    void bit_stream_worker( xclhwemhal2::bitStreamArg& lbitstream, std::shared_ptr<hw_context>& ihw_context);
    void initMemoryManager(std::shared_ptr<hw_context>& lhwctx, std::list<xclemulation::DDRBank>& DDRBankList);

    xclhwemhal2::HwEmShim* handle;
    std::mutex mtx;
    std::vector<std::shared_ptr<xrt::xclbin> > mXclBinList;
    std::map<saie_partition_unique_metadata, std::shared_ptr<hw_context>>  m_loaded_xclbin_metadata;       
         
    //pair<key,value> - pair<uuid, hw_context>
    //std::map<uuid_string, std::shared_ptr<hw_context>> xclLoad;
  };
}
#endif