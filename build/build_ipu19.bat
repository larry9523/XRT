@ECHO OFF

IF "%1"=="" (
  GOTO Help
)

IF DEFINED MSVC_PARALLEL_JOBS ( SET LOCAL_MSVC_PARALLEL_JOBS=%MSVC_PARALLEL_JOBS%) ELSE ( SET LOCAL_MSVC_PARALLEL_JOBS=3 )

IF "%1" == "clean" (
  GOTO Clean
)

IF "%1" == "-clean" (
  GOTO Clean
)

IF "%1" == "-help" (
  GOTO Help
)

REM Default location of XRT-IPU dependencies built by xrtdeps-win19.py
SET EXT_DIR=c:/Xilinx/XRT/ext

IF "%1" == "-debug" (
  IF NOT "%2" == "" (
      SET EXT_DIR=%2
  )
  echo EXT_DIR = %EXT_DIR%
  GOTO DebugBuild
)

IF "%1" == "-release" (
  IF "%2" == "" (
    SET EXT_DIR=c:/Xilinx/XRT/ext
  ) ELSE (
    SET EXT_DIR=%2
  )
  IF "%2" == "-package" (
    SET EXT_DIR=c:/Xilinx/XRT/ext
  ) ELSE (
    SET EXT_DIR=%2
  )
  echo EXT_DIR = %EXT_DIR%
  GOTO ReleaseBuild
)


IF "%1" == "-all" (
  IF NOT "%2" == "" (
      SET EXT_DIR=%2
  )
  CALL:DebugBuild
  IF errorlevel 1 (exit /B %errorlevel%)

  IF NOT "%2" == "" (
      SET EXT_DIR=%2
  )
  CALL:ReleaseBuild
  IF errorlevel 1 (exit /B %errorlevel%)

  goto:EOF
)

ECHO Unknown option: %1
GOTO Help


REM --------------------------------------------------------------------------
:Help
ECHO.
ECHO Usage: build.bat [options]
ECHO.
ECHO [-help]                        - List this help
ECHO [-clean^|clean]                - Remove build directories
ECHO [-debug] [boost-install-dir]   - Creates a debug build
ECHO [-release] [boost-install-dir] - Creates a release build
ECHO.
ECHO Additional options to be used afer with the '-release' option:
ECHO   [-package]               - Packages the release build to a MSI archive.
ECHO                              Note: Depends on the WIX application.
ECHO example: build_ipu.bat -release C:\Xilinx\XRT\ext

GOTO:EOF

REM --------------------------------------------------------------------------
:Clean
IF EXIST WDebug (
  ECHO Removing 'WDebug' directory...
  rmdir /S /Q WDebug
)
IF EXIST WRelease (
  ECHO Removing 'WRelease' directory...
  rmdir /S /Q WRelease
)
GOTO:EOF


REM --------------------------------------------------------------------------
:DebugBuild
echo ====================== Windows Debug Build ============================
MKDIR WDebug
PUSHD WDebug

ECHO MSVC Compile Parallel Jobs: %LOCAL_MSVC_PARALLEL_JOBS%

cmake -G "Visual Studio 16 2019" -DMSVC_PARALLEL_JOBS=%LOCAL_MSVC_PARALLEL_JOBS% -DKHRONOS=%EXT_DIR% -DBOOST_ROOT=%EXT_DIR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXRT_IPU_BUILD=yes ../../src
IF errorlevel 1 (POPD & exit /B %errorlevel%)

cmake --build . --verbose --config Debug
IF errorlevel 1 (POPD & exit /B %errorlevel%)

cmake --build . --verbose --config Debug --target install
IF errorlevel 1 (POPD & exit /B %errorlevel%)

POPD
GOTO:EOF

REM --------------------------------------------------------------------------
:ReleaseBuild
ECHO ====================== Windows Release Build ============================
MKDIR WRelease
PUSHD WRelease

ECHO MSVC Compile Parallel Jobs: %LOCAL_MSVC_PARALLEL_JOBS%

SET CREATE_PACKAGE=false

REM Evaluate the additional options
REM Warning: Do not put any echo statements in the "IF" blocks.  Doing so
REM          will result in expansion issues. e.g. the variable will not be set
SHIFT
:shift_loop_release
IF "%1" == "-package" (
  SET CREATE_PACKAGE=true
  SHIFT
  GOTO:shift_loop_release
)

cmake -G "Visual Studio 16 2019"  -DXCL_MGMT=%XCLMGMT_DRIVER% -DXOCL_USER=%XOCLUSER_DRIVER% -DXCL_MGMT2=%XCLMGMT2_DRIVER% -DXOCL_USER2=%XOCLUSER2_DRIVER% -DMSVC_PARALLEL_JOBS=%LOCAL_MSVC_PARALLEL_JOBS% -DKHRONOS=%EXT_DIR% -DBOOST_ROOT=%EXT_DIR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXRT_IPU_BUILD=yes ../../src
IF errorlevel 1 (POPD & exit /B %errorlevel%)

cmake --build . --verbose --config Release
IF errorlevel 1 (POPD & exit /B %errorlevel%)

cmake --build . --verbose --config Release --target install
IF errorlevel 1 (POPD & exit /B %errorlevel%)

ECHO ====================== Zipping up Installation Build ============================
cpack -G ZIP -C Release

IF "%CREATE_PACKAGE%" == "true" (
  ECHO ====================== Creating MSI Archive ============================
  cpack -G WIX -C Release
)

popd
GOTO:EOF
