# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
#
# Loopback mock shim for ELF / module / patching tests (no NPU hardware).
# Activated at runtime with: export XRT_ELF_MOCK=1

set(MOCK_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(COMMON_PCIE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../common")

add_library(xdna_mock_objects OBJECT
  xdna_mock.cxx
  ${COMMON_PCIE_SRC_DIR}/system_pcie.cpp
  ${COMMON_PCIE_SRC_DIR}/device_pcie.cpp
  )

target_include_directories(xdna_mock_objects
  PRIVATE
  ${XRT_SOURCE_DIR}/runtime_src
  ${MOCK_SRC_DIR}
  )

add_library(xrt_xdna_mock SHARED
  $<TARGET_OBJECTS:xdna_mock_objects>
  $<TARGET_OBJECTS:core_common_objects>
  )

set_target_properties(xrt_xdna_mock PROPERTIES
  VERSION ${XRT_VERSION_STRING}
  SOVERSION ${XRT_SOVERSION}
  )

target_link_libraries(xrt_xdna_mock
  PRIVATE
  xrt_coreutil
  rt
  uuid
  )

install(TARGETS xrt_xdna_mock
  EXPORT xrt-targets
  RUNTIME DESTINATION ${XRT_INSTALL_BIN_DIR} COMPONENT ${XRT_COMPONENT}
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_COMPONENT} NAMELINK_COMPONENT ${XRT_DEV_COMPONENT}
  ARCHIVE DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_DEV_COMPONENT}
  )
