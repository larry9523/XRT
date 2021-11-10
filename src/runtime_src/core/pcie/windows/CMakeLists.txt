# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2021 Xilinx, Inc. All rights reserved.
#

if (${XRT_IPU_BUILD} STREQUAL "yes")
	add_subdirectory(ipu)
else()
	add_subdirectory(alveo)
endif()
