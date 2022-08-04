/*
* SPDX-License-Identifier: Apache-2.0
* Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
*/



SimNow "hostenv" support files
==============================

These files provide the necessary plubming to allow users the ability to run FSDL(or any C code) within a Simnow model container.
Using a SimNow model container gives the C code ability to inject different stimulus into the SimNow fabric, much like a 
BFM(Bus Functional Model) or a standalone execution engine.  These BFM-like models are composed of a few components:

* User-written C code (FSDL, trimmed down OS driver, etc) compiled into a dll/.so
* SimNow "hostenv" support files
* SimNow Environment and the SimNow "stream" device model.  The "stream" device model provides the host for the
  user-written dll.

Compiling the user-written C code into a dll/.so
================================================

The interface from the user's C code into SimNow is a simple set of pure "C" functions provided by the "hostenv" support
files.  The user must compile thier code using these support files.  Example below:

set HOST_ENV_DIR=/home/jpflores/hostenv
g++ -std=c++11 -fPIC -shared -I $HOST_ENV_DIR -o libxrt.dll xrt_sample.c $HOST_ENV_DIR/host_env.cpp

host_env.cpp MUST be compiled into dll/.so.  The "C" functions are declared in host_env.h. The user's C code can include
this file explicitly, or it can "extern" define the necessary interface routines:

extern "C" unsigned int RD_SMN_ADDR(IN unsigned long long SmnAddress);
extern "C" void WR_SMN_ADDR(IN unsigned long long SmnAddress, IN unsigned int Value);
extern "C" void WAIT_NS(IN unsigned int TimeToDelayInNanoseconds);
extern "C" void RD_SYSHUB(IN unsigned long long SysHubAddress, OUT void *pData, unsigned num_bytes);
extern "C" void WR_SYSHUB(IN unsigned long long SysHubAddress, IN const void *pData, unsigned num_bytes);

Loading the dll/.so into SimNow
===============================

The dll/.so is loaded into SimNow using the "stream" device model surrogate.  You can load the dll/.so into SimNow using the following 
SimNow command (using the SimNow command console):

stream.LoadFsdl --lib-path $DEVELDIR/broadsword/devices/stream/xrt/libxrt.dll

When the Simnow simulation runs, the loaded dll/.so will execute as a standalone execution engine and inject stimulus into the Simnow
fabric, interacting with other platform model/firmware components.


