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
    << "  --ctrl-id <n>      Control-code id / group index (default 0)\n"
    << "  --patch-arg <name> Argument symbol to patch (repeatable)\n"
    << "  --patch-addr <hex> Fake device address for corresponding --patch-arg\n"
    << "  --with-device      Also run xrt::module + xrt::bo path (needs XRT_ELF_MOCK=1)\n"
    << "  -h                 Help\n\n"
    << "Environment:\n"
    << "  XRT_ELF_MOCK=1     Use libxrt_xdna_mock.so (malloc-backed BOs, no hardware)\n\n"
    << "Notes:\n"
    << "  Default tests use module_int::patch() into a malloc buffer — no device required.\n"
    << "  BO addresses for patching can be any uint64_t (typically host pointer values).\n";
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

xrt_core::elf_patcher::buf_type
ctrltext_buf_type(xrt::elf::platform p)
{
  if (p == xrt::elf::platform::aie2p)
    return xrt_core::elf_patcher::buf_type::ctrltext;
  return xrt_core::elf_patcher::buf_type::ctrltext;
}

void
test_elf_api(const std::string& elf_path)
{
  xrt::elf elf{elf_path};
  auto platform = elf.get_platform();

  std::cout << "[elf] path=" << elf_path << "\n"
            << "[elf] platform=" << platform_string(platform) << "\n"
            << "[elf] partition_size=" << elf.get_partition_size() << "\n";

  auto names = elf.get_kernel_names();
  std::cout << "[elf] kernels=" << names.size();
  for (const auto& n : names)
    std::cout << " " << n;
  std::cout << "\n";
}

void
test_patch_malloc(const xrt::module& mod,
                  xrt::elf::platform platform,
                  uint32_t ctrl_id,
                  const std::vector<std::pair<std::string, uint64_t>>& args)
{
  auto type = ctrltext_buf_type(platform);
  size_t sz = xrt_core::module_int::get_patch_buf_size(mod, type, ctrl_id);
  if (sz == 0)
    fail("patch buffer size is zero");

  mock_alloc buf{sz};
  std::memset(buf.get(), 0xCD, sz);

  xrt_core::module_int::patch(mod, buf.get(), sz, &args, type, ctrl_id);

  std::cout << "[patch-malloc] patched " << args.size() << " arg(s) into "
            << sz << "-byte ctrltext buffer @ "
            << std::hex << reinterpret_cast<uint64_t>(buf.get()) << std::dec << "\n";
}

void
test_patch_with_device(const std::string& elf_path,
                       uint32_t ctrl_id,
                       const std::vector<std::pair<std::string, uint64_t>>& args)
{
  const char* mock = std::getenv("XRT_ELF_MOCK");
  if (!mock || std::string(mock).empty())
    fail("--with-device requires XRT_ELF_MOCK=1 (libxrt_xdna_mock.so)");

  xrt::elf elf{elf_path};
  xrt::device device{0};
  xrt::hw_context ctx{device, elf, xrt::hw_context::access_mode::shared};

  xrt::bo empty_ctrlpkt{};
  auto mod = xrt_core::module_int::create_module_run(elf, ctx, ctrl_id, empty_ctrlpkt);

  for (const auto& [name, addr] : args) {
    mock_alloc backing{4096};
    std::memset(backing.get(), 0, backing.size());

    xrt::bo arg_bo{device, backing.get(), 4096, xrt::bo::flags::host_only};
    xrt_core::module_int::patch(mod, name, 0, arg_bo);

    std::cout << "[patch-bo] " << name << " bo.address=0x"
              << std::hex << arg_bo.address() << std::dec << "\n";
  }

  std::cout << "[patch-bo] module_run created and patched via xrt::bo\n";
}

struct options
{
  std::string elf_path;
  uint32_t ctrl_id = 0;
  std::vector<std::pair<std::string, uint64_t>> patch_args;
  bool with_device = false;
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
    if (i + 1 >= argc)
      fail("missing value for " + arg);

    if (arg == "--elf")
      opt.elf_path = argv[++i];
    else if (arg == "--ctrl-id")
      opt.ctrl_id = static_cast<uint32_t>(std::stoul(argv[++i]));
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

    test_elf_api(opt.elf_path);

    xrt::elf elf{opt.elf_path};
    xrt::module mod{elf};

    if (opt.patch_args.empty()) {
      mock_alloc a{4096};
      mock_alloc b{4096};
      std::vector<std::pair<std::string, uint64_t>> demo = {
        {"ifm", reinterpret_cast<uint64_t>(a.get())},
        {"ofm", reinterpret_cast<uint64_t>(b.get())},
      };
      std::cout << "[patch-malloc] using demo args ifm/ofm with malloc addresses\n";
      test_patch_malloc(mod, elf.get_platform(), opt.ctrl_id, demo);
    }
    else {
      // Fill unset addresses with malloc-backed values
      std::vector<mock_alloc> holders;
      for (auto& [name, addr] : opt.patch_args) {
        if (addr == 0) {
          holders.emplace_back(4096);
          addr = reinterpret_cast<uint64_t>(holders.back().get());
        }
      }
      test_patch_malloc(mod, elf.get_platform(), opt.ctrl_id, opt.patch_args);
    }

    if (opt.with_device)
      test_patch_with_device(opt.elf_path, opt.ctrl_id, opt.patch_args);

    std::cout << "PASS\n";
    return 0;
  }
  catch (const std::exception& ex) {
    std::cerr << "FAIL: " << ex.what() << "\n";
    return 1;
  }
}
