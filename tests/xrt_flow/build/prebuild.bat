@ECHO OFF

REM This assumes that xrtdeps-win19.py was run once using <XRT-IPU root>\ext as the install folder

REM build_ipu19.bat uses cmake, which requires the Visual Studio Developer Command Prompt

SET VC_VS_SOURCEPATH=%1
SET CONFIGURATION=%2

SET XRT_FLOW_ROOT=%~dp0..
SET XRT_IPU_ROOT=%XRT_FLOW_ROOT%\..\..
SET BUILDPATH=%XRT_IPU_ROOT%\build\W%CONFIGURATION%

REM Compile XRT-IPU if there is no existing build
IF EXIST %BUILDPATH% (
  ECHO XRT-IPU build already exists at: %BUILDPATH%
  GOTO:EOF
)
ECHO Build %CONFIGURATION% XRT-IPU

REM Strip quotes and trailing semicolon
SET VC_VS_SOURCEPATH_STRIP=%VC_VS_SOURCEPATH:~1,-2%

REM Set up vcvars environment and run build_ipu19.bat
SET VCVAR_SCRIPTPATH=%VC_VS_SOURCEPATH_STRIP%\..\..\Build\vcvars64.bat
call "%VCVAR_SCRIPTPATH%"
call "%XRT_IPU_ROOT%\build\build_ipu19.bat" -%CONFIGURATION% %XRT_IPU_ROOT%\ext
