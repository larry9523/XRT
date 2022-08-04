#pragma once

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "hostenv_types.h"

class RBuffer
{
private:
    void AssertHead();
    void AssertTail();

protected:
    uint32_t m_Head;
    uint32_t m_Tail;
    uint32_t m_Buffer;
    uint32_t m_Size;

    uint64_t m_io_hdl;
public:
    RBuffer(uint32_t head, uint32_t tail, uint32_t buffer, uint32_t bufferSize, uint64_t io_hdl)
     : m_Head( head )
     , m_Tail( tail )
     , m_Buffer( buffer )
     , m_Size( bufferSize )
     , m_io_hdl(io_hdl)
    { }

    uint32_t GetTailIndex();
    uint32_t GetHeadIndex();
    uint32_t GetBufferAddr();
    uint32_t GetBufferSize();

    void SetTailIndex(uint32_t index);
    void SetHeadIndex(uint32_t index);

    uint32_t ReadBuffer(uint32_t offset, void *outBuffer, uint32_t len);
    uint32_t WriteBuffer(uint32_t offset, void *inBuffer, uint32_t len);

    void SetU32(uint32_t index, uint32_t val);
    uint32_t GetU32(uint32_t index);
};

