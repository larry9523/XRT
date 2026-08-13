# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.

cmake_minimum_required(VERSION 3.16.0)
project(elf_mock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED XRT_SOURCE_DIR)
  if (DEFINED ENV{XRT_SOURCE_DIR})
    set(XRT_SOURCE_DIR $ENV{XRT_SOURCE_DIR})
  else()
    get_filename_component(XRT_SOURCE_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
  endif()
endif()

find_package(XRT REQUIRED HINTS $ENV{XILINX_XRT}/share/cmake/XRT)

add_executable(elf_mock main.cpp)

target_include_directories(elf_mock PRIVATE
  ${XRT_INCLUDE_DIRS}
  ${XRT_SOURCE_DIR}/runtime_src
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(elf_mock PRIVATE XRT::xrt_coreutil)

install(TARGETS elf_mock RUNTIME DESTINATION bin OPTIONAL)
