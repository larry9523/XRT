// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
// Copyright (C) 2019 Samsung Semiconductor, Inc
#define XCL_DRIVER_DLL_EXPORT
#define XRT_CORE_PCIE_WINDOWS_SOURCE
#include "shim.h"                      // this file implements shim.h
#include "core/common/xrt_profiling.h" // this file implements xrt_profiling.h

#include "xrt_mem.h"
#include "xclfeatures.h"
#include "core/common/config_reader.h"
#include "core/common/xclbin_parser.h"
#include "core/common/message.h"
#include "core/common/system.h"
#include "core/common/device.h"
#include "core/common/query_requests.h"
#include "core/common/AlignedAllocator.h"
#include "core/include/xcl_perfmon_parameters.h"

#include <windows.h>
#include <winioctl.h>
#include <setupapi.h>
#include <strsafe.h>
#include <crtdefs.h>


#include <cstring>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <regex>

#pragma warning(disable : 4100 4996)
#pragma comment (lib, "Setupapi.lib")

namespace { // private implementation details

struct shim
{
  using buffer_handle_type = xclBufferHandle; // xrt.h
  unsigned int m_devidx;
  bool m_locked = false;
  HANDLE m_dev;
  std::shared_ptr<xrt_core::device> m_core_device;

  // create shim object, open the device, store the device handle
  shim(unsigned int devidx)
    : m_devidx(devidx)
  {
    // open device associated with devidx
    m_dev = CreateFileW(L"\\\\.\\XRT-USER-0" XRT_USER_DEVICE_DEVICE_NAMESPACE,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        0,
                        OPEN_EXISTING,
                        0,
                        0);

    if (m_dev == INVALID_HANDLE_VALUE) {
      auto error = GetLastError();
      xrt_core::message::
        send(xrt_core::message::severity_level::error,"XRT", "CreateFile failed with error %d",error);
      throw std::runtime_error("CreateFile failed with error " + std::to_string(error));
    }

    m_core_device = xrt_core::get_userpf_device(this, devidx);
  }

  // destruct shim object, close the device
  ~shim()
  {
    // close the device
    CloseHandle(m_dev);
  }

  buffer_handle_type
  alloc_bo(size_t size, unsigned int flags)
  {
    HANDLE bufferHandle;
    DWORD error = ERROR_UNABLE_TO_CLEAN;
    XRT_CREATE_BO_ARGS createBOArgs;
    DWORD bytesWritten;
    xcl_bo_flags bo_flags{ flags };

    bufferHandle = CreateFileW(L"\\\\.\\XRT-USER-0" XRT_USER_DEVICE_BUFFER_OBJECT_NAMESPACE,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              0,
                              OPEN_EXISTING,
                              0,
                              0);

    //
    // If this call fails, check to figure out what the error is and report it.
    //
    if (bufferHandle == INVALID_HANDLE_VALUE) {

        error = GetLastError();

        xrt_core::message::
          send(xrt_core::message::severity_level::error, "XRT", "CreateFile failed with error %d", error);

        goto done;
    }

    //'size' needs to be multiple of 4K
    createBOArgs.Size = ((size % 4096) == 0) ? size : (((4096 + size) / 4096) * 4096);
    createBOArgs.BankNumber = bo_flags.bank;
    createBOArgs.Flags = flags;

    if (flags & XCL_BO_FLAGS_HOST_ONLY) {
        createBOArgs.BufferType = XRT_BUFFER_TYPE_HOST_ONLY;
    } else if (flags & XCL_BO_FLAGS_EXECBUF) {
        createBOArgs.BufferType = XRT_BUFFER_TYPE_EXECBUF;
    } else {
        createBOArgs.BufferType = XRT_BUFFER_TYPE_NORMAL;
    }

    if (!DeviceIoControl(bufferHandle,
                         IOCTL_KIPUDRV_CREATE_BO,
                         &createBOArgs,
                         sizeof(XRT_CREATE_BO_ARGS),
                         0,
                         0,
                         &bytesWritten,
                         nullptr)) {

        error = GetLastError();

        xrt_core::message::
          send(xrt_core::message::severity_level::error, "XRT", "DeviceIoControl 4 failed with error %d", error);

        goto done;
    }

    error = ERROR_SUCCESS;

done:

    if (error != ERROR_SUCCESS) {

        if (bufferHandle != INVALID_HANDLE_VALUE) {

            CloseHandle(bufferHandle);
            bufferHandle = INVALID_HANDLE_VALUE;

        }

    }

    return bufferHandle;
  }

  buffer_handle_type
  alloc_user_ptr_bo(void* userptr, size_t size, unsigned int flags)
  {
    HANDLE bufferHandle;
    DWORD error = ERROR_UNABLE_TO_CLEAN;
    XRT_USERPTR_BO_ARGS userPtrBO;
    DWORD bytesWritten;
    xcl_bo_flags bo_flags{ flags };

    bufferHandle = CreateFileW(L"\\\\.\\XRT-USER-0" XRT_USER_DEVICE_BUFFER_OBJECT_NAMESPACE,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               0,
                               OPEN_EXISTING,
                               0,
                               0);

    //
    // If this call fails, check to figure out what the error is and report it.
    //
    if (bufferHandle == INVALID_HANDLE_VALUE) {

      error = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error,"XRT", "CreateFile failed with error %d",error);

      goto done;

    }

    userPtrBO.Address = userptr;
    userPtrBO.Size = ((size % 4096) == 0) ? size : (((4096 + size) / 4096) * 4096);
    userPtrBO.BankNumber = bo_flags.bank; //16 bit BankNumber
    userPtrBO.BufferType = XRT_BUFFER_TYPE_USERPTR;
    userPtrBO.Flags = flags;

    if (!DeviceIoControl(bufferHandle,
                         IOCTL_KIPUDRV_USERPTR_BO,
                         &userPtrBO,
                         sizeof(XRT_USERPTR_BO_ARGS),
                         0,
                         0,
                         &bytesWritten,
                         nullptr)) {

      error = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error,"XRT", "DeviceIoControl 4 failed with error %d", error);

      goto done;
    }

    error = ERROR_SUCCESS;

  done:

    if (error != ERROR_SUCCESS) {

      if (bufferHandle != INVALID_HANDLE_VALUE) {

        CloseHandle(bufferHandle);
        bufferHandle = INVALID_HANDLE_VALUE;

      }

    }

    return bufferHandle;
  }


  void*
  map_bo(buffer_handle_type handle, bool write)
  {
    DWORD bytesWritten;
    XRT_MAP_BO_RESULT mapBO;
    DWORD  code;

    if (handle)
      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "IOCTL_KIPUDRV_MAP_BO");
    else {
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "IOCTL_KIPUDRV_MAP_BO: Invalid Handle");
      return nullptr;
    }

    if (!DeviceIoControl(handle,
                         IOCTL_KIPUDRV_MAP_BO,
                         0,
                         0,
                         &mapBO,
                         sizeof(XRT_MAP_BO_RESULT),
                         &bytesWritten,
                         nullptr)) {

      code = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "DeviceIoControl 3 failed with error %d", code);
      return nullptr;
    }
    else {

      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "Mapped Address = 0x%p"
             ,mapBO.MappedUserVirtualAddress);

      //
      // Now zero it...
      //
      //RP   memset(mapBO.MappedUserVirtualAddress,
      //RP	   0,
      //RP	   (size_t)sizeToAllocate);

      return (void *)mapBO.MappedUserVirtualAddress;
    }
  }

  int
  unmap_bo(buffer_handle_type handle, void* addr)
  {
    // TODO : Implement
    return 0;
  }

  void
  free_bo(buffer_handle_type handle)
  {
    //As per OSR, just close the handle of BO.
    if(handle)
      CloseHandle(handle);
  }

  int
  sync_bo(buffer_handle_type handle, xclBOSyncDirection dir, size_t size, size_t offset)
  {
    DWORD bytesWritten;
    DWORD  error;
    XRT_SYNC_BO_ARGS syncBo = { 0 };

    syncBo.Direction = (dir == XCL_BO_SYNC_BO_TO_DEVICE) ? XRT_BUFFER_DIRECTION_TO_DEVICE : XRT_BUFFER_DIRECTION_FROM_DEVICE;
    syncBo.Offset = offset;
    syncBo.Size = size;

    if (!DeviceIoControl(handle,
                         IOCTL_KIPUDRV_SYNC_BO,
                         &syncBo,
                         sizeof(XRT_SYNC_BO_ARGS),
                         nullptr,
                         0,
                         &bytesWritten,
                         nullptr)) {

      error = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "Sync write failed with error %d", error);

      return error;
    }

    return 0;
  }


  int
  open_context(uint32_t slot_idx, const xuid_t xclbin_id, unsigned int ip_idx, bool shared)
  {
    HANDLE deviceHandle = m_dev;
    XRT_CTX_ARGS ctxArgs = { 0 };
    DWORD bytesRet;

    ctxArgs.Operation = XRT_CTX_OP_ALLOC_CTX;
    ctxArgs.Flags = (shared) ? XRT_CTX_SHARED : XRT_CTX_EXCLUSIVE;
    ctxArgs.CuIndex = ip_idx;
    ctxArgs.SlotIdx = slot_idx;
    memcpy(&ctxArgs.XclBinUuid, xclbin_id, sizeof(xuid_t));

#if 0
    char str[512] = { 0 };
    uuid_unparse_lower(ctxArgs.XclBinUuid, str);
    xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "xclbin_uuid = %s\n", str);
#endif
    if (!DeviceIoControl(deviceHandle,
                         IOCTL_KIPUDRV_CTX,
                         &ctxArgs,
                         sizeof(XRT_CTX_ARGS),
                         NULL,
                         0,
                         &bytesRet,
                         NULL)) {

      auto error = GetLastError();

      if (error == ERROR_RETRY) {

        xrt_core::message::
            send(xrt_core::message::severity_level::error, "XRT", "CTX failed retrying..");

        error = EAGAIN;

        return error;

      }

      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "CTX failed with error %d", error);
      return error;
    }

    return 0;
  }

  int
  open_context(uint32_t slot, const xuid_t xclbin_id, const char* cuname, bool shared)
  {
    // TODO: implement
    // For now default to single slot behavior
    return open_context(slot, xclbin_id, m_core_device->get_cuidx(slot, cuname).index, shared);
  }

  int
  close_context(const xuid_t xclbin_id, unsigned int ip_idx)
  {
    HANDLE deviceHandle = m_dev;
    XRT_CTX_ARGS ctxArgs = { 0 };
    DWORD bytesRet;

    ctxArgs.Operation = XRT_CTX_OP_FREE_CTX;
    ctxArgs.CuIndex = ip_idx;
    memcpy(&ctxArgs.XclBinUuid, xclbin_id, sizeof(xuid_t));

    if (!DeviceIoControl(deviceHandle,
                         IOCTL_KIPUDRV_CTX,
                         &ctxArgs,
                         sizeof(XRT_CTX_ARGS),
                         NULL,
                         0,
                         &bytesRet,
                         NULL)) {

      auto error = GetLastError();
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "CTX failed with error %d", error);
      return error;
    }

    return 0;
  }

  int
  exec_buf(buffer_handle_type handle)
  {
    HANDLE deviceHandle = m_dev;
    XRT_EXECBUF_ARGS execArgs = { 0 };
    DWORD bytesRet;
    execArgs.ExecBO = handle;

    if (!DeviceIoControl(deviceHandle,
                         IOCTL_KIPUDRV_EXECBUF,
                         &execArgs,
                         sizeof(XRT_EXECBUF_ARGS),
                         NULL,
                         0,
                         &bytesRet,
                         NULL)) {

      auto error = GetLastError();
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "CTX failed with error %d", error);

      if (GetLastError() == ERROR_BAD_COMMAND) {

        //
        // Device is already configured, not really a problem...
        //
        xrt_core::message::
          send(xrt_core::message::severity_level::info, "XRT", "Device already configured!");
        return -1; //ERROR_SUCCESS;
      }

      return error;

    }
    return 0;
  }

  int
  exec_wait(int msec)
  {
    HANDLE deviceHandle = m_dev;
    BOOLEAN workToDo;
    XRT_EXECPOLL_ARGS pollArgs;
    DWORD error;
    DWORD commandsCompleted;

    workToDo = FALSE;
    commandsCompleted = 0;

    pollArgs.DelayInMS = msec;

    if (!DeviceIoControl(deviceHandle,
                         IOCTL_KIPUDRV_EXECPOLL,
                         &pollArgs,
                         sizeof(XRT_EXECPOLL_ARGS),
                         NULL,
                         0,
                         &commandsCompleted,
                         NULL)) {

      error = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT"
             ,"DeviceIoControl IOCTL_KIPUDRV_EXECPOLL failed with error %d", error);

      goto done;
    }

    workToDo = TRUE;

  done:

    return workToDo;

  }

  int
  get_bo_properties(buffer_handle_type handle, struct xclBOProperties* properties)
  {
    XRT_INFO_BO_RESULT infoBo = { 0 };
    DWORD error;
    DWORD bytesRet;

    if (!DeviceIoControl(handle,
                         IOCTL_KIPUDRV_INFO_BO,
                         NULL,
                         0,
                         &infoBo,
                         sizeof(XRT_INFO_BO_RESULT),
                         &bytesRet,
                         NULL)) {

      error = GetLastError();
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT"
             ,"get_bo_Properties - DeviceIoControl failed with error %d", error);
    }

    properties->handle = 0;
    properties->flags  = infoBo.Flags;
    properties->size   = infoBo.Size;
    properties->paddr  = infoBo.Paddr;

    return 0;
  }

  bool SendIoctlReadAxlf(PUCHAR ImageBuffer, DWORD BuffSize)
  {
    HANDLE deviceHandle = m_dev;
    DWORD error = 0;
    DWORD bytesWritten;
    size_t off = 0;
    uint64_t ksize = 0;
    uint64_t aie_size = 0;
    PXRT_READ_AXLF_ARGS axlf_obj = nullptr;

    auto top = reinterpret_cast<const axlf*>(ImageBuffer);

    auto kernels = xrt_core::xclbin::get_kernels(top);
    /* Calculate size of kernels */
    for (auto& kernel : kernels) {
        ksize += sizeof(kernel_info) + sizeof(argument_info) * (kernel.args.size() -1);
    }

    /* Calculate size of AIE partition information */
    auto aie_part = xrt_core::xclbin::get_aie_partition(top);
    aie_size += sizeof(aie_info) + sizeof(uint64_t) * (aie_part.start_col_list.size() ?
                                                (aie_part.start_col_list.size() - 1) : 0);

    /* create buffer of total size to be sent via ioctl*/
    std::vector<char> axlf_binary(aie_size + ksize + sizeof (XRT_READ_AXLF_ARGS));
    axlf_obj = reinterpret_cast<XRT_READ_AXLF_ARGS*>(axlf_binary.data());
	axlf_obj->ksize = ksize;

    /* To enhance CU subdevice and KDS/ERT, driver needs all details about kernels
     * while loading xclbin.
     *
     * Why we extract data from XML metadata?
     *  1. Kernel is NOT a good place to parse xml. It prefers binary.
     *  2. All kernel details are in the xml today.
     *
     * What would happen in the future?
     *  XCLBIN would contain fdt as metadata. At that time, this
     *  could be removed.
     *
     * Binary format:
     * +-----------------------+
     * | Kernel[0]             |
     * |   name[64]            |
     * |   anums               |
     * |   argument[0]         |
     * |   argument[1]         |
     * |   argument[...]       |
     * |-----------------------|
     * | Kernel[1]             |
     * |   name[64]            |
     * |   anums               |
     * |   argument[0]         |
     * |   argument[1]         |
     * |   argument[...]       |
     * |-----------------------|
     * | Kernel[...]           |
     * |   ...                 |
     * +-----------------------+
     */

    for (auto& kernel : kernels) {
        auto krnl = reinterpret_cast<kernel_info *>(&axlf_obj->data[0] + off);

        if (kernel.name.size() > sizeof(krnl->name))
            return 1;
        std::strncpy(krnl->name, kernel.name.c_str(), sizeof(krnl->name)-1);
        krnl->name[sizeof(krnl->name)-1] = '\0';
        krnl->anums = kernel.args.size();
        krnl->range = kernel.range;

        int ai = 0;
        for (auto& arg : kernel.args) {
            if (arg.name.size() > sizeof(krnl->args[ai].name)) {

               xrt_core::message::
                send(xrt_core::message::severity_level::error, "XRT", "Argument name length invalid.");
               return 1;
            }
            std::strncpy(krnl->args[ai].name, arg.name.c_str(), sizeof(krnl->args[ai].name)-1);
            krnl->args[ai].name[sizeof(krnl->args[ai].name)-1] = '\0';
            krnl->args[ai].offset = arg.offset;
            krnl->args[ai].size   = arg.size;
            // XCLBIN doesn't define argument direction yet and it only support
            // input arguments.
            // Driver use 1 for input argument and 2 for output.
            // Let's refine this line later.
            krnl->args[ai].dir    = 1;
            ai++;
        }
        off += sizeof(kernel_info) + sizeof(argument_info) * (kernel.args.size() - 1);
    }

    axlf_obj->asize = aie_size;
    auto ainfo = reinterpret_cast<aie_info*>(&axlf_obj->data[0] + off);

    // Set default partition using all 5 AIE columns
    // If there is AIE_PARTITION section in XCLBIN, we will call into
    // Resource Solver to require a partition. Otherwise, we will just
    // use the whole AIE array by default
    ainfo->npart = 1;
    ainfo->ncol = 5;
    ainfo->start_col_list[0] = 0;

    /* If aie metadata found we use that data */
    if (aie_part.ncol) {

        ainfo->npart = (uint32_t)aie_part.start_col_list.size();
        ainfo->ncol = aie_part.ncol;
        int i = 0;
        for (auto& start_col : aie_part.start_col_list) {

            ainfo->start_col_list[i] = start_col;
            i++;

        }

    }

    /* To make download xclbin and configure KDS/ERT as an atomic operation. */
    axlf_obj->kds_cfg.ert = 0;// xrt_core::config::get_ert();
    axlf_obj->kds_cfg.polling = 0;// xrt_core::config::get_ert_polling();
    axlf_obj->kds_cfg.cu_dma = 0;// xrt_core::config::get_ert_cudma();
    axlf_obj->kds_cfg.cu_isr = 0;// xrt_core::config::get_ert_cuisr() && xrt_core::xclbin::get_cuisr(top);
    axlf_obj->kds_cfg.cq_int = 0;// xrt_core::config::get_ert_cqint();
    axlf_obj->kds_cfg.dataflow = 0;// xrt_core::config::get_feature_toggle("Runtime.dataflow") || xrt_core::xclbin::get_dataflow(top);
    axlf_obj->kds_cfg.rw_shared = 0;// xrt_core::config::get_rw_shared();

    /* TODO: In scheduler.cpp init() function, it use get_ert_slots(void) to get slot size.
     * But we cannot do this here, since the xclbin is not registered.
     * Currently, emulation flow use get_ert_slots() as well.
     * We will consider how to better determine slot size in new kds.
     */
    //axlf_obj.kds_cfg.slot_size = mCoreDevice->get_ert_slots().second;
    auto xml_hdr = xrt_core::xclbin::get_axlf_section(top, EMBEDDED_METADATA);
    if (!xml_hdr)
        throw std::runtime_error("No xml metadata in xclbin");
    auto xml_size = xml_hdr->m_sectionSize;
    auto xml_data = reinterpret_cast<const char*>(reinterpret_cast<const char*>(top) + xml_hdr->m_sectionOffset);
    axlf_obj->kds_cfg.slot_size = (uint32_t)m_core_device->get_ert_slots(xml_data, xml_size).second;

    if (!DeviceIoControl(deviceHandle,
                         IOCTL_KIPUDRV_DOWNLOAD_XCLBIN,
                         ImageBuffer,
                         BuffSize,
                         axlf_obj,
                         (DWORD)axlf_binary.size(),
                         &bytesWritten,
                         nullptr)) {

      error = GetLastError();

      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "DeviceIoControl failed with error %d", error);
    }

    return error ? false : true;

  }

  int
  load_xclbin(const struct axlf* buffer)
  {
    DWORD buffSize = 0;
    bool succeeded;

    //
    // FIrst test
    //
    buffSize = (DWORD) buffer->m_header.m_length;

    xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_READ_AXLF... ");

    succeeded = SendIoctlReadAxlf((PUCHAR)buffer, buffSize);

    if (succeeded) {
      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "OK");
    }
    else {
      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "FAILED");
      return 1;
    }

    //
    // Second test...
    //
    xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_STAT (Kipudrv StatMemTopology)... ");


    if (succeeded) {
      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "OK");
    }
    else {
      xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "FAILED");
      return 1;
    }

    return 0;
  }




  bool
  lock_device()
  {
    if (!xrt_core::config::get_multiprocess() && m_locked)
      return false;

    return m_locked = true;
  }

  bool
  unlock_device()
  {
    m_locked = false;
    return true;
  }
  void
  get_sensor_info(xcl_sensor* value)
  {
    DWORD bytes = 0;
    bool status = DeviceIoControl(m_dev,
        IOCTL_KIPUDRV_SENSOR_INFO,
        nullptr,
        0,
        value,
        sizeof(xcl_sensor),
        &bytes,
        nullptr);

    if (!status || bytes != sizeof(xcl_sensor))
      throw std::runtime_error("DeviceIoControl DeviceIoControl (get_sensor_info) failed");
  }

  void
  get_board_info(xcl_board_info* value)
  {
    DWORD bytes = 0;
    bool status = DeviceIoControl(m_dev,
        IOCTL_KIPUDRV_BOARD_INFO,
        nullptr,
        0,
        value,
        sizeof(xcl_board_info),
        &bytes,
        nullptr);

    if (!status || bytes != sizeof(xcl_board_info))
      throw std::runtime_error("DeviceIoControl IOCTL_KIPUDRV_BOARD_INFO (get_board_info) failed");
  }

  void
  get_bdf_info(uint16_t bdf[3])
  {
    // TODO: code share with mgmt
    GUID guid = GUID_DEVINTERFACE_KIPUDRV;
    auto hdevinfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    SP_DEVINFO_DATA dev_info_data;
    dev_info_data.cbSize = sizeof(dev_info_data);
    DWORD size;
    SetupDiEnumDeviceInfo(hdevinfo, m_devidx, &dev_info_data);
    SetupDiGetDeviceRegistryProperty(hdevinfo, &dev_info_data, SPDRP_LOCATION_INFORMATION,
                                     nullptr, nullptr, 0, &size);
    std::string buf(static_cast<size_t>(size), 0);
    SetupDiGetDeviceRegistryProperty(hdevinfo, &dev_info_data, SPDRP_LOCATION_INFORMATION,
                                     nullptr, (PBYTE)buf.data(), size, nullptr);

    std::regex regex("\\D+(\\d+)\\D+(\\d+)\\D+(\\d+)");
    std::smatch match;
    if (std::regex_search(buf, match, regex))
      std::transform(match.begin() + 1, match.end(), bdf,
                     [](const auto& m) {
                       return static_cast<uint16_t>(std::stoi(m.str()));
                     });
  }

  xrt_core::query::kds_cu_info::result_type
  kds_cu_info()
  {

      bool succeeded = 0;
      HANDLE deviceHandle = m_dev;
      XRT_SLOT_INFORMATION slotInfo;
      PXRT_KDS_CU_INFORMATION kdsCuInfo;
      XRT_STAT_CLASS_ARGS statClass = { 0 };
      DWORD bytesRet;
      DWORD bytesRequired;
      DWORD i;
      xrt_core::query::kds_cu_info::result_type vec;

      xrt_core::message::
          send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_STAT (Kipudrv xclbin_slots)... ");

      statClass.StatClass = XrtStatXclinSlots;

      succeeded = DeviceIoControl(deviceHandle,
          IOCTL_KIPUDRV_STAT,
          &statClass,
          sizeof(XRT_STAT_CLASS_ARGS),
          &slotInfo,
          sizeof(XRT_SLOT_INFORMATION),
          &bytesRet,
          NULL);

      if (succeeded) {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "OK");
      }
      else {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "FAILED");

          return vec;
      }

      bytesRequired = FIELD_OFFSET(XRT_KDS_CU_INFORMATION, CuInfo);
      bytesRequired += (slotInfo.CuCount * sizeof(XRT_KDS_CU));

      std::vector<char> kdsCuInfo_vec(bytesRequired);
      kdsCuInfo = reinterpret_cast<PXRT_KDS_CU_INFORMATION>(kdsCuInfo_vec.data());

      xrt_core::message::
          send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_STAT (Kipudrv kds_cu_info)... ");

      statClass.StatClass = XrtStatKdsCU;

      succeeded = DeviceIoControl(m_dev,
          IOCTL_KIPUDRV_STAT,
          &statClass,
          sizeof(XRT_STAT_CLASS_ARGS),
          kdsCuInfo,
          bytesRequired,
          &bytesRet,
          NULL);

      if (succeeded) {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "OK");
      }
      else {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "FAILED");

          return vec;
      }


      for (i = 0; i < kdsCuInfo->CuCount; i++) {

          xrt_core::query::kds_cu_info::data data;

          data.slot_index = kdsCuInfo->CuInfo[i].SlotIdx;
          data.index = kdsCuInfo->CuInfo[i].CuIdx;
          data.name = kdsCuInfo->CuInfo[i].kname;
          data.base_addr = 0xdeadbeef;
          data.status = 0;
          data.usages = 0;

          vec.push_back(std::move(data));

      }

      return vec;
  }

  xrt_core::query::xclbin_slots::result_type
  xclbin_slots()
  {

      bool succeeded = 0;
      HANDLE deviceHandle = m_dev;
      XRT_SLOT_INFORMATION slotInfo;
      PXRT_KDS_CU_INFORMATION kdsCuInfo;
      XRT_STAT_CLASS_ARGS statClass = { 0 };
      DWORD bytesRet;
      DWORD bytesRequired;
      DWORD i;
      xrt_core::query::xclbin_slots::result_type vec;
      char str[512] = { 0 };

      xrt_core::message::
          send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_STAT (Kipudrv xclbin_slots)... ");

      statClass.StatClass = XrtStatXclinSlots;

      succeeded = DeviceIoControl(deviceHandle,
          IOCTL_KIPUDRV_STAT,
          &statClass,
          sizeof(XRT_STAT_CLASS_ARGS),
          &slotInfo,
          sizeof(XRT_SLOT_INFORMATION),
          &bytesRet,
          NULL);

      if (succeeded) {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "OK");
      }
      else {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "FAILED");
          return vec;
      }

      bytesRequired = FIELD_OFFSET(XRT_KDS_CU_INFORMATION, CuInfo);
      bytesRequired += (slotInfo.SlotCount * sizeof(XRT_KDS_CU));

      std::vector<char> kdsCuInfo_vec(bytesRequired);
      kdsCuInfo = reinterpret_cast<PXRT_KDS_CU_INFORMATION>(kdsCuInfo_vec.data());

      xrt_core::message::
          send(xrt_core::message::severity_level::debug, "XRT", "Calling IOCTL_KIPUDRV_STAT (Kipudrv kds_cu_info)... ");

      statClass.StatClass = XrtStatSlotInfo;

      succeeded = DeviceIoControl(m_dev,
          IOCTL_KIPUDRV_STAT,
          &statClass,
          sizeof(XRT_STAT_CLASS_ARGS),
          kdsCuInfo,
          bytesRequired,
          &bytesRet,
          NULL);

      if (succeeded) {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "OK");
      }
      else {

          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "FAILED");

          return vec;
      }

      for (i = 0; i < kdsCuInfo->CuCount; i++) {

          xrt_core::query::xclbin_slots::slot_info data;

          data.slot = kdsCuInfo->CuInfo[i].SlotIdx;
          uuid_unparse_lower((const unsigned char*)&kdsCuInfo->CuInfo[i].XclBinUuid, str);
          data.uuid = str;

          vec.push_back(std::move(data));

      }

      return vec;
  }
  void
  get_errors(char* buffer)
  {
      DWORD bytes = 0;
      auto err = reinterpret_cast<struct xcl_errors*>(buffer);

      bool status = DeviceIoControl(m_dev,
          IOCTL_KIPUDRV_ERROR_INFO,
          nullptr,
          0,
          err,
          sizeof(xcl_errors),
          &bytes,
          nullptr);

      if (!status || bytes != sizeof(xcl_errors))
          throw std::runtime_error("DeviceIoControl IOCTL_KIPUDRV_ERROR_INFO (errors) failed");

  }

  int
  ErrorInject(uint16_t num, uint16_t driver, uint16_t severity, uint16_t module, uint16_t eclass)
  {
      DWORD bytes = 0;
      XOCL_ERROR_INJECT_ARGS errorinject = { XOCL_ERROR_OP_INJECT, num, driver, severity, module, eclass };

      bool status = DeviceIoControl(m_dev,
          IOCTL_KIPUDRV_ERROR_INJECT,
          &errorinject,
          sizeof(errorinject),
          nullptr,
          0,
          &bytes,
          nullptr);

      if (status) {
          xrt_core::message::
              send(xrt_core::message::severity_level::debug, "XRT", "OK");
      }
      else {
          xrt_core::message::
              send(xrt_core::message::severity_level::error, "XRT", "DeviceIoControl IOCTL_KIPUDRV_ERROR_INJECT failed ");
          return 1;
      }

      return 0;
  }

}; // struct shim

shim*
get_shim_object(xclDeviceHandle handle)
{
  // TODO: Do some sanity check
  return reinterpret_cast<shim*>(handle);
}

shim*
get_shim_object(const xrt_core::device* device)
{
  // TODO: Do some sanity check
  return get_shim_object(device->get_device_handle());
}

}

namespace userpf {

xrt_core::query::kds_cu_info::result_type
kds_cu_info(const xrt_core::device* device)
{
  auto shim = get_shim_object(device);
  return shim->kds_cu_info();
}

xrt_core::query::xclbin_slots::result_type
xclbin_slots(const xrt_core::device* device)
{
  auto shim = get_shim_object(device);
  return shim->xclbin_slots();
}

void
get_rom_info(xclDeviceHandle hdl, FeatureRomHeader* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_rom_info()");
  //auto shim = get_shim_object(hdl);
  //shim->get_rom_info(value);
}

void
get_device_info(xclDeviceHandle hdl, XOCL_DEVICE_INFORMATION* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_device_info()");
  //auto shim = get_shim_object(hdl);
  //shim->get_device_info(value);
}

void
get_mem_topology(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_mem_topology()");
  //auto shim = get_shim_object(hdl);
  //shim->get_mem_topology(buffer, size, size_ret);
}

void
get_group_mem_topology(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_group_mem_topology()");
  //auto shim = get_shim_object(hdl);
  //shim->get_group_mem_topology(buffer, size, size_ret);
}

void
get_temp_by_mem_topology(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_temp_by_mem_topology()");
  //auto shim = get_shim_object(hdl);
  //shim->get_temp_by_mem_topology(buffer, size, size_ret);
}

void
get_memstat(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret, bool raw)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_memstat()");
  //auto shim = get_shim_object(hdl);
  //shim->get_memstat(buffer, size, size_ret, raw);
}

void
get_ip_layout(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_ip_layout()");
  //auto shim = get_shim_object(hdl);
  //shim->get_ip_layout(buffer, size, size_ret);
}

void
get_debug_ip_layout(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
    xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "get_debug_ip_layout()");
    //auto shim = get_shim_object(hdl);
    //shim->get_debug_ip_layout(buffer, size, size_ret);
}

void
get_bdf_info(xclDeviceHandle hdl, uint16_t bdf[3])
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_bdf_info()");
  auto shim = get_shim_object(hdl);
  shim->get_bdf_info(bdf);
}

void
get_mailbox_info(xclDeviceHandle hdl, xcl_mailbox* value)
{
  xrt_core::message::send(xrt_core::message::severity_level::debug, "XRT", "mailbox_info()");
  //auto shim = get_shim_object(hdl);
  //shim->get_mailbox_info(value);
}

void
get_sensor_info(xclDeviceHandle hdl, xcl_sensor* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "sensor_info()");
  shim* shim = get_shim_object(hdl);
  shim->get_sensor_info(value);
}

void
get_icap_info(xclDeviceHandle hdl, xcl_pr_region* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "icap_info()");
  //shim* shim = get_shim_object(hdl);
  //shim->get_icap_info(value);
}

void
get_board_info(xclDeviceHandle hdl, xcl_board_info* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "board_info()");
  shim* shim = get_shim_object(hdl);
  shim->get_board_info(value);
}

void
get_mig_ecc_info(xclDeviceHandle hdl, xcl_mig_ecc* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "mig_ecc_info()");
  //shim* shim = get_shim_object(hdl);
  //shim->get_mig_ecc_info(value);
}

void
get_firewall_info(xclDeviceHandle hdl, xcl_firewall* value)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "firewall_info()");
  //shim* shim = get_shim_object(hdl);
  //shim->get_firewall_info(value);
}

void
get_kds_custat(xclDeviceHandle hdl, char* buffer, DWORD size, int* size_ret)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "get_kds_custat()");
  //shim* shim = get_shim_object(hdl);
  //shim->get_kds_custat(buffer, size, size_ret);
}

void
get_errors(xclDeviceHandle hdl, char* buffer)
{
    xrt_core::message::
        send(xrt_core::message::severity_level::debug, "XRT", "xocl errors()");
    shim* shim = get_shim_object(hdl);
    shim->get_errors(buffer);
}
} // namespace userpf

// Basic
unsigned int
xclProbe()
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclProbe()");
  GUID guid = GUID_DEVINTERFACE_KIPUDRV;

  HDEVINFO device_info =
    SetupDiGetClassDevs((LPGUID) &guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (device_info == INVALID_HANDLE_VALUE) {
    xrt_core::message::
      send(xrt_core::message::severity_level::error, "XRT", "GetDevices INVALID_HANDLE_VALUE");
    return 0;
  }

  SP_DEVICE_INTERFACE_DATA device_interface;
  device_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

  // enumerate through devices
  DWORD index;
  for (index = 0;
       SetupDiEnumDeviceInterfaces(device_info, NULL, &guid, index, &device_interface);
       ++index) {

    // get required buffer size
    ULONG detailLength = 0;
    if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_interface, NULL, 0, &detailLength, NULL)
        && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "SetupDiGetDeviceInterfaceDetail - get length failed");
      break;
    }

    // allocate space for device interface detail
    auto dev_detail = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, detailLength));
    if (!dev_detail) {
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "HeapAlloc failed");
      break;
    }
    dev_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    // get device interface detail
    if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_interface, dev_detail, detailLength, NULL, NULL)) {
      xrt_core::message::
        send(xrt_core::message::severity_level::error, "XRT", "SetupDiGetDeviceInterfaceDetail - get detail failed");
      HeapFree(GetProcessHeap(), 0, dev_detail);
      break;
    }

    HeapFree(GetProcessHeap(), 0, dev_detail);
  }

  SetupDiDestroyDeviceInfoList(device_info);

  return index;
}

xclDeviceHandle
xclOpen(unsigned int deviceIndex, const char *logFileName, xclVerbosityLevel level)
{
  try {
    xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "xclOpen()");
    return new shim(deviceIndex);
  }
  catch (const xrt_core::error& ex) {
    xrt_core::send_exception_message(ex.what());
  }
  catch (const std::exception& ex) {
    xrt_core::send_exception_message(ex.what());
  }

  return nullptr;
}

void
xclClose(xclDeviceHandle handle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclClose()");
  auto shim = get_shim_object(handle);
  delete shim;
}


// XRT Buffer Management APIs
xclBufferHandle
xclAllocBO(xclDeviceHandle handle, size_t size, int unused, unsigned int flags)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclAllocBO()");
  auto shim = get_shim_object(handle);
  return shim->alloc_bo(size, flags);
}

xclBufferHandle
xclAllocUserPtrBO(xclDeviceHandle handle, void *userptr, size_t size, unsigned int flags)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclAllocUserPtrBO()");
  auto shim = get_shim_object(handle);
  return shim->alloc_user_ptr_bo(userptr, size, flags);
}

void*
xclMapBO(xclDeviceHandle handle, xclBufferHandle boHandle, bool write)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclMapBO()");
  auto shim = get_shim_object(handle);
  return shim->map_bo(boHandle, write);
}

int
xclUnmapBO(xclDeviceHandle handle, xclBufferHandle boHandle, void* addr)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclUnmapBO()");
  auto shim = get_shim_object(handle);
  return shim->unmap_bo(boHandle, addr);
}

void
xclFreeBO(xclDeviceHandle handle, xclBufferHandle boHandle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclFreeBO()");
  auto shim = get_shim_object(handle);
  return shim->free_bo(boHandle);
}

int
xclSyncBO(xclDeviceHandle handle, xclBufferHandle boHandle, xclBOSyncDirection dir, size_t size, size_t offset)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclSyncBO()");
  auto shim = get_shim_object(handle);
  return shim->sync_bo(boHandle, dir, size, offset);
}

int
xclCopyBO(xclDeviceHandle handle, xclBufferHandle dstBoHandle,
          xclBufferHandle srcBoHandle, size_t size, size_t dst_offset,
          size_t src_offset)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclCopyBO() NOT IMPLEMENTED");
  return ENOSYS;
}

int
xclReClock2(xclDeviceHandle handle, unsigned short region,
            const uint16_t* targetFreqMHz)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclReClock2() NOT IMPLEMENTED");
  return ENOSYS;
}

// Compute Unit Execution Management APIs
int
xclOpenContext(xclDeviceHandle handle, const xuid_t xclbinId, unsigned int ipIndex, bool shared)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclOpenContext()");
  auto shim = get_shim_object(handle);

  //Virtual resources are not currently supported by driver
  return (ipIndex == (unsigned int)-1) ? 0 : shim->open_context(0, xclbinId, ipIndex, shared);
}

int
xclOpenContextByName(xclDeviceHandle handle, uint32_t slot, const xuid_t xclbin_uuid, const char* cuname, bool shared)
{
  try {
    auto shim = get_shim_object(handle);
    return shim->open_context(slot, xclbin_uuid, cuname, shared);
  }
  catch (const xrt_core::error& ex) {
    xrt_core::send_exception_message(ex.what());
    return ex.get_code();
  }
  catch (const std::exception& ex) {
    xrt_core::send_exception_message(ex.what());
    return -ENOENT;
  }
}

int xclCloseContext(xclDeviceHandle handle, const xuid_t xclbinId, unsigned int ipIndex)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclCloseContext()");
  auto shim = get_shim_object(handle);

  //Virtual resources are not currently supported by driver
  return (ipIndex == (unsigned int) -1) ? 0 : shim->close_context(xclbinId, ipIndex);
}

int
xclExecBuf(xclDeviceHandle handle, xclBufferHandle cmdBO)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclExecBuf()");
  auto shim = get_shim_object(handle);
  return shim->exec_buf(cmdBO);
}

int
xclExecWait(xclDeviceHandle handle, int timeoutMilliSec)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclExecWait()");
  auto shim = get_shim_object(handle);
  return shim->exec_wait(timeoutMilliSec);
}

xclBufferExportHandle
xclExportBO(xclDeviceHandle handle, xclBufferHandle boHandle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclExportBO() NOT IMPLEMENTED");
  return INVALID_HANDLE_VALUE;
}

xclBufferHandle
xclImportBO(xclDeviceHandle handle, xclBufferExportHandle fd, unsigned flags)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclImportBO() NOT IMPLEMENTED");
  return INVALID_HANDLE_VALUE;
}

int
xclCloseExportHandle(xclBufferExportHandle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclCloseExportHandle() NOT IMPLEMENTED");
  return 0;
}

int
xclGetBOProperties(xclDeviceHandle handle, xclBufferHandle boHandle,
		   struct xclBOProperties *properties)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclGetBOProperties()");
  auto shim = get_shim_object(handle);
  return shim->get_bo_properties(boHandle,properties);
}

int
xclLoadXclBin(xclDeviceHandle handle, const struct axlf *buffer)
{
  try {
    xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "xclLoadXclbin()");
    auto shim = get_shim_object(handle);
    if (auto ret =shim->load_xclbin(buffer))
      return ret;
    auto core_device = xrt_core::get_userpf_device(shim);
    core_device->register_axlf(buffer);
    return 0;
  }
  catch (const xrt_core::error& ex) {
    xrt_core::send_exception_message(ex.what());
    return ex.get_code();
  }
  catch (const std::exception& ex) {
    xrt_core::send_exception_message(ex.what());
    return -EINVAL;
  }
}

unsigned int
xclVersion()
{
  return 2;
}

int
xclGetDeviceInfo2(xclDeviceHandle handle, struct xclDeviceInfo2 *info)
{
  std::memset(info, 0, sizeof(xclDeviceInfo2));
  info->mMagic = 0;
  info->mHALMajorVersion = XCLHAL_MAJOR_VER;
  info->mHALMinorVersion = XCLHAL_MINOR_VER;
  info->mMinTransferSize = 0;
  info->mDMAThreads = 2;
  info->mDataAlignment = 4096; // 4k

  auto shim = get_shim_object(handle);
  auto name = xrt_core::device_query<xrt_core::query::rom_vbnv>(shim->m_core_device);
  auto len = name.copy(info->mName, sizeof info->mName - 1, 0);
  info->mName[len] = 0;

  return 0;
}

int
xclLockDevice(xclDeviceHandle handle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclLockDevice()");
  auto shim = get_shim_object(handle);
  return shim->lock_device() ? 0 : 1;
}

int
xclUnlockDevice(xclDeviceHandle handle)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclUnlockDevice()");
  auto shim = get_shim_object(handle);
  return shim->unlock_device() ? 0 : 1;
}

ssize_t
xclUnmgdPwrite(xclDeviceHandle handle, unsigned int flags, const void *buf, size_t count, uint64_t offset)
{
  return 0;
}

ssize_t
xclUnmgdPread(xclDeviceHandle handle, unsigned int flags, void *buf, size_t count, uint64_t offset)
{
  return 0;
}

size_t xclReadBO(xclDeviceHandle handle, xclBufferHandle boHandle, void *dst, size_t size, size_t skip)
{
  xrt_core::message::
    send(xrt_core::message::severity_level::debug, "XRT", "xclReadBO()");
  //auto shim = get_shim_object(handle);
  //return shim->read_bo(boHandle, dst, size, skip);
    return 1;
}

void
xclGetDebugIpLayout(xclDeviceHandle hdl, char* buffer, size_t size, size_t* size_ret)
{
  //userpf::get_debug_ip_layout(hdl, buffer, size, size_ret);
}

int
xclErrorInject(xclDeviceHandle handle, uint16_t num, uint16_t driver, uint16_t severity, uint16_t module, uint16_t eclass)
{
  xrt_core::message::
      send(xrt_core::message::severity_level::debug, "XRT", "xclExecBuf()");
  auto shim = get_shim_object(handle);
  return shim->ErrorInject(num, driver, severity, module, eclass);
}


// Deprecated APIs
size_t
xclWrite(xclDeviceHandle handle, enum xclAddressSpace space, uint64_t offset, const void *hostbuf, size_t size)
{
    xrt_core::message::
        send(xrt_core::message::severity_level::error,"XRT", "xclWrite Not supported ");
    return size;
}

size_t
xclRead(xclDeviceHandle handle, enum xclAddressSpace space,
        uint64_t offset, void *hostbuf, size_t size)
{

    xrt_core::message::
        send(xrt_core::message::severity_level::error,"XRT", "xclRead Not supported ");
    int *data = (int*) hostbuf;
    *data = 0x2;
    return size;
}

// Restricted read/write on IP register space
int
xclRegWrite(xclDeviceHandle handle, uint32_t ipidx, uint32_t offset, uint32_t data)
{
  return 1;
}

int
xclRegRead(xclDeviceHandle handle, uint32_t ipidx, uint32_t offset, uint32_t* datap)
{
  return 1;
}

int
xclGetTraceBufferInfo(xclDeviceHandle handle, uint32_t nSamples,
                      uint32_t& traceSamples, uint32_t& traceBufSz)
{
  xrt_core::message::send(xrt_core::message::severity_level::debug, "XRT", "xclGetTraceBufferInfo()");
  uint32_t bytesPerSample = (XPAR_AXI_PERF_MON_0_TRACE_WORD_WIDTH / 8);
  traceBufSz = MAX_TRACE_NUMBER_SAMPLES * bytesPerSample;   /* Buffer size in bytes */
  traceSamples = nSamples;
  return 0;
}

int
xclReadTraceData(xclDeviceHandle handle, void* traceBuf, uint32_t traceBufSz,
                 uint32_t numSamples, uint64_t ipBaseAddress,
                 uint32_t& wordsPerSample)
{
  xrt_core::message::send(xrt_core::message::severity_level::debug, "XRT", "xclReadTraceData()");
  //auto shim = get_shim_object(handle);

  // Create trace buffer on host (requires alignment)
  const int traceBufWordSz = traceBufSz / 4;  // traceBufSz is in number of bytes
  uint32_t size = 0;

  wordsPerSample = (XPAR_AXI_PERF_MON_0_TRACE_WORD_WIDTH / 32);
  uint32_t numWords = numSamples * wordsPerSample;

  xrt_core::AlignedAllocator<uint32_t> alignedBuffer(AXI_FIFO_RDFD_AXI_FULL, traceBufWordSz);
  uint32_t* hostbuf = alignedBuffer.getBuffer();

  // Now read trace data
  memset((void *)hostbuf, 0, traceBufSz);
  // Iterate over chunks
  // NOTE: AXI limits this to 4K bytes per transfer
  uint32_t chunkSizeWords = 256 * wordsPerSample;
  if (chunkSizeWords > 1024) chunkSizeWords = 1024;
  uint32_t chunkSizeBytes = 4 * chunkSizeWords;
  uint32_t words=0;

  // Read trace a chunk of bytes at a time
  if (numWords > chunkSizeWords) {
    for (; words < (numWords-chunkSizeWords); words += chunkSizeWords) {
      #if 0
          if(mLogStream.is_open())
            mLogStream << __func__ << ": reading " << chunkSizeBytes << " bytes from 0x"
                          << std::hex << (ipBaseAddress + AXI_FIFO_RDFD_AXI_FULL) /*fifoReadAddress[0] or AXI_FIFO_RDFD*/ << " and writing it to 0x"
                          << (void *)(hostbuf + words) << std::dec << std::endl;
      #endif
      //shim->unmgd_pread(0 /*flags*/, (void *)(hostbuf + words) /*buf*/, chunkSizeBytes /*count*/, ipBaseAddress + AXI_FIFO_RDFD_AXI_FULL /*offset : or AXI_FIFO_RDFD*/);
      size += chunkSizeBytes;
    }
  }

  // Read remainder of trace not divisible by chunk size
  if (words < numWords) {
    chunkSizeBytes = 4 * (numWords - words);
#if 0
      if(mLogStream.is_open()) {
        mLogStream << __func__ << ": reading " << chunkSizeBytes << " bytes from 0x"
                      << std::hex << (ipBaseAddress + AXI_FIFO_RDFD_AXI_FULL) /*fifoReadAddress[0]*/ << " and writing it to 0x"
                      << (void *)(hostbuf + words) << std::dec << std::endl;
      }
#endif
    //shim->unmgd_pread(0 /*flags*/, (void *)(hostbuf + words) /*buf*/, chunkSizeBytes /*count*/, ipBaseAddress + AXI_FIFO_RDFD_AXI_FULL /*offset : or AXI_FIFO_RDFD*/);
    size += chunkSizeBytes;
  }
#if 0
    if(mLogStream.is_open())
        mLogStream << __func__ << ": done reading " << size << " bytes " << std::endl;
#endif
  memcpy((char*)traceBuf, (char*)hostbuf, traceBufSz);

  return size;
}

int
xclGetSubdevPath(xclDeviceHandle handle,  const char* subdev,
                 uint32_t idx, char* path, size_t size)
{
  return 0;
}

int
xclP2pEnable(xclDeviceHandle handle, bool enable, bool force)
{
  return 1; // -ENOSYS;
}

int
xclCmaEnable(xclDeviceHandle handle, bool enable, uint64_t force)
{
  return -ENOSYS;
}

int
xclUpdateSchedulerStat(xclDeviceHandle handle)
{
  return 1; // -ENOSYS;
}

int
xclInternalResetDevice(xclDeviceHandle handle, xclResetKind kind)
{
  return 1; // -ENOSYS;
}
