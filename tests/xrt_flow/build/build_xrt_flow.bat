REM Copyright(C) 2022 Advanced Micro Devices, Inc. All rights reserved.

@echo OFF

REM Compile a separate test executable against each test case

set CONFIGURATION=%1

if "%1"=="" (
  echo "Usage: build_xrt_flow.bat <Debug or Release>"
  goto:eof
)

set XRT_FLOW_ROOT=%~dp0..
set EXE_PATH=%XRT_FLOW_ROOT%\\x64\\%CONFIGURATION%

set TEST_CASE=conv
call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

set TEST_CASE=elemw
call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

set TEST_CASE=pool
call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

set TEST_CASE=resnet50
call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

set TEST_CASE=multilayer
call :BUILD_TEST_CASE
if %ERRORLEVEL% neq 0 goto FAIL

exit /b 0

:BUILD_TEST_CASE
echo ------------------------
echo Build xrt_flow_%TEST_CASE%.exe
echo ------------------------
msbuild %XRT_FLOW_ROOT%\\xrt_flow.sln /p:Configuration="%CONFIGURATION%" /p:Platform=x64 /p:XRT_FLOW_TEST_CASE="%TEST_CASE%"
if %ERRORLEVEL% neq 0 goto BUILD_TEST_CASE_EXIT

REM Rename the xrt_flow executable to match with the test case
copy /y %EXE_PATH%\\xrt_flow.exe %EXE_PATH%\\xrt_flow_%TEST_CASE%.exe
if %ERRORLEVEL% neq 0 goto BUILD_TEST_CASE_EXIT
:BUILD_TEST_CASE_EXIT
exit /b %ERRORLEVEL%

:FAIL
echo Error when building or copying xrt_flow_%TEST_CASE%.exe
exit /b %ERRORLEVEL%
