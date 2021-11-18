/*
* Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
*/

#ifndef OS_H
#define OS_H

// Pointers are byte offset relative to IPU SRAM 

typedef struct os_ipu_mnmg_ch_t_ {
      uint32_t    tail_ptr;
      uint32_t    head_ptr;
      uint32_t    buffer_ptr;
      uint32_t    buffer_size;
} os_ipu_mnmg_ch_t;

typedef struct os_ipu_mnmg_t_ {
  os_ipu_mnmg_ch_t os_to_ipu_ch;
  os_ipu_mnmg_ch_t ipu_to_os_ch;
} os_ipu_mnmg_t;

typedef union os_ipu_mnmg_u_ {
  os_ipu_mnmg_t f;
  uint32_t      d[sizeof(os_ipu_mnmg_t)/4];
} os_ipu_mnmg_u;


#endif

