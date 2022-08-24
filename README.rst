
Xilinx Runtime for Windows
==========================

Xilinx Runtime (XRT) is implemented as as a combination of userspace and kernel
driver components.

Clone Code
-----------

``git clone --recursive https://gitenterprise.xilinx.com/XRT/XRT-IPU.git -b master``

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

* ``python xrtdeps-win19.py --boost minimal --icd --opencl --install_dir <XRT-IPU root>\ext --build_dir <XRT-IPU root>\ext_build``
* This builds and installs dependencies necessary for the XRT-IPU build (e.g. Boost, OpenCL)

3. **Run build batch file in XRT-IPU\\build**

* ``build_ipu19.bat -clean``
* ``build_ipu19.bat -release <XRT-IPU root>\ext``
* Build generates these files in ``build\WRelease``
   
  * XRT-IPU build artifacts in the ``xilinx\xrt`` folder (also zipped to ``XRT_202210.2.13.0_WindowsServer2019-amd64.zip``)
  * MSVC 2019 solution file ``XRT.sln``

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
It will display a list of directories that it has found within your --build_dir (this should be ``<XRT-IPU root>\\ext_build``)::

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
