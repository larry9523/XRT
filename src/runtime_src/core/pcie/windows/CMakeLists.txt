# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
if (XRT_IPU_BUILD)
  add_subdirectory(ipu)
elseif (MCDM_IN_XRT)
  add_subdirectory(mcdm)
elseif (NOT MCDM)
  add_subdirectory(alveo)
endif()
