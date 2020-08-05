#include "core/include/experimental/xrt_aie.h"
#include "core/common/system.h"
#include "core/common/device.h"
#include "core/common/xrt_graph.h"

namespace xrt {

class graph_impl {
private:
  std::shared_ptr<xrt_core::device> device;
  xclGraphHandle handle;

public:
  graph_impl(xclDeviceHandle dhdl, xclGraphHandle ghdl)
    : device(xrt_core::get_userpf_device(dhdl)), handle(ghdl)
  {}

  xclGraphHandle
  get_handle() const
  {
    return handle;
  }
};

}

static std::shared_ptr<xrt::graph_impl>
open_graph(xclDeviceHandle dhdl, const uuid_t xclbin_uuid, const char* graph_name)
{
  auto device = xrt_core::get_userpf_device(dhdl);
  auto handle = device->open_graph(xclbin_uuid, graph_name);
  auto ghdl = std::make_shared<xrt::graph_impl>(dhdl, handle);
  return ghdl;
}

xrtGraphHandle
xrtGraphOpen(xclDeviceHandle handle, const uuid_t xclbin_uuid, const char* graph)
{
  try {
    auto graph_hdl = open_graph(handle, xclbin_uuid, graph);
    return graph_hdl.get();
  }
  catch (const std::exception& ex) {
    xrt_core::send_exception_message(ex.what());
    return XRT_NULL_HANDLE;
  }
}
