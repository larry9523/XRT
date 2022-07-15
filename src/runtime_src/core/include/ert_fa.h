/*
 *  SPDX-License-Identifier: Apache-2.0
 *  Copyright (C) 2020, Xilinx Inc
 *  Copyright (C) 2022, Advanced Micro Devices, Inc.  All rights reserved.
 */
#ifndef _ERT_FA_H_
#define _ERT_FA_H_

#if defined(__KERNEL__)
# include <linux/types.h>
#else
# include <stdint.h>
#endif

#ifdef _WIN32
# pragma warning( push )
# pragma warning( disable : 4201 4200 )
#endif

/**
 * ERT fast adapter error type
 *
 * @ERT_FA_DESC_FIFO_OVERRUN
 * @ERT_FA_DESC_DECERR
 * @ERT_FA_TASKCOUNT_DECERR
 */
typedef enum ert_fa_error_type
{
  ERT_FA_DESC_FIFO_OVERRUN = 0x1,
  ERT_FA_DESC_DECERR = 0x2,
  ERT_FA_TASKCOUNT_DECERR = 0x4
} ert_fa_error_t;

/**
 * ERT fast adapter status type
 *
 * @ERT_FA_UNDEFINED
 * @ERT_FA_ISSUED
 * @ERT_FA_COMPLETED
 */
typedef enum ert_fa_status_type
{
  ERT_FA_UNDEFINED = 0xFFFFFFFF,
  ERT_FA_ISSUED = 0x0,
  ERT_FA_COMPLETED = 0x1
} ert_fa_status_t;

/**
 * struct ert_fa_desc_entry - kernel input/output descriptor entry
 *
 * @arg_offset: offset within the acc aperture
 * @arg_size:   size of argument in bytes
 * @arg_value:  arg_size number of bytes containing arg value
 */
struct ert_fa_desc_entry {
  uint32_t arg_offset;
  uint32_t arg_size;
  uint32_t arg_value[];
};

/**
 * struct ert_fa_descriptor - fast adapter kernel descriptor
 *
 * @status:             descriptor control synchronization word
 * @num_input_entries:  number of input arg entries
 * @input_entry_bytes:  total number of bytes for input args
 * @num_output_entries: number of output arg entries
 * @output_entry_bytes: total number of bytes for output args
 * @data:               payload comprised of desc entries for inputs and outputs
 *
 * The io_entries is an array of input entries with num_input_entries
 * elements followed by an array of output entries with
 * num_output_entries elements starting at address io_entries +
 * input_entry_bytes
 */
struct ert_fa_descriptor {
  ert_fa_status_t status;
  uint32_t num_input_entries;
  uint32_t input_entry_bytes;
  uint32_t num_output_entries;
  uint32_t output_entry_bytes;
  uint32_t data[];
};

#ifdef _WIN32
# pragma warning( pop )
#endif

#endif
