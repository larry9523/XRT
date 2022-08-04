// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.


#include <boost/property_tree/xml_parser.hpp>
#include "core/include/xclbin.h"
#include "ip_layout_struct.h"
#include "shim.h"

namespace xclipu
{

    cIpuManager::cIpuManager(xclhwemhal2::HwEmShim* hwshim)
    {
        handle = hwshim;
    }
    bool cIpuManager::xclbin_not_initiated(const uuid_t Uuid)
    {
        /*
        auto uuid = uuid_to_string(Uuid);
        std::lock_guard<std::mutex> lk{ mtx };
        auto itr = xclLoad.find(uuid);
        if (itr != xclLoad.end())
        {
            auto lhwctx = itr->second;
            if (lhwctx->mSlotID == -1)
                return true;
        }
        */
        return false;

    }
    void cIpuManager::update_slotid(const uuid_t iUuid, uint32_t iSlotId)
    {
        /*
        DEBUG_MSGS_COUT(" SlotID is received::" << iSlotId );
        auto uuid = uuid_to_string(iUuid);
        if (xclbin_not_initiated(iUuid))
        {
            std::lock_guard<std::mutex> lk{ mtx };
            xclLoad[uuid]->mSlotID = iSlotId;
        }
        else
        {
            // take care of duplicate UUID's
        }
        */
    }
    void cIpuManager::add_xclbin(const xrt::xclbin& iXclBin)
    {
        std::lock_guard<std::mutex> lk{ mtx };
        auto itop = reinterpret_cast<const axlf*>(iXclBin.get_axlf());
        auto iuuid = uuid_to_string(itop->m_header.uuid);
        for ( auto& x : mXclBinList)
        {
        auto laxlf = reinterpret_cast<const axlf*>(x->get_axlf());
        auto luud = uuid_to_string(laxlf->m_header.uuid);
        // entry is found already, so I don't push same entry again.
        if (luud.compare(iuuid) == 0)
            return;
        }
        
        mXclBinList.push_back(std::make_shared<xrt::xclbin>(iXclBin));
        std::cout<<"\n mXclBinList size is::"<<mXclBinList.size()<<std::endl;
    }
    std::shared_ptr<xrt::xclbin> cIpuManager::get_xclbin(std::string& iUUID)
    {   
        std::lock_guard<std::mutex> lk{ mtx };
        DEBUG_MSGS_COUT(" UUID asking from  get_xclbin is ::"<<iUUID);
        for ( auto& j : mXclBinList)
        {
            auto laxlf = reinterpret_cast<const axlf*>(j->get_axlf());
            auto luuid = uuid_to_string(laxlf->m_header.uuid);
            // entry is found already, so I don't push same entry again.
            if (luuid.compare(iUUID) == 0)
            {
                DEBUG_MSGS_COUT(" UUID is found in vector of xclbinlist "<<luuid);
                return j;
            }
            
        }
        DEBUG_MSGS_COUT(" UUID is NOT FOUND in vector of xclbinlist ");
        return nullptr;
    }
    void cIpuManager::reset_xclbin(uint32_t iSlotID) noexcept
    {
        // Need to cover duplicate xclbin or may be slot -xclbin
        uuid_string lUUID = "";
        auto p = get_pair(iSlotID);
        std::lock_guard<std::mutex> lk{ mtx };
        m_loaded_xclbin_metadata.erase(p);          // will throw exception if not found, app crashes as it is noexcept.
        
    }
    saie_partition_unique_metadata cIpuManager::get_pair(int islotid)
    {
        std::lock_guard<std::mutex> lk{ mtx };
        for(auto& x: m_loaded_xclbin_metadata)
        {
            if (x.first.mslot_uuid.first == islotid)
                return x.first;
        }
        return { {0,0} };
    }
    std::shared_ptr<hw_context> cIpuManager::get_hw_context(int islotid, std::string suuid)
    {
        saie_partition_unique_metadata laie{ {islotid,suuid} };
        return m_loaded_xclbin_metadata[laie];                      // ensure memory is taken care.
    }

    void cIpuManager::init_xclbin_metadata(const uuid_t iUUID, int islotid)
    {
        
        auto uuid = uuid_to_string(iUUID);
        saie_partition_unique_metadata laie{ {islotid,uuid} };
        auto lxclbin_metadata = get_xclbin(uuid);
        auto lhw_context = std::make_shared<hw_context>();              //empty hw_context object
        lhw_context->mXclBin = std::make_shared<xrt::xclbin>(*lxclbin_metadata);
        lhw_context->mSlotID = islotid;
        // scoped lock
        {
            std::lock_guard<std::mutex> lk{ mtx };
            m_loaded_xclbin_metadata[laie] = lhw_context;
        }
        
       // auto itr = xclLoad.find(uuid);
        if (1)
        {
        ssize_t zipFileSize = 0;
        ssize_t xmlFileSize = 0;
        ssize_t debugFileSize = 0;
        ssize_t memTopologySize = 0;
        ssize_t pdiSize = 0;
        ssize_t emuDataSize = 0;
        ssize_t ipuDataSize = 0;



        std::unique_ptr<char[]> zipFile;
        std::unique_ptr<char[]> xmlFile;
        std::unique_ptr<char[]> debugFile;
        std::unique_ptr<char[]> memTopology;
        std::unique_ptr<char[]> pdi;
        std::unique_ptr<char[]> emuData;
        std::unique_ptr<char[]> ipuData;

        auto header = lhw_context->mXclBin->get_axlf();
        char* bitstreambin = reinterpret_cast<char*> (const_cast<xclBin*> (header));
        if (std::memcmp(bitstreambin, "xclbin2", 7))
        {
            std::cerr << "\n memcmp is failing \n";
            return;
        }
        // check xclbin version with vivado tool version
        // xclemulation::checkXclibinVersionWithTool(header);

        auto top = reinterpret_cast<const axlf*>(header);
        if (auto sec = xclbin::get_axlf_section(top, EMBEDDED_METADATA))
        {
            xmlFileSize = sec->m_sectionSize;
            xmlFile = std::make_unique<char[]>(xmlFileSize);
            memcpy(xmlFile.get(), bitstreambin + sec->m_sectionOffset, xmlFileSize);
        }
        if (auto sec = xclbin::get_axlf_section(top, BITSTREAM))
        {
            zipFileSize = sec->m_sectionSize;
            zipFile = std::make_unique<char[]>(zipFileSize);
            memcpy(zipFile.get(), bitstreambin + sec->m_sectionOffset, zipFileSize);
        }
        if (auto sec = xclbin::get_axlf_section(top, DEBUG_IP_LAYOUT))
        {
            debugFileSize = sec->m_sectionSize;
            debugFile = std::make_unique<char[]>(debugFileSize);
            memcpy(debugFile.get(), bitstreambin + sec->m_sectionOffset, debugFileSize);
        }
        //if (auto sec = xrt_core::xclbin::get_axlf_section(top, ASK_GROUP_TOPOLOGY))
        if (auto sec = xclbin::get_axlf_section(top, ASK_GROUP_TOPOLOGY))
        {
            memTopologySize = sec->m_sectionSize;
            memTopology = std::make_unique<char[]>(memTopologySize);
            memcpy(memTopology.get(), bitstreambin + sec->m_sectionOffset, memTopologySize);
        }
        if (auto sec = xclbin::get_axlf_section(top, PDI))
        {
            pdiSize = sec->m_sectionSize;
            pdi = std::make_unique<char[]>(pdiSize);
            memcpy(pdi.get(), bitstreambin + sec->m_sectionOffset, pdiSize);
        }
        if (auto sec = xclbin::get_axlf_section(top, EMULATION_DATA))
        {
            emuDataSize = sec->m_sectionSize;
            emuData = std::make_unique<char[]>(emuDataSize);
            memcpy(emuData.get(), bitstreambin + sec->m_sectionOffset, emuDataSize);
        }
        // read data related IP LAYOUT , kds info should be filled from this layout
        if (auto sec = xclbin::get_axlf_section(top, IP_LAYOUT))
        {
            ipuDataSize = sec->m_sectionSize;
            ipuData = std::make_unique<char[]>(ipuDataSize);
            memcpy(ipuData.get(), bitstreambin + sec->m_sectionOffset, ipuDataSize);
        }

        xclhwemhal2::bitStreamArg lbitStreamArgs;

        lbitStreamArgs.m_zipFile = zipFile.get();
        lbitStreamArgs.m_zipFileSize = zipFileSize;
        lbitStreamArgs.m_xmlfile = xmlFile.get();
        lbitStreamArgs.m_xmlFileSize = xmlFileSize;
        lbitStreamArgs.m_debugFile = debugFile.get();
        lbitStreamArgs.m_debugFileSize = debugFileSize;
        lbitStreamArgs.m_memTopology = memTopology.get();
        lbitStreamArgs.m_memTopologySize = memTopologySize;
        lbitStreamArgs.m_pdi = pdi.get();
        lbitStreamArgs.m_pdiSize = pdiSize;
        lbitStreamArgs.m_emuData = emuData.get();
        lbitStreamArgs.m_emuDataSize = emuDataSize;
        // IPU Layout
        lbitStreamArgs.m_ipuData = ipuData.get();
        lbitStreamArgs.m_ipuDataSize = ipuDataSize;

        lbitStreamArgs.m_top = top;
        bit_stream_worker( lbitStreamArgs, lhw_context);

        //auto lipurb = handle->get_ipurb();
            
        }
       
    }


    void cIpuManager::initMemoryManager( std::shared_ptr<hw_context>& lhwctx, std::list<xclemulation::DDRBank>& DDRBankList)
    {
        //auto lhwctx = xclLoad[iUUID];
        std::list<xclemulation::DDRBank>::iterator start = DDRBankList.begin();
        std::list<xclemulation::DDRBank>::iterator end = DDRBankList.end();
        uint64_t base = 0;
        for (;start != end; start++)
        {
            const uint64_t bankSize = (*start).ddrSize;
            lhwctx->mDdrBanks.push_back(*start);
            lhwctx->mDDRMemoryManager.push_back(new xclemulation::MemoryManager(bankSize, base, getpagesize()));
            base += bankSize;
        }
    }
    void cIpuManager::bit_stream_worker( xclhwemhal2::bitStreamArg& args, std::shared_ptr<hw_context>& ihw_context)
    {

        auto hwshim = static_cast<xclhwemhal2::HwEmShim*>(handle);
        if (!hwshim)
        {
            std::cerr << "\n handler pointer is invalid\n";
            return;
        }

        auto lddr = hwshim->getMemoryManager();
        initMemoryManager(ihw_context, lddr);
        //auto lhwctx = xclLoad[iUUID];

        const mem_topology* m_mem = (reinterpret_cast<const ::mem_topology*>(args.m_memTopology));
        if (m_mem)
        {
            ihw_context->mMembanks.clear();
            for (int32_t i = 0; i < m_mem->m_count; ++i)
            {
                if (m_mem->m_mem_data[i].m_type == MEM_TYPE::MEM_STREAMING)
                    continue;
                std::string tag = reinterpret_cast<const char*>(m_mem->m_mem_data[i].m_tag);
                ihw_context->mMembanks.emplace_back(membank{ m_mem->m_mem_data[i].m_base_address, tag, m_mem->m_mem_data[i].m_size * 1024, i });
            }
            if (m_mem->m_count > 0)
            {
                ihw_context->mDDRMemoryManager.clear();
            }

            for (auto it : ihw_context->mMembanks)
            {
                //CR 966701: alignment to 4k (instead of mDeviceInfo.mDataAlignment)
                ihw_context->mDDRMemoryManager.push_back(new xclemulation::MemoryManager(it.size, it.base_addr, getpagesize(), it.tag));

                std::size_t found = it.tag.find("HOST");
                if (found != std::string::npos) {
                    ihw_context->host_sptag_idx = it.index;
                }
            }

            for (auto it : ihw_context->mDDRMemoryManager)
            {
                std::string tag = it->tag();

                //continue if not MBG group
                if (tag.find("MBG") == std::string::npos) {
                    continue;
                }

                // Connectivity provided with the bus direction for HBM[31:0], XCLBIN creates the large group of memory with all the HBM[31:0] size
                // like MBG. It indicates allocation of sequential memory is possible and not to limited size of one HBM. Hence creating the
                // HBM child memories (HBM subsets listed in RTD which falls under the range of MBG) for MBG memory type
                for (auto it2 : ihw_context->mDDRMemoryManager)
                {
                    if (it2->size() != 0 && it2 != it &&
                        it->start() <= it2->start() &&
                        (it->start() + it->size()) >= (it2->start() + it2->size()))
                    {
                        //add HBM child memories to MBG large group[
                        it->mChildMemories.push_back(it2);
                    }
                }
            }
        }

        xrt::xclbin xclbin_object{ args.m_top };
        ihw_context->sCUinfo.maie_partition_obj = xrt_core::xclbin::get_aie_partition(args.m_top);
        for (const auto& kernel : xclbin_object.get_kernels()) {
            DEBUG_MSGS_COUT(" the kernel name is \t ::" << kernel.get_name());

            auto props = xrt_core::xclbin_int::get_properties(kernel);

            for (const auto& cu : kernel.get_cus()) {
                // KDS information
                if (props.type == xrt_core::xclbin::kernel_properties::kernel_type::dpu)
                {
                    kds_info lkds;
                    lkds.base_addr = cu.get_base_address();
                    lkds.name = cu.get_name();
                    lkds.status = 0;   //dummy
                    lkds.usages = 0;   //dummy
                    ihw_context->sCUinfo.mCU_kds_info_list.push_back(lkds);

                    ihw_context->sCUinfo.mCUFunctionalMap[ihw_context->sCUinfo.mCUs++] = static_cast<uint32_t>(props.functional);
                    if (props.address_range != 0 && !props.name.empty() && !lkds.name.empty())
                    {
                        ihw_context->sCUinfo.mCURangeMap[lkds.name] = props.address_range;
                    }
                }

            }
        }

    }
   /*
    std::shared_ptr<hw_context> cIpuManager::get_hwctx(const uuid_t iUuid)
    {
        auto uuid = uuid_to_string(iUuid);
        return get_hwctx(uuid);
    }
    std::shared_ptr<hw_context> cIpuManager::get_hwctx(const std::string uuid)
    {

        std::lock_guard<std::mutex> lk{ mtx };
        auto itr = xclLoad.find(uuid);
        if (itr != xclLoad.end())
        {
            DEBUG_MSGS_COUT(" uuid is found and returning hwctx object." );
            return itr->second;
        }
        std::cerr << "\n uuid is NOT found and returning nullptr hwctx object \n";
        return  nullptr;
    }
    */
    std::shared_ptr<hw_context> cIpuManager::get_hwctx(const uint32_t iSlotID)
    {
        std::lock_guard<std::mutex> lk{ mtx };
        for (auto& x : m_loaded_xclbin_metadata)
        {
            if (x.first.mslot_uuid.first == int(iSlotID))
                return x.second;
        }

        std::cerr << "\n iSlotID is NOT found and returning nullptr hwctx object \n";
        return  nullptr;
    }
    std::string cIpuManager::uuid_to_string(const uuid_t iUUID)
    {
        uuid_t luuid;
        std::string oUUID;
        uuid_copy(luuid, iUUID);
        char c_uuid[100];
        uuid_unparse(luuid, c_uuid);
        oUUID.assign(c_uuid);

        return oUUID;
    }

}
