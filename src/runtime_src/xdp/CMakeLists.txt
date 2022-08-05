# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
#

# For IPU, only support xdp_core, xdp_native_plugin, and xdp_user_plugin

add_compile_options("-DSKIP_IOCTL")
add_compile_options("-DSKIP_AIE_INI")


if (NOT WIN32)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${XRT_SOURCE_DIR}/include/1_2
  )

else()
# = WINDOWS ===================================================================

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${XRT_SOURCE_DIR}/include/1_2
  ${KHRONOS}/include
  )
endif()

# ====== File directory definitions for both Linux and Windows =========
set(XRT_XDP_PROFILE_DEVICE_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/profile/device")
set(XRT_XDP_PROFILE_DEVICE_MMAPPED_MON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/profile/device/mmapped_monitors")
set(XRT_XDP_PROFILE_DEVICE_IOCTL_MON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/profile/device/ioctl_monitors")
set(XRT_XDP_PROFILE_HAL_DEVICE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/profile/device/hal_device")
set(XRT_XDP_PROFILE_XRT_DEVICE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/profile/device/xrt_device")
set(XRT_XDP_PROFILE_NATIVE_PLUGIN_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/profile/plugin/native")
set(XRT_XDP_PROFILE_NATIVE_WRITER_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/profile/writer/native")
set(XRT_XDP_PROFILE_USER_PLUGIN_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/profile/plugin/user")
set(XRT_XDP_PROFILE_USER_WRITER_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/profile/writer/user")
set(XRT_XDP_PROFILE_BASE_PLUGIN_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/profile/plugin/vp_base")
set(XRT_XDP_PROFILE_DATABASE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/profile/database")
set(XRT_XDP_PROFILE_EVENTS_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/profile/database/events")
set(XRT_XDP_PROFILE_STATIC_INFO_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/profile/database/static_info")
set(XRT_XDP_PROFILE_EVENTS_CREATOR_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/profile/database/events/creator")
set(XRT_XDP_PROFILE_VP_WRITER_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/profile/writer/vp_base")

file(GLOB XRT_NEW_XDP_CORE_FILES
  "${XRT_XDP_PROFILE_BASE_PLUGIN_DIR}/*.h"
  "${XRT_XDP_PROFILE_BASE_PLUGIN_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_DATABASE_DIR}/*.h"
  "${XRT_XDP_PROFILE_DATABASE_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_DEVICE_DIR}/*.h"
  "${XRT_XDP_PROFILE_DEVICE_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_DEVICE_MMAPPED_MON_DIR}/*.h"
  "${XRT_XDP_PROFILE_DEVICE_MMAPPED_MON_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_DEVICE_IOCTL_MON_DIR}/*.h"
  "${XRT_XDP_PROFILE_DEVICE_IOCTL_MON_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_EVENTS_DIR}/*.h"
  "${XRT_XDP_PROFILE_EVENTS_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_STATIC_INFO_DIR}/*.h"
  "${XRT_XDP_PROFILE_STATIC_INFO_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_EVENTS_CREATOR_DIR}/*.h"
  "${XRT_XDP_PROFILE_EVENTS_CREATOR_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_VP_WRITER_DIR}/*.h"
  "${XRT_XDP_PROFILE_VP_WRITER_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_XRT_DEVICE_DIR}/*.h"
  "${XRT_XDP_PROFILE_XRT_DEVICE_DIR}/*.cpp"
  )

file(GLOB XRT_XDP_PROFILE_NATIVE_PLUGIN_FILES
  "${XRT_XDP_PROFILE_NATIVE_PLUGIN_DIR}/*.h"
  "${XRT_XDP_PROFILE_NATIVE_PLUGIN_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_NATIVE_WRITER_DIR}/*.h"
  "${XRT_XDP_PROFILE_NATIVE_WRITER_DIR}/*.cpp"
  )

file(GLOB XRT_XDP_PROFILE_USER_PLUGIN_FILES
  "${XRT_XDP_PROFILE_USER_PLUGIN_DIR}/*.h"
  "${XRT_XDP_PROFILE_USER_PLUGIN_DIR}/*.cpp"
  "${XRT_XDP_PROFILE_USER_WRITER_DIR}/*.h"
  "${XRT_XDP_PROFILE_USER_WRITER_DIR}/*.cpp"
  )

# ========== XDP Core Library For Both Linux and Windows =============

add_library(xdp_core SHARED ${XRT_NEW_XDP_CORE_FILES})
add_dependencies(xdp_core xrt_coreutil)
target_link_libraries(xdp_core PRIVATE xrt_coreutil)

set_target_properties(xdp_core PROPERTIES VERSION ${XRT_VERSION_STRING} SOVERSION ${XRT_SOVERSION})

install (TARGETS xdp_core
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR} ${XRT_NAMELINK_SKIP}
  RUNTIME DESTINATION ${XRT_INSTALL_BIN_DIR}
)

install (TARGETS xdp_core
  ARCHIVE DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_DEV_COMPONENT}
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR} COMPONENT ${XRT_DEV_COMPONENT} ${XRT_NAMELINK_ONLY}
)

# ============ XDP Plugin Modules For Both Linux and Windows ===========

add_library(xdp_native_plugin MODULE ${XRT_XDP_PROFILE_NATIVE_PLUGIN_FILES})
add_library(xdp_user_plugin MODULE ${XRT_XDP_PROFILE_USER_PLUGIN_FILES})

add_dependencies(xdp_native_plugin xdp_core xrt_coreutil)
add_dependencies(xdp_user_plugin xdp_core xrt_coreutil)

target_link_libraries(xdp_native_plugin PRIVATE xdp_core xrt_coreutil)
target_link_libraries(xdp_user_plugin PRIVATE xdp_core xrt_coreutil)

set_target_properties(xdp_native_plugin PROPERTIES VERSION ${XRT_VERSION_STRING} SOVERSION ${XRT_SOVERSION})
set_target_properties(xdp_user_plugin PROPERTIES VERSION ${XRT_VERSION_STRING} SOVERSION ${XRT_SOVERSION})

install (TARGETS xdp_native_plugin
                 xdp_user_plugin
  LIBRARY DESTINATION ${XRT_INSTALL_LIB_DIR}/xrt/module
)
