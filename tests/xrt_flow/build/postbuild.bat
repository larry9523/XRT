@ECHO OFF

REM Copy over xrt_flow.exe and its dependencies to export folder
REM that can be dropped and run standalone in simnow image

SET CONFIGURATION=%1
SET TEST_CASE=%2
SET DPU_TARGET=%3

SET XRT_FLOW_ROOT=%~dp0..
SET XRT_IPU_ROOT=%XRT_FLOW_ROOT%\..\..
SET EXPORT=%XRT_FLOW_ROOT%\x64\%CONFIGURATION%\xrt_flow_test
SET DPU=%XRT_IPU_ROOT%\DPU\host_code\xrt_host_oo

REM Create export folder
MKDIR %EXPORT%
MKDIR %EXPORT%\bin

REM Copy over xrt_flow.exe
COPY /y %XRT_FLOW_ROOT%\x64\%CONFIGURATION%\xrt_flow.exe %EXPORT%\xrt_flow.exe

REM Copy over XRT-IPU DLLs
COPY /y %XRT_IPU_ROOT%\build\W%CONFIGURATION%\xilinx\xrt\bin\xrt_core.dll %EXPORT%\xrt_core.dll
COPY /y %XRT_IPU_ROOT%\build\W%CONFIGURATION%\xilinx\xrt\bin\xrt_coreutil.dll %EXPORT%\xrt_coreutil.dll
COPY /y %XRT_IPU_ROOT%\build\W%CONFIGURATION%\xilinx\xrt\bin\xrt_core.dll %EXPORT%\bin\xrt_core.dll
COPY /y %XRT_IPU_ROOT%\build\W%CONFIGURATION%\xilinx\xrt\bin\xrt_coreutil.dll %EXPORT%\bin\xrt_coreutil.dll

REM Copy over test case data folders
XCOPY /e /i /y %DPU%\%DPU_TARGET% %EXPORT%\

REM Copy over other files (XCL binary, xrt.ini, helper batch script)
COPY /y %DPU%\..\1x3_oo.xclbin %EXPORT%
COPY /y %XRT_FLOW_ROOT%\target\xrt.ini %EXPORT%
COPY /y %XRT_FLOW_ROOT%\target\run_xrt_flow.bat %EXPORT%