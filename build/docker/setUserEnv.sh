#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All Rights Reserved.

# This Scripts sets the user env variables to run the gradle tasks in XRT IPU docker container
# source setUserEnv.sh

export XDOCK_USER=${USER}
export XDOCK_UID=$(id -u ${USER})
export XDOCK_GROUP=${GROUP}
export XDOCK_GID=$(id -g ${USER})
