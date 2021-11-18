/*
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "ipuhenv.h"
#include "rbuffer.h"
#include "mgmt_msg.h"
#include "ipurb_hwemu.h"

#include <cstdio>

void RBuffer::AssertHead()
{
    if(GetHeadIndex() > GetBufferSize()) {
        printf("========== Invalid Head! ==========\n");
        printf("Head(0x%X)|Size(0x%X)\n", GetHeadIndex(), GetBufferSize());
        printf("===================================\n");
    }
}

void RBuffer::AssertTail()
{
    if(GetTailIndex() > GetBufferSize()) {
        printf("========== Invalid Tail! ==========\n");
        printf("Tail(0x%X)|Size(0x%X)\n", GetTailIndex(), GetBufferSize());
        printf("===================================\n");
    }
}

uint32_t RBuffer::GetTailIndex()
{
    // return RD_SMN_MMIO_ADDR(m_Tail);
    return RD_SMN_MMIO_ADDR_XRT(m_io_hdl, m_Tail);
}

uint32_t RBuffer::GetHeadIndex()
{
    // return RD_SMN_MMIO_ADDR(m_Head);
    return RD_SMN_MMIO_ADDR_XRT(m_io_hdl, m_Head);
}

uint32_t RBuffer::GetBufferAddr()
{
    return m_Buffer;
}

uint32_t RBuffer::GetBufferSize()
{
    return m_Size;
}

void RBuffer::SetTailIndex(uint32_t index)
{
    if(index <= GetBufferSize()) {
        // WR_SMN_MMIO_ADDR(m_Tail, index);
        WR_SMN_MMIO_ADDR_XRT(m_io_hdl, m_Tail, index);
    } else {
        printf("!!!!! %s : Invalid index: 0x%X max: 0x%X\n", __FUNCTION__, index, GetBufferSize());
    }
}

void RBuffer::SetHeadIndex(uint32_t index)
{
    if(index <= GetBufferSize()) {
        // WR_SMN_MMIO_ADDR(m_Head, index);
        WR_SMN_MMIO_ADDR_XRT(m_io_hdl, m_Head, index);
    } else {
        printf("!!!!! %s : Invalid index: 0x%X max: 0x%X\n", __FUNCTION__, index, GetBufferSize());
    }
}

void RBuffer::SetU32(uint32_t index, uint32_t val)
{
    if(index < GetBufferSize()) {
        // WR_SMN_MMIO_ADDR((GetBufferAddr() + index), val);
        WR_SMN_MMIO_ADDR_XRT(m_io_hdl, (GetBufferAddr() + index), val);
    } else {
        printf("!!!!! %s : Invalid index: 0x%X max: 0x%X\n", __FUNCTION__, index, GetBufferSize());
    }
}

uint32_t RBuffer::GetU32(uint32_t index)
{
    AssertTail();
    AssertHead();
    if(index < GetBufferSize()) {
        // return RD_SMN_MMIO_ADDR((GetBufferAddr() + index));
        return RD_SMN_MMIO_ADDR_XRT(m_io_hdl, (GetBufferAddr() + index));
    } else {
        printf("!!!!! %s : Invalid index: 0x%X max: 0x%X\n", __FUNCTION__, index, GetBufferSize());
    }

    return 0xFFFFFFFF;
}

