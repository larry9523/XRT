# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.

if(XRT_IPU_BUILD)
  add_subdirectory(ipu)
elseif(MCDM)
  add_subdirectory(mcdm)
else()
  add_subdirectory(alveo)
endif()
