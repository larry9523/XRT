REM Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.

@echo ON

if "%1"=="" goto USAGE
if "%2"=="" goto USAGE

SET XRT_FLOW_TEST=%1
SET XCL_BIN=%2

rename %XRT_FLOW_TEST% workspace
set XILINX_XRT=%~dp0
xrt_flow_%XRT_FLOW_TEST%.exe %~dp0\%XCL_BIN%
rename workspace %XRT_FLOW_TEST%
exit /b 0

:USAGE
echo Usage: run_xrt_flow.bat <test_case> <local_path_to_xclbin>
exit /b 0
