# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2021 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_subdirectory(include)
add_subdirectory(common)

if ( (NOT DEFINED MCDM) AND WIN32)
  add_subdirectory(tools)
endif()


if (${XRT_NATIVE_BUILD} STREQUAL "yes")
  add_subdirectory(pcie)
else()
  add_subdirectory(common)
  add_subdirectory(edge)
endif()

if (${XRT_CLANGTIDY_REVIEW})
  add_subdirectory(edge)
endif()
