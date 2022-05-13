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

IF "%1" == "-debug" (
  SET USE_DEFAULT_DEPS=false
  IF "%2" == "" (SET USE_DEFAULT_DEPS=true)
  IF %USE_DEFAULT_DEPS% == true (
    SET BOOST_DEBUG=c:/Xilinx/XRT/ext
    SET KHRONOS_DEBUG=c:/Xilinx/XRT/ext
  ) ELSE (
      SET BOOST_DEBUG=%2
	  SET KHRONOS_DEBUG=%2
  )
  echo BOOST_DEBUG = %BOOST_DEBUG%
  echo KHRONOS_DEBUG = %KHRONOS_DEBUG%
  GOTO DebugBuild
)

IF "%1" == "-release" (
  SET USE_DEFAULT_DEPS=false
  IF "%2" == "" (SET USE_DEFAULT_DEPS=true)
  IF "%2" == "-package" (SET USE_DEFAULT_DEPS=true)
  IF %USE_DEFAULT_DEPS% == true (
    SET BOOST=c:/Xilinx/XRT/ext
    SET KHRONOS=c:/Xilinx/XRT/ext
  ) ELSE (
      SET BOOST=%2
	  SET KHRONOS=%2
  )
  echo BOOST = %BOOST%
  echo KHRONOS = %KHRONOS%
  GOTO ReleaseBuild
)


IF "%1" == "-all" (
  IF "%2" == "" (
    SET BOOST_DEBUG=c:/Xilinx/XRT/ext
    SET KHRONOS_DEBUG=c:/Xilinx/XRT/ext
  ) ELSE (
      SET BOOST_DEBUG=%2
	  SET KHRONOS_DEBUG=%2
  )
  CALL:DebugBuild
  IF errorlevel 1 (exit /B %errorlevel%)

  IF "%2" == "" (
    SET BOOST=c:/Xilinx/XRT/ext
    SET KHRONOS=c:/Xilinx/XRT/ext
  ) ELSE (
      SET BOOST=%2
	  SET KHRONOS=%2
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

cmake -G "Visual Studio 16 2019" -DMSVC_PARALLEL_JOBS=%LOCAL_MSVC_PARALLEL_JOBS% -DKHRONOS=%KHRONOS% -DBOOST_ROOT=%BOOST% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXRT_IPU_BUILD=yes ../../src
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

cmake -G "Visual Studio 16 2019"  -DXCL_MGMT=%XCLMGMT_DRIVER% -DXOCL_USER=%XOCLUSER_DRIVER% -DXCL_MGMT2=%XCLMGMT2_DRIVER% -DXOCL_USER2=%XOCLUSER2_DRIVER% -DMSVC_PARALLEL_JOBS=%LOCAL_MSVC_PARALLEL_JOBS% -DKHRONOS=%KHRONOS% -DBOOST_ROOT=%BOOST% -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DXRT_IPU_BUILD=yes ../../src
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
