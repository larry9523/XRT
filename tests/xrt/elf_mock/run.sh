#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Quick helper to build and run elf_mock against the local XRT tree.
#
# Example ELFs (after building XRT with aiebu tests):
#   $XRT_ROOT/build/Release/runtime_src/core/common/aiebu/test/aie2ps-ctrlcode/eff_net_coal/eff_net_coal.elf
#
# Generate an aie2p ELF (tinyyolo) once:
#   TDIR=$XRT_ROOT/src/runtime_src/core/common/aiebu/test/cpp_test/aie2/tinyyolo_preempt
#   W=/tmp/xrt_elf_mock_tinyyolo && mkdir -p $W && cp $TDIR/config.json $W/
#   base64 -d $TDIR/ml_txn_lp.b64 > $W/ml_txn_lp.bin
#   base64 -d $TDIR/pdi.b64 > $W/00000000-0000-0000-0000-000000001111.pdi
#   $XRT_ROOT/build/Release/runtime_src/core/common/aiebu/test/cpp_test/cpp_api/aie2_full_elf_cpp \
#     tinyyolo_preempt $W
#   # -> $W/tinyyolo_preempt_in_mem.elf

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
XRT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
BUILD_DIR="${XRT_ROOT}/build/Release"
XILINX_XRT="${BUILD_DIR}/opt/xilinx/xrt"
TEST_BUILD="${SCRIPT_DIR}/../build/Release"
ELF_MOCK_BIN="${TEST_BUILD}/elf_mock/elf_mock"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--build] --elf <file.elf> [elf_mock options...]

Environment (auto-set unless already exported):
  XILINX_XRT       ${XILINX_XRT}
  LD_LIBRARY_PATH  .../opt/xilinx/xrt/lib (prepended)
  XRT_ELF_MOCK=1   when --with-device is passed

Examples:
  $(basename "$0") --build --elf \$XRT_ROOT/build/Release/runtime_src/core/common/aiebu/test/aie2ps-ctrlcode/sanity_cpp.elf
  $(basename "$0") --elf path/to/kernel.elf --with-device
EOF
}

DO_BUILD=0
PASS_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --build) DO_BUILD=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) PASS_ARGS+=("$1"); shift ;;
  esac
done

if [[ ${#PASS_ARGS[@]} -eq 0 ]] || [[ "${PASS_ARGS[0]}" != "--elf" ]]; then
  usage
  exit 1
fi

export XILINX_XRT
export LD_LIBRARY_PATH="${XILINX_XRT}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

if [[ "${DO_BUILD}" -eq 1 ]]; then
  echo "[run.sh] Building XRT (npu)..."
  "${XRT_ROOT}/build/build.sh" -npu -opt -j "$(nproc)" -disable-werror

  echo "[run.sh] Building elf_mock..."
  mkdir -p "${TEST_BUILD}"
  cmake -S "${SCRIPT_DIR}/.." -B "${TEST_BUILD}" -DCMAKE_BUILD_TYPE=Release -DXILINX_XRT="${XILINX_XRT}"
  cmake --build "${TEST_BUILD}" --target elf_mock
fi

if [[ ! -x "${ELF_MOCK_BIN}" ]]; then
  echo "elf_mock not found at ${ELF_MOCK_BIN}; run with --build first" >&2
  exit 1
fi

WITH_DEVICE=0
for arg in "${PASS_ARGS[@]}"; do
  [[ "${arg}" == "--with-device" ]] && WITH_DEVICE=1
done

if [[ "${WITH_DEVICE}" -eq 1 ]]; then
  export XRT_ELF_MOCK=1
  echo "[run.sh] XRT_ELF_MOCK=1 (using libxrt_xdna_mock.so)"
fi

exec "${ELF_MOCK_BIN}" "${PASS_ARGS[@]}"
