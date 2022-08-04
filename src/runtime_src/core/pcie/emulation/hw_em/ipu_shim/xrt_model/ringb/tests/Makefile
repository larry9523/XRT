# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

ifeq ($(OS),Windows_NT)
	ifeq ($(shell uname -s),) # not in a bash-like shell
		CLEANUP = del /F /Q
		MKDIR = mkdir
	else # in a bash-like shell, like msys
		CLEANUP = rm -f
		MKDIR = mkdir -p
	endif
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=out
endif

ECHO=@echo -e

TLD=$(abspath ../..)
RINGB_DIR=$(TLD)/ringb
XRT_DIR=$(abspath $(TLD)/xrt)
$(info XRT_DIR : $(XRT_DIR))
IPU_DIR=$(abspath $(TLD)/../..)
$(info IPU_DIR : $(IPU_DIR))
UNITY_DIR=$(abspath $(IPU_DIR)/../Unity)
$(info UNITY_DIR : $(UNITY_DIR))

PATHU = $(UNITY_DIR)/src
PATHS = $(RINGB_DIR)
PATHT = $(RINGB_DIR)/tests
PATHB = $(RINGB_DIR)/build
PATHD = $(RINGB_DIR)/build/depends
PATHO = $(RINGB_DIR)/build/objs
PATHR = $(RINGB_DIR)/build/results

BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

SRCT = $(wildcard $(PATHT)/*.c)
SRCU = $(wildcard $(PATHU)/*.c)

SRCS = \
	$(PATHS)/ring_buffer.c

INC = \
	. \
	$(PATHU) \
	$(PATHS) \
	$(RINGB_DIR)/include \
	$(XRT_DIR) \
	$(IPU_DIR)/api

DEFS = \
	TEST

COMPILE=gcc -c -g -O0
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS = \
	-fPIE \
	$(INC:%=-I%) \
	$(DEFS:%=-D%)

RESULTS = $(patsubst $(PATHT)/Test%.c,$(PATHR)/Test%.txt,$(SRCT) )
OUTS = $(patsubst $(PATHT)/Test%.c,$(PATHB)/Test%.$(TARGET_EXTENSION),$(SRCT) )
OBJS = $(patsubst $(PATHT)/Test%.c,$(PATHO)/Test%.o,$(SRCT) ) $(patsubst $(PATHS)/%.c,$(PATHO)/%.o,$(SRCS) )

PASSED = `grep -s PASS $(PATHR)/*.txt`
FAIL = `grep -s FAIL $(PATHR)/*.txt`
IGNORE = `grep -s IGNORE $(PATHR)/*.txt`

RFILES = $(wildcard $(PATHR)*.txt)

.PHONY: test
test: $(BUILD_PATHS) $(RESULTS)
	$(ECHO) "-----------------------\nIGNORES:\n-----------------------"
	$(ECHO) "$(IGNORE)"
	$(ECHO) "-----------------------\nFAILURES:\n-----------------------"
	$(ECHO) "$(FAIL)"
	$(ECHO) "\nDONE"

$(PATHR)/%.txt: $(PATHB)/%.$(TARGET_EXTENSION)
	$(shell $< > $@ 2>&1)

$(PATHB)/Test%.$(TARGET_EXTENSION): $(OBJS) $(PATHO)/unity.o #$(PATHD)/Test%.d
	$(LINK) -o $@ $^

$(PATHO)/%.o:: $(PATHT)/%.c
	$(info "Compile PATHT")
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)/%.o:: $(PATHS)/%.c
	$(info "Compile PATHS")
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)/%.o:: $(PATHU)/%.c $(PATHU)/%.h
	$(info "Compile PATHU")
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHD)/%.d:: $(PATHT)/%.c
	$(DEPEND) $@ $<

$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

.PHONY: clean
clean:
	$(CLEANUP) $(PATHO)/*.o
	$(CLEANUP) $(PATHB)/*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)/*.txt

.PHONY: distclean
distclean:
	$(RM) -r $(PATHD)
	$(RM) -r $(PATHO)
	$(RM) -r $(PATHR)
	$(RM) -r $(PATHB)

.PRECIOUS: $(PATHB)/Test%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)/%.d
.PRECIOUS: $(PATHO)/%.o
.PRECIOUS: $(PATHR)/%.txt

list:
	@grep '^[^#[:space:]].*:' Makefile
