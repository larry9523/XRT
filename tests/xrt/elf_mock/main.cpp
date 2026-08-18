// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.

#include "mock_alloc.hpp"

#include "core/common/api/elf_patcher.h"
#include "core/common/api/module_int.h"

#include "xrt/experimental/xrt_elf.h"
#include "xrt/experimental/xrt_module.h"
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_hw_context.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void
usage(const char* prog)
{
  std::cout
    << "Usage: " << prog << " --elf <file.elf> [options]\n\n"
    << "Options:\n"
    << "  --elf <path>       ELF control-code binary (required)\n"
    << "  --ctrl-id <n>      Control-code id / group index (default: auto from --kernel)\n"
    << "  --kernel <name>    Kernel name for ctrl-code id lookup (default: first kernel)\n"
    << "  --patch-arg <name> Argument symbol to patch (repeatable)\n"
    << "  --patch-addr <hex> Fake device address for corresponding --patch-arg\n"
    << "  --load-only        Skip patching; only load ELF (+ module_run with --with-device)\n"
    << "  --with-device      Also run xrt::module_run + xrt::bo path (needs XRT_ELF_MOCK=1)\n"
    << "  -h                 Help\n\n"
    << "Environment:\n"
    << "  XRT_ELF_MOCK=1     Use libxrt_xdna_mock.so (malloc-backed BOs, no hardware)\n\n"
    << "Notes:\n"
    << "  Default patch test uses arg0/arg1 with malloc addresses when --patch-arg omitted.\n"
    << "  For legacy ELFs without .group sections, pass --kernel <name>.\n";
}

[[noreturn]] void
fail(const std::string& msg)
{
  throw std::runtime_error(msg);
}

std::string
platform_string(xrt::elf::platform p)
{
  switch (p) {
  case xrt::elf::platform::aie2p: return "aie2p";
  case xrt::elf::platform::aie2ps: return "aie2ps";
  case xrt::elf::platform::aie2ps_legacy: return "aie2ps_legacy";
  case xrt::elf::platform::aie4: return "aie4";
  case xrt::elf::platform::aie4a: return "aie4a";
  case xrt::elf::platform::aie4z: return "aie4z";
  default: return "unknown(" + std::to_string(static_cast<int>(p)) + ")";
  }
}

void
test_elf_api(const xrt::elf& elf, const std::string& elf_path)
{
  std::cout << "[elf] path=" << elf_path << "\n"
            << "[elf] platform=" << platform_string(elf.get_platform()) << "\n";

  try {
    std::cout << "[elf] partition_size=" << elf.get_partition_size() << "\n";
  }
  catch (const std::exception& ex) {
    std::cout << "[elf] partition_size=(unavailable: " << ex.what() << ")\n";
  }

  auto kernels = elf.get_kernels();
  std::cout << "[elf] kernels=" << kernels.size();
  for (const auto& krnl : kernels)
    std::cout << " " << krnl.get_name();
  std::cout << "\n";
}

uint32_t
resolve_ctrl_id(const xrt::elf& elf, const std::optional<uint32_t>& explicit_id,
                const std::string& kernel_name)
{
  if (explicit_id)
    return *explicit_id;

  auto elf_hdl = elf.get_handle();
  std::string lookup = kernel_name;

  if (lookup.empty()) {
    auto kernels = elf.get_kernels();
    if (kernels.empty())
      fail("ELF has no kernels; pass --ctrl-id explicitly");

    const auto& krnl = kernels.front();
    lookup = krnl.get_name();
    auto instances = krnl.get_instances();
    if (instances.size() == 1)
      lookup = lookup + ":" + instances.front().get_name();
    else if (instances.size() > 1)
      lookup = lookup + ":" + instances.front().get_name();

    std::cout << "[ctrl-id] auto-selected '" << lookup << "'\n";
  }
  else if (lookup.find(':') == std::string::npos) {
    for (const auto& krnl : elf.get_kernels()) {
      if (krnl.get_name() != lookup)
        continue;
      auto instances = krnl.get_instances();
      if (instances.size() == 1)
        lookup = lookup + ":" + instances.front().get_name();
      else if (instances.size() > 1)
        lookup = lookup + ":" + instances.front().get_name();
      break;
    }
  }

  auto id = elf_hdl->get_ctrlcode_id(lookup);
  std::cout << "[ctrl-id] '" << lookup << "' -> id " << id << "\n";
  return id;
}

void
test_module_load(const xrt::elf& elf, uint32_t ctrl_id)
{
  const char* mock = std::getenv("XRT_ELF_MOCK");
  if (!mock || std::string(mock).empty())
    fail("--with-device requires XRT_ELF_MOCK=1 (libxrt_xdna_mock.so)");

  xrt::device device{0};
  xrt::hw_context ctx{device, elf};
  xrt::bo empty_ctrlpkt{};

  auto mod = xrt_core::module_int::create_module_run(elf, ctx, ctrl_id, empty_ctrlpkt);

  auto platform = elf.get_platform();
  if (platform == xrt::elf::platform::aie2p) {
    auto sz = xrt_core::module_int::get_patch_buf_size(
      mod, xrt_core::elf_patcher::buf_type::ctrltext, ctrl_id);
    std::cout << "[module-run] aie2p ctrltext size=" << sz << " bytes\n";
    auto cp_sz = xrt_core::module_int::get_patch_buf_size(
      mod, xrt_core::elf_patcher::buf_type::ctrldata, ctrl_id);
    std::cout << "[module-run] aie2p ctrldata size=" << cp_sz << " bytes\n";
  }
  else {
    auto sz = xrt_core::module_int::get_patch_buf_size(
      mod, xrt_core::elf_patcher::buf_type::ctrltext, ctrl_id);
    std::cout << "[module-run] ctrltext size=" << sz << " bytes (device BO filled)\n";
  }

  std::cout << "[module-run] create_module_run OK (mock device, malloc-backed BOs)\n";
}

void
test_patch_malloc(const xrt::module& mod,
                  xrt::elf::platform platform,
                  uint32_t ctrl_id,
                  const std::vector<std::pair<std::string, uint64_t>>& args)
{
  auto type = xrt_core::elf_patcher::buf_type::ctrltext;
  size_t sz = xrt_core::module_int::get_patch_buf_size(mod, type, ctrl_id);
  if (sz == 0)
    fail("patch buffer size is zero");

  mock_alloc buf{sz};
  std::memset(buf.get(), 0xCD, sz);

  xrt_core::module_int::patch(mod, static_cast<uint8_t*>(buf.get()), sz, &args, type, ctrl_id);

  std::cout << "[patch-malloc] patched " << args.size() << " arg(s) into "
            << sz << "-byte ctrltext buffer @ "
            << std::hex << reinterpret_cast<uint64_t>(buf.get()) << std::dec << "\n";
}

void
test_patch_with_device(const xrt::elf& elf,
                       uint32_t ctrl_id,
                       const std::vector<std::pair<std::string, uint64_t>>& args)
{
  xrt::device device{0};
  xrt::hw_context ctx{device, elf};
  xrt::bo empty_ctrlpkt{};
  auto mod = xrt_core::module_int::create_module_run(elf, ctx, ctrl_id, empty_ctrlpkt);

  for (const auto& [name, addr] : args) {
    mock_alloc backing{4096};
    std::memset(backing.get(), 0, backing.size());

    xrt::bo arg_bo{ctx, backing.get(), 4096, xrt::bo::flags::host_only, xrt::memory_group{0}};
    xrt_core::module_int::patch(mod, name, 0, arg_bo);

    std::cout << "[patch-bo] " << name << " bo.address=0x"
              << std::hex << arg_bo.address() << std::dec << "\n";
  }

  std::cout << "[patch-bo] module_run created and patched via xrt::bo\n";
}

struct options
{
  std::string elf_path;
  std::optional<uint32_t> ctrl_id;
  std::string kernel_name;
  std::vector<std::pair<std::string, uint64_t>> patch_args;
  bool with_device = false;
  bool load_only = false;
};

options
parse_args(int argc, char** argv)
{
  options opt;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      usage(argv[0]);
      std::exit(0);
    }
    if (arg == "--with-device") {
      opt.with_device = true;
      continue;
    }
    if (arg == "--load-only") {
      opt.load_only = true;
      continue;
    }
    if (i + 1 >= argc)
      fail("missing value for " + arg);

    if (arg == "--elf")
      opt.elf_path = argv[++i];
    else if (arg == "--ctrl-id")
      opt.ctrl_id = static_cast<uint32_t>(std::stoul(argv[++i]));
    else if (arg == "--kernel")
      opt.kernel_name = argv[++i];
    else if (arg == "--patch-arg") {
      std::string name = argv[++i];
      opt.patch_args.emplace_back(name, 0);
    }
    else if (arg == "--patch-addr") {
      if (opt.patch_args.empty())
        fail("--patch-addr without preceding --patch-arg");
      uint64_t addr = std::stoull(argv[++i], nullptr, 0);
      opt.patch_args.back().second = addr;
    }
    else
      fail("unknown option: " + arg);
  }

  if (opt.elf_path.empty())
    fail("missing --elf");
  return opt;
}

} // namespace

int
main(int argc, char** argv)
{
  try {
    auto opt = parse_args(argc, argv);

    xrt::elf elf{opt.elf_path};
    test_elf_api(elf, opt.elf_path);

    auto ctrl_id = resolve_ctrl_id(elf, opt.ctrl_id, opt.kernel_name);

    if (opt.with_device)
      test_module_load(elf, ctrl_id);

    if (!opt.load_only) {
      xrt::module mod{elf};
      std::vector<std::pair<std::string, uint64_t>> patch_args = opt.patch_args;
      std::vector<mock_alloc> holders;

      if (patch_args.empty()) {
        holders.emplace_back(4096);
        holders.emplace_back(4096);
        patch_args = {
          {"arg0", reinterpret_cast<uint64_t>(holders[0].get())},
          {"arg1", reinterpret_cast<uint64_t>(holders[1].get())},
        };
        std::cout << "[patch-malloc] using demo args arg0/arg1 with malloc addresses\n";
      }
      else {
        for (auto& [name, addr] : patch_args) {
          if (addr == 0) {
            holders.emplace_back(4096);
            addr = reinterpret_cast<uint64_t>(holders.back().get());
          }
        }
      }

      test_patch_malloc(mod, elf.get_platform(), ctrl_id, patch_args);

      if (opt.with_device)
        test_patch_with_device(elf, ctrl_id, patch_args);
    }

    std::cout << "PASS\n";
    return 0;
  }
  catch (const std::exception& ex) {
    std::cerr << "FAIL: " << ex.what() << "\n";
    return 1;
  }
}
