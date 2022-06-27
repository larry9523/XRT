REM Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.

@echo OFF

REM Build xrt_flow.exe
REM With the unified config.h, there is no need to compile a separate executable per test case

set CONFIGURATION=%1

if "%1"=="" (
  echo "Usage: build_xrt_flow.bat <Debug or Release>"
  goto:eof
)

set XRT_FLOW_ROOT=%~dp0..
set EXE_PATH=%XRT_FLOW_ROOT%\\x64\\%CONFIGURATION%

REM See DPU\host_code\xrt_host_oo\1x4
set DPU_TARGET=1x4

call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

exit /b 0

:BUILD_TEST_CASE
echo ------------------------
echo Build xrt_flow.exe
echo ------------------------
msbuild %XRT_FLOW_ROOT%\\xrt_flow.sln /p:Configuration="%CONFIGURATION%" /p:Platform=x64 /p:DPU_TARGET="%DPU_TARGET%"
if %ERRORLEVEL% neq 0 goto BUILD_TEST_CASE_EXIT
:BUILD_TEST_CASE_EXIT
exit /b %ERRORLEVEL%

:FAIL
echo Error when building or copying xrt_flow.exe
exit /b %ERRORLEVEL%

