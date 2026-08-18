// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
//
// Minimal XDNA loopback mock: host memory BOs (malloc), instant command complete.
// Enable with: export XRT_ELF_MOCK=1

#define XCL_DRIVER_DLL_EXPORT
#define XRT_CORE_COMMON_SOURCE

#include "core/common/cuidx_type.h"
#include "core/common/device.h"
#include "core/common/error.h"
#include "core/common/ishim.h"
#include "core/common/query_requests.h"
#include "core/common/shim/buffer_handle.h"
#include "core/common/shim/hwctx_handle.h"
#include "core/common/shim/hwqueue_handle.h"
#include "core/common/system.h"
#include "core/include/shim_int.h"
#include "core/include/xrt/detail/ert.h"
#include "core/pcie/common/device_pcie.h"
#include "core/pcie/common/system_pcie.h"

#include "xrt/xrt_hw_context.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace xrt_core::xdna_mock {

// ---------------------------------------------------------------------------
// In-memory buffer object (malloc or caller userptr)
// ---------------------------------------------------------------------------
struct bo_entry
{
  void* ptr = nullptr;
  size_t size = 0;
  uint64_t flags = 0;
  bool owned = false;
  xclBufferHandle id = XRT_NULL_BO;
};

class mock_shim;

class malloc_buffer : public buffer_handle
{
  mock_shim* m_shim;
  xclBufferHandle m_id;

public:
  malloc_buffer(mock_shim* shim, xclBufferHandle id)
    : m_shim(shim)
    , m_id(id)
  {}

  std::unique_ptr<shared_handle>
  share() const override
  {
    throw std::runtime_error("xdna_mock: BO export not supported");
  }

  void*
  map(map_type) override;

  void
  unmap(void*) override
  {}

  void
  sync(direction, size_t, size_t) override
  {}

  void
  copy(const buffer_handle*, size_t, size_t, size_t) override
  {
    throw std::runtime_error("xdna_mock: BO copy not supported");
  }

  properties
  get_properties() const override;

  xclBufferHandle
  get_xcl_handle() const override
  {
    return m_id;
  }
};

// ---------------------------------------------------------------------------
// Hardware queue: mark ERT packets completed immediately
// ---------------------------------------------------------------------------
class mock_hwqueue : public hwqueue_handle
{
  mock_shim* m_shim;

  static void
  complete_cmd(buffer_handle* cmd)
  {
    if (!cmd)
      return;

    auto mapped = cmd->map(buffer_handle::map_type::write);
    auto* pkt = reinterpret_cast<ert_packet*>(mapped);
    if (pkt)
      pkt->state = ERT_CMD_STATE_COMPLETED;
    cmd->unmap(mapped);
  }

public:
  explicit
  mock_hwqueue(mock_shim* shim)
    : m_shim(shim)
  {}

  void
  submit_command(buffer_handle* cmd) override
  {
    complete_cmd(cmd);
  }

  int
  wait_command(buffer_handle* cmd, uint32_t) const override
  {
    complete_cmd(cmd);
    return 1;
  }
};

// ---------------------------------------------------------------------------
// Hardware context
// ---------------------------------------------------------------------------
class mock_hwctx : public hwctx_handle
{
  mock_shim* m_shim;
  hwctx_handle::slot_id m_slot = 0;
  std::unique_ptr<mock_hwqueue> m_queue;

public:
  mock_hwctx(mock_shim* shim, hwctx_handle::slot_id slot)
    : m_shim(shim)
    , m_slot(slot)
    , m_queue(std::make_unique<mock_hwqueue>(shim))
  {}

  hwctx_handle::slot_id
  get_slotidx() const override
  {
    return m_slot;
  }

  hwqueue_handle*
  get_hw_queue() override
  {
    return m_queue.get();
  }

  std::unique_ptr<buffer_handle>
  alloc_bo(void* userptr, size_t size, uint64_t flags) override;

  std::unique_ptr<buffer_handle>
  alloc_bo(size_t size, uint64_t flags) override
  {
    return alloc_bo(nullptr, size, flags);
  }

  cuidx_type
  open_cu_context(const std::string&) override
  {
    return cuidx_type{};
  }

  void
  close_cu_context(cuidx_type) override
  {}

  void
  exec_buf(buffer_handle* cmd) override
  {
    if (m_queue && cmd)
      m_queue->submit_command(cmd);
  }
};

// ---------------------------------------------------------------------------
// Shim device (coreutil-facing)
// ---------------------------------------------------------------------------
class mock_device;

class mock_shim
{
  friend class malloc_buffer;
  friend class mock_hwctx;

  xrt_core::device::id_type m_index;
  std::shared_ptr<xrt_core::device> m_core_device;
  std::mutex m_mutex;
  xclBufferHandle m_next_bo = 1;
  std::map<xclBufferHandle, bo_entry> m_bos;
  hwctx_handle::slot_id m_next_slot = 0;

  static mock_shim*
  check(xclDeviceHandle handle)
  {
    auto* shim = static_cast<mock_shim*>(handle);
    if (!shim)
      throw xrt_core::error("Invalid xdna_mock device handle");
    return shim;
  }

public:
  explicit
  mock_shim(xrt_core::device::id_type index)
    : m_index(index)
    , m_core_device(xrt_core::get_userpf_device(this, index))
  {}

  ~mock_shim()
  {
    for (auto& [id, bo] : m_bos) {
      if (bo.owned && bo.ptr)
        std::free(bo.ptr);
    }
  }

  static mock_shim*
  from(xclDeviceHandle handle)
  {
    return check(handle);
  }

  xrt_core::device::id_type
  get_index() const
  {
    return m_index;
  }

  bo_entry*
  get_bo_entry(xclBufferHandle id)
  {
    auto it = m_bos.find(id);
    if (it == m_bos.end())
      throw xrt_core::error("xdna_mock: unknown buffer handle");
    return &it->second;
  }

  xclBufferHandle
  alloc_bo_internal(void* userptr, size_t size, unsigned flags)
  {
    std::lock_guard lk(m_mutex);
    if (size == 0)
      throw xrt_core::error(EINVAL, "xdna_mock: invalid BO size");

    bo_entry entry;
    entry.size = size;
    entry.flags = flags;
    entry.id = m_next_bo++;

    if (userptr) {
      entry.ptr = userptr;
      entry.owned = false;
    }
    else {
      entry.ptr = std::aligned_alloc(4096, (size + 4095) & ~size_t(4095));
      if (!entry.ptr)
        throw xrt_core::system_error(ENOMEM, "xdna_mock: malloc failed");
      entry.owned = true;
      std::memset(entry.ptr, 0, size);
    }

    auto id = entry.id;
    m_bos.emplace(id, std::move(entry));
    return id;
  }

  void
  free_bo(xclBufferHandle id)
  {
    std::lock_guard lk(m_mutex);
    auto it = m_bos.find(id);
    if (it == m_bos.end())
      return;
    if (it->second.owned && it->second.ptr)
      std::free(it->second.ptr);
    m_bos.erase(it);
  }

  std::unique_ptr<buffer_handle>
  export_buffer(xclBufferHandle id)
  {
    check_bo(id);
    return std::make_unique<malloc_buffer>(this, id);
  }

  void
  check_bo(xclBufferHandle id)
  {
    if (m_bos.find(id) == m_bos.end())
      throw xrt_core::error("xdna_mock: invalid BO handle");
  }

  std::unique_ptr<hwctx_handle>
  create_hwctx()
  {
    std::lock_guard lk(m_mutex);
    auto slot = m_next_slot++;
    return std::make_unique<mock_hwctx>(this, slot);
  }

  hwqueue_handle*
  get_hw_queue(hwctx_handle* ctx)
  {
    return static_cast<mock_hwctx*>(ctx)->get_hw_queue();
  }
};

void*
malloc_buffer::map(map_type)
{
  return m_shim->get_bo_entry(m_id)->ptr;
}

buffer_handle::properties
malloc_buffer::get_properties() const
{
  const auto* bo = m_shim->get_bo_entry(m_id);
  return {bo->flags, bo->size, reinterpret_cast<uint64_t>(bo->ptr)};
}

std::unique_ptr<buffer_handle>
mock_hwctx::alloc_bo(void* userptr, size_t size, uint64_t flags)
{
  auto hdl = m_shim->alloc_bo_internal(userptr, size, static_cast<unsigned>(flags));
  return m_shim->export_buffer(hdl);
}

// Device query table (minimal Ryzen/NPU identity)
namespace {

namespace query = xrt_core::query;

static std::map<query::key_type, std::unique_ptr<query::request>> s_query_tbl;

struct mock_device_info
{
  static std::any
  device_class(const xrt_core::device*)
  {
    return query::device_class::type::ryzen;
  }

  static std::any
  vbnv(const xrt_core::device*)
  {
    return std::string("xdna-mock");
  }

  static std::any
  pcie_id(const xrt_core::device*)
  {
    query::pcie_id::data id{};
    id.device_id = 0x17f0;
    id.revision_id = 0x11;
    return id;
  }

  static std::any
  bdf(const xrt_core::device*)
  {
    return std::string("mock:00:00.0");
  }
};

template <typename QueryRequestType, std::any (*Getter)(const xrt_core::device*)>
struct func0_get : virtual QueryRequestType
{
  std::any
  get(const xrt_core::device* dev) const override
  {
    return Getter(dev);
  }
};

template <typename QueryRequestType, std::any (*Getter)(const xrt_core::device*)>
static void
emplace_query()
{
  s_query_tbl.emplace(QueryRequestType::key,
                    std::make_unique<func0_get<QueryRequestType, Getter>>());
}

struct query_init { query_init() {
  emplace_query<query::device_class, mock_device_info::device_class>();
  emplace_query<query::rom_vbnv, mock_device_info::vbnv>();
  emplace_query<query::pcie_id, mock_device_info::pcie_id>();
  emplace_query<query::pcie_bdf, mock_device_info::bdf>();
}};
static query_init s_query_init;

} // namespace

class mock_device : public shim<device_pcie>
{
public:
  mock_device(xrt_core::device::handle_type handle, xrt_core::device::id_type id, bool user)
    : shim<device_pcie>(handle, id, user)
  {}

  const query::request&
  lookup_query(query::key_type key) const override
  {
    auto it = s_query_tbl.find(key);
    if (it == s_query_tbl.end())
      throw query::no_such_key(key);
    return *(it->second);
  }

  std::unique_ptr<hwctx_handle>
  create_hw_context(const xrt::uuid&,
                    const xrt::hw_context::cfg_param_type&,
                    xrt::hw_context::access_mode) const override
  {
    return mock_shim::from(get_device_handle())->create_hwctx();
  }

  std::unique_ptr<hwctx_handle>
  create_hw_context(uint32_t,
                    const xrt::hw_context::cfg_param_type&,
                    xrt::hw_context::access_mode) const override
  {
    return mock_shim::from(get_device_handle())->create_hwctx();
  }

  std::unique_ptr<buffer_handle>
  alloc_bo(size_t size, uint64_t flags) override
  {
    return xrt::shim_int::alloc_bo(get_device_handle(), size, xcl_bo_flags{flags}.flags);
  }

  std::unique_ptr<buffer_handle>
  alloc_bo(void* userptr, size_t size, uint64_t flags) override
  {
    return xrt::shim_int::alloc_bo(get_device_handle(), userptr, size, xcl_bo_flags{flags}.flags);
  }
};

// ---------------------------------------------------------------------------
// XRT system (replaces pci::system_linux when libxrt_xdna_mock is loaded)
// ---------------------------------------------------------------------------
class system : public system_pcie
{
public:
  system()
  {
    xclProbe();
  }

  std::pair<xrt_core::device::id_type, xrt_core::device::id_type>
  get_total_devices(bool) const override
  {
    auto n = xclProbe();
    return {n, n};
  }

  std::shared_ptr<xrt_core::device>
  get_userpf_device(xrt_core::device::id_type id) const override
  {
    return xrt_core::get_userpf_device(xclOpen(id, nullptr, XCL_QUIET));
  }

  std::shared_ptr<xrt_core::device>
  get_userpf_device(xrt_core::device::handle_type handle,
                    xrt_core::device::id_type id) const override
  {
    return std::make_shared<mock_device>(handle, id, true);
  }

  std::shared_ptr<xrt_core::device>
  get_mgmtpf_device(xrt_core::device::id_type) const override
  {
    throw std::runtime_error("xdna_mock: no mgmt device");
  }

  void
  program_plp(const xrt_core::device*, const std::vector<char>&, bool) const override
  {
    throw std::runtime_error("xdna_mock: program_plp not supported");
  }
};

static system*
singleton()
{
  static system s;
  return &s;
}

struct system_init { system_init() { singleton(); } };
static system_init s_system_init;

std::shared_ptr<xrt_core::device>
get_userpf_device(xrt_core::device::handle_type handle, xrt_core::device::id_type id)
{
  return singleton()->get_userpf_device(handle, id);
}

} // namespace xrt_core::xdna_mock

// ---------------------------------------------------------------------------
// C HAL API (exported from libxrt_xdna_mock.so)
// ---------------------------------------------------------------------------
static unsigned int s_device_count = 0;
static std::map<unsigned int, xrt_core::xdna_mock::mock_shim*> s_open_devices;

extern "C" {

unsigned int
xclProbe()
{
  s_device_count = 1;
  return s_device_count;
}

xclDeviceHandle
xclOpen(unsigned int deviceIndex, const char*, xclVerbosityLevel)
{
  if (deviceIndex >= s_device_count)
    return nullptr;

  auto it = s_open_devices.find(deviceIndex);
  if (it != s_open_devices.end())
    return it->second;

  auto* shim = new xrt_core::xdna_mock::mock_shim(deviceIndex);
  s_open_devices[deviceIndex] = shim;
  return shim;
}

void
xclClose(xclDeviceHandle handle)
{
  auto* shim = xrt_core::xdna_mock::mock_shim::from(handle);
  s_open_devices.erase(shim->get_index());
  delete shim;
}

size_t
xclWrite(xclDeviceHandle, xclAddressSpace, uint64_t, const void*, size_t)
{
  return 0;
}

size_t
xclRead(xclDeviceHandle, xclAddressSpace, uint64_t, void*, size_t)
{
  return 0;
}

int
xclGetDeviceInfo2(xclDeviceHandle, xclDeviceInfo2* info)
{
  if (!info)
    return -EINVAL;
  std::memset(info, 0, sizeof(*info));
  std::strncpy(info->mName, "amd:xdna-mock:1.0", sizeof(info->mName) - 1);
  info->mMagic = 0x586C0C6C;
  return 0;
}

xclBufferHandle
xclAllocBO(xclDeviceHandle handle, size_t size, int, unsigned flags)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->alloc_bo_internal(nullptr, size, flags);
}

xclBufferHandle
xclAllocUserPtrBO(xclDeviceHandle handle, void* userptr, size_t size, unsigned flags)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->alloc_bo_internal(userptr, size, flags);
}

void
xclFreeBO(xclDeviceHandle handle, xclBufferHandle bo)
{
  xrt_core::xdna_mock::mock_shim::from(handle)->free_bo(bo);
}

void*
xclMapBO(xclDeviceHandle handle, xclBufferHandle bo, bool)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->get_bo_entry(bo)->ptr;
}

int
xclUnmapBO(xclDeviceHandle, xclBufferHandle, void*)
{
  return 0;
}

int
xclSyncBO(xclDeviceHandle, xclBufferHandle, xclBOSyncDirection, size_t, size_t)
{
  return 0;
}

int
xclGetBOProperties(xclDeviceHandle handle, xclBufferHandle bo, xclBOProperties* prop)
{
  if (!prop)
    return -EINVAL;
  const auto* entry = xrt_core::xdna_mock::mock_shim::from(handle)->get_bo_entry(bo);
  prop->handle = bo;
  prop->flags = entry->flags;
  prop->size = entry->size;
  prop->paddr = reinterpret_cast<uint64_t>(entry->ptr);
  return 0;
}

int
xclOpenContext(xclDeviceHandle, const uuid_t, unsigned int, bool)
{
  return 0;
}

int
xclCloseContext(xclDeviceHandle, const uuid_t, unsigned int)
{
  return 0;
}

int
xclExecBuf(xclDeviceHandle handle, unsigned int cmdBO)
{
  auto* entry = xrt_core::xdna_mock::mock_shim::from(handle)->get_bo_entry(cmdBO);
  if (!entry || !entry->ptr)
    return -EINVAL;

  auto* pkt = reinterpret_cast<ert_packet*>(entry->ptr);
  if (pkt)
    pkt->state = ERT_CMD_STATE_COMPLETED;
  return 0;
}

int
xclExecWait(xclDeviceHandle, int)
{
  return 1;
}

int
xclRegWrite(xclDeviceHandle, uint32_t, uint32_t, uint32_t)
{
  return 0;
}

int
xclRegRead(xclDeviceHandle, uint32_t, uint32_t, uint32_t* datap)
{
  if (datap)
    *datap = 0;
  return 0;
}

ssize_t
xclUnmgdPread(xclDeviceHandle, unsigned, void*, size_t, uint64_t)
{
  return 0;
}

ssize_t
xclUnmgdPwrite(xclDeviceHandle, unsigned, const void*, size_t, uint64_t)
{
  return 0;
}

int
xclLoadXclBin(xclDeviceHandle, const struct axlf*)
{
  return 0;
}

int
xclReClock2(xclDeviceHandle, unsigned short, const unsigned short*)
{
  return 0;
}

int
xclP2pEnable(xclDeviceHandle, bool, bool)
{
  return 0;
}

} // extern "C"

int
xclCmaEnable(xclDeviceHandle, bool, uint64_t)
{
  return 0;
}

int
xclInternalResetDevice(xclDeviceHandle, xclResetKind)
{
  return 0;
}

int
xclUpdateSchedulerStat(xclDeviceHandle)
{
  return 0;
}

namespace xrt::shim_int {

std::unique_ptr<xrt_core::hwctx_handle>
create_hw_context(xclDeviceHandle handle,
                  const xrt::uuid&,
                  const xrt::hw_context::cfg_param_type&,
                  xrt::hw_context::access_mode)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->create_hwctx();
}

std::unique_ptr<xrt_core::hwctx_handle>
create_hw_context(xclDeviceHandle handle, uint32_t)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->create_hwctx();
}

std::unique_ptr<xrt_core::buffer_handle>
alloc_bo(xclDeviceHandle handle, size_t size, unsigned int flags)
{
  auto id = xrt_core::xdna_mock::mock_shim::from(handle)->alloc_bo_internal(nullptr, size, flags);
  return xrt_core::xdna_mock::mock_shim::from(handle)->export_buffer(id);
}

std::unique_ptr<xrt_core::buffer_handle>
alloc_bo(xclDeviceHandle handle, void* userptr, size_t size, unsigned int flags)
{
  auto id = xrt_core::xdna_mock::mock_shim::from(handle)->alloc_bo_internal(userptr, size, flags);
  return xrt_core::xdna_mock::mock_shim::from(handle)->export_buffer(id);
}

xrt_core::hwqueue_handle*
get_hw_queue(xclDeviceHandle handle, xrt_core::hwctx_handle* ctx)
{
  return xrt_core::xdna_mock::mock_shim::from(handle)->get_hw_queue(ctx);
}

void
submit_command(xclDeviceHandle handle, xrt_core::hwqueue_handle* q, xrt_core::buffer_handle* cmd)
{
  if (q && cmd)
    q->submit_command(cmd);
  else if (auto* shim = xrt_core::xdna_mock::mock_shim::from(handle); q && cmd)
    shim->get_hw_queue(nullptr)->submit_command(cmd);
}

int
wait_command(xclDeviceHandle, xrt_core::hwqueue_handle* q, xrt_core::buffer_handle* cmd, int timeout_ms)
{
  if (q && cmd)
    return q->wait_command(cmd, timeout_ms);
  return 0;
}

} // namespace xrt::shim_int
