REM Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.

@echo ON

if "%1"=="" (
  SET XRT_FLOW_TEST=conv
) else (
  SET XRT_FLOW_TEST=%1
)

rename %XRT_FLOW_TEST% workspace
set XILINX_XRT=%~dp0
xrt_flow_%XRT_FLOW_TEST%.exe %~dp0\4cmt_oo.xclbin
