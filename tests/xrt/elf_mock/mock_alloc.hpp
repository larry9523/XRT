// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.

#pragma once

#include <cstdlib>
#include <stdexcept>

// RAII host buffer — used as fake "device address" for module_int::patch().
class mock_alloc
{
  void* m_ptr = nullptr;
  size_t m_size = 0;

public:
  explicit
  mock_alloc(size_t size)
    : m_size(size)
  {
    if (size == 0)
      throw std::runtime_error("mock_alloc: invalid size");
    m_ptr = std::aligned_alloc(4096, (size + 4095) & ~size_t(4095));
    if (!m_ptr)
      throw std::runtime_error("mock_alloc: aligned_alloc failed");
  }

  ~mock_alloc()
  {
    std::free(m_ptr);
  }

  mock_alloc(const mock_alloc&) = delete;
  mock_alloc& operator=(const mock_alloc&) = delete;

  mock_alloc(mock_alloc&& other) noexcept
    : m_ptr(other.m_ptr)
    , m_size(other.m_size)
  {
    other.m_ptr = nullptr;
    other.m_size = 0;
  }

  void*
  get() const
  {
    return m_ptr;
  }

  size_t
  size() const
  {
    return m_size;
  }

  // Transfer ownership to caller (e.g. xrt::bo host-only constructor).
  void*
  release()
  {
    void* p = m_ptr;
    m_ptr = nullptr;
    return p;
  }
};
