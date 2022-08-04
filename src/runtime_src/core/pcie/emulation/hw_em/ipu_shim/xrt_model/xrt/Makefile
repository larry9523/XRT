# SPDX-License-Identifier: Apache-2.0
# Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

EXT = dll
TARGET = libxrt

CXX = g++
CC = g++

XRT_DIR=$(abspath .)
$(info XRT_DIR : $(XRT_DIR))
HOST_ENV_DIR=$(abspath $(XRT_DIR)/../hostenv)
$(info HOST_ENV_DIR : $(HOST_ENV_DIR))
IPU_DIR=$(abspath $(XRT_DIR)/../../..)
$(info IPU_DIR : $(IPU_DIR))
RINGB_DIR=$(abspath $(XRT_DIR)/../ringb)
$(info RINGB_DIR : $(RINGB_DIR))
SOC_HEADERS_DIR=$(abspath $(IPU_DIR)/../soc_headers)
$(info SOC_HEADERS_DIR : $(SOC_HEADERS_DIR))

# Api header hash checks
API_H_FILES += ../../../../ipu/api/ipu_msg.h ../../../../ipu/api/mgmt_msg.h
API_HASH=$(shell cat $(API_H_FILES) | md5sum | cut -d ' ' -f 1)
API_HASH_HIGH=0x$(shell echo $(API_HASH) | cut -b1-16)
API_HASH_LOW=0x$(shell echo $(API_HASH) | cut -b17-32)
CFLAGS += -DAPI_HASH_HIGH=$(API_HASH_HIGH) -DAPI_HASH_LOW=$(API_HASH_LOW)

# Add directories that hold unit (or other) tests - they should build and test with a simple 'make' command in that
# directory

### BOZO: TODO: FIXME: Can't include these tests because when the Unity test framework is not available, such as the CI
### build, the entire build will fail.
#TESTS = \
#	$(RINGB_DIR)/tests

# Add any include path here
INCLUDE = \
	$(XRT_DIR) \
	$(IPU_DIR)/api \
	$(IPU_DIR)/include \
	$(SOC_HEADERS_DIR) \
	$(HOST_ENV_DIR) \
	$(RINGB_DIR)/include

# Add CPP sources only here
CXXSRCS = \
	xrt_startup.cpp \
	$(HOST_ENV_DIR)/host_env.cpp \
	$(RINGB_DIR)/ipuhenvring.cpp \
	$(RINGB_DIR)/rbuffer.cpp \
	$(RINGB_DIR)/rxbuffer.cpp \
	$(RINGB_DIR)/txbuffer.cpp

# Add C sources only here
CSRCS = \

# Picks up all C and CPP sources and adds them to the objects to build/clean
OBJS = \
	$(CXXSRCS:.cpp=.o) \
	$(CSRCS:.c=.o)

# For debugging - print all the defined objects
$(info OBJS: $(OBJS))

# C only flags
CFLAGS += \
	-fPIC \
	$(INCLUDE:%=-I%)

# CPP flags add CPP specific flags to the "default" CFLAGS
CXXFLAGS = \
	-std=c++11 \
	$(CFLAGS)

# Linker flags
LDFLAGS = \
	-shared


# The rest are just the targets and rules to build them
.PHONY: all
all: $(TARGET).$(EXT)

$(TARGET).$(EXT) : $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET).$(EXT)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

