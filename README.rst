
Xilinx Runtime for Windows
==========================

Xilinx Runtime (XRT) is implemented as as a combination of userspace and kernel
driver components.

Clone Code
-----------

``git clone --recursive git@github.amd.com:ATG-Xilinx/XRT-IPU.git -b atg-dev``

Build XRT-IPU on Windows
------------------------

1. **Open Developer Command Prompt for VS 2019**

* Open an elevated command prompt and run ``"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"``

  * These steps have only been validated with a regular command prompt, so it is not recommended to use other terminal environments (e.g. Git Bash, PowerShell, etc).

* Alternatively, search for "Developer Command Prompt for VS 2019" in Windows and run it
* **IMPORTANT:** You must use this command prompt for the rest of the XRT-IPU build commands in this section

  * vcvars64.bat sets up tools, environment variables, and registry settings that these build steps depend upon

Note: not sure since when, IT blocks the registry access, which Visual Studio developer's command prompt requires. 
This could cause some strange compilation error for boost, e.g., cannot find the headers actually in SDK that installed already. 
Please check if you have registry access permission by regedit command. 

2. **Run XRT-IPU\\src\\runtime_src\\tools\\scripts\\xrtdeps-win19.py (Requires Python3)**

* ``python xrtdeps-win19.py --boost minimal --icd --opencl``
* This builds and installs dependencies necessary for the XRT-IPU build (e.g. Boost, OpenCL)

3. **Run build batch file in XRT-IPU\\build**

* ``build_ipu19.bat -clean``
* ``build_ipu19.bat -release C:\Xilinx\XRT\ext``
* Build generates these files in ``build\WRelease``
   
  * XRT-IPU build artifacts in the ``xilinx\xrt`` folder (also zipped to ``XRT_202210.2.13.0_WindowsServer2019-amd64.zip``)
  * MSVC 2019 solution file ``XRT.sln``

Build and Run Test Application
------------------------------

1. **Build xrt_flow**

* In ``tests\xrt_flow\build``, run ``build_xrt_flow.bat Release``
* This generates a test folder ``xrt_flow_test`` within ``tests\xrt_flow\x64\Release``

2. **Prepare Simnow Image**

* Copy over ``xrt_flow_test`` to a location on your Simnow VMDK image (e.g. use VirtualBox, OSFMount, etc).

3. **Run xrt_flow on Simnow**

In your SimNow session, open an elevated command prompt and run the following commands: 

* ``cd xrt_flow_test`` 
* ``run_xrt_flow.bat <test case name (e.g. conv)>``

Troubleshooting
---------------

* xrtdeps-win19.py: Issues with building Boost

``Failed to build Boost.Build engine. Please consult bootstrap.log for further diagnostics.``

``fatal error C1083: Cannot open include file: 'ctype.h': No such file or directory``

Double check the following:

* You have registry access
* You are using the latest version of Visual Studio 2019
* You are running inside a Developer Command Prompt for Visual Studio 2019

Note that it should not be necessary at any point to manually copy, build, or install Boost libraries.

* xrtdeps-win19.py: ``ERROR: Existing build present for the given library to be installed.``

xrtdeps-win19.py has a limitation where it requires a clean build directory before it installs the Boost and OpenCL libraries. 
It will display a list of directories that it has found within C:\Xilinx\XRT::

    boost-1.75.0 build directory .................... [found]
    Khronos OpenCL headers build directory .......... [not found]
    Khronos OpenCL ICD build directory .............. [not found]

Delete the existing directories and try again.

* xrtdeps-win19.py: ``CMake Error at C:/Program Files/CMake/share/cmake-3.14/Modules/FindBoost.cmake``

There have been reports of local installations of CMake interfering with the build, as the scripts expect to use the CMake installation packaged with Visual Studio.
If you have a local installation of CMake, try removing this from your PATH or other environment variables.

* xrtdeps-win19.py: ``Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR GTEST_MAIN_LIBRARY)``

This is expected and can be safely ignored.

* build_ipu19.bat: ``Syntax error in cmake code when parsing string, invalid escape sequence \U``

This error is complaining about the use of back slashes used in Windows paths, but this does not affect the build and can be safely ignored.
Alternatively, you can use forward slashes when providing the XRT-IPU install directory path.

* build_ipu19.bat: ``BOOST and KHRONOS environment variables are empty``

Some developers have observed these variables being empty at the first run of build_ipu19.bat.
If you encounter this and later run into a build issue, try running build_ipu19.bat -clean and build_ipu19.bat -release again.

* xrt_flow.exe: Various compilation errors, unable to find XRT header includes

Double check that the configuration of your XRT-IPU build (default: x64 Release) matches with the configuration of your xrt_flow build.

* xrt_flow.exe: ``MSVCP140.dll Is Missing`` or other C++ exception pop-ups complaining about missing DLLs

xrt_flow requires C++ runtime libraries which are missing on your Simnow image. Installation steps depend on whether you have Release or Debug built.

* Release - Run vc_redist.x64.exe on your Simnow image: https://docs.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170
* Debug - Copy the debug-version DLLs over to the same folder that contains xrt_flow.exe. You should be able to find these on your build machine at these paths:
  
  * C:\\Windows\\System32\\ucrtbased.dll
  * C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\VC\\Redist\\MSVC\\14.29.30133\\debug_nonredist\\x64\\Microsoft.VC142.DebugCRT

Note that exact version numbers will depend on your Visual Studio installation.

* xrt_flow.exe: ``[XRT] ERROR: DeviceIoControl IOCTL_KIPUDRV_EXECPOLL failed with error 259`` and ``XrtKdsClientPoll failed!``

These error messages may show up in the xrt_flow command prompt and WinDbg log respectively. 
XRT polls for the status of the running command every second and may timeout due to Simnow slowness. 
These errors are usually benign. Instead, watch for either "Test failed with <#> mismatches" or "Test passed" in the xrt_flow command prompt.
