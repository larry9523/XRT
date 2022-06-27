REM Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.

@echo ON

if "%1"=="" goto USAGE

SET XRT_FLOW_TEST=%1

rename %XRT_FLOW_TEST% workspace
set XILINX_XRT=%~dp0
xrt_flow.exe %~dp0\1x3_oo.xclbin
rename workspace %XRT_FLOW_TEST%
exit /b 0

:USAGE
echo Usage: run_xrt_flow.bat <test_case>
exit /b 0

