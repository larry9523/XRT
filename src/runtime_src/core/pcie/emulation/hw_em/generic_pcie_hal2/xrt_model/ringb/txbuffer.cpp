/*
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "rbuffer.h"
#include "txbuffer.h"
#include "ipuhenv.h"
#include "mgmt_msg.h"
#include <cstdio>

bool TxBuffer::IsEmpty()
{
    return (GetHeadIndex() == GetTailIndex());
}

void TxBuffer::DebugPrint(const char *name)
{
    printf("%s TX::Head(0x%X)|", name, m_Head);
    printf("Tail(0x%X)|", m_Tail);
    printf("Buffer(0x%X)|", m_Buffer);
    printf("HeadV(0x%X)|", GetHeadIndex());
    printf("TailV(0x%X)|", GetTailIndex());
    printf("Size(0x%X)\n", m_Size);
}

void TxBuffer::WriteData(uint32_t index, uint32_t data)
{
    SetU32(index, data);
}

uint32_t TxBuffer::ReadData(uint32_t index)
{
    return GetU32(index);
}

void TxBuffer::SetTombstone()
{
    SetU32(GetTailIndex(), IPU_MSG_TOMBSTONE);
}

bool TxBuffer::CheckTail(const uint32_t needed)
{
    printf("CheckTail(needed=0x%X) : Head(0x%X)|Tail(0x%X)\n", needed, GetHeadIndex(), GetTailIndex());
    uint32_t sizeAtTail = 0;

    if(GetTailIndex() < GetHeadIndex()) {
        // Currently the tail is wrapped and the head is in front, so we can use all the data
        // between the current tail and the head
        sizeAtTail = GetHeadIndex() - GetTailIndex();
    } else {
        // If the head and tail aren't swapped, we can check how much room is available between
        // the tail and the end of the buffer.
        sizeAtTail = GetBufferSize() - GetTailIndex();
        if(sizeAtTail < needed) {
            // If there still wasn't room, all that's left if what's available between the start
            // of the buffer and the current head pointer
            sizeAtTail = GetHeadIndex();
            if(sizeAtTail >= needed) {
                // If there is space at the start of the buffer, it has to be wrapped before it
                // can be used.
                printf("[RING ***** ##### @@@@@ !!!!! *****] Setting Tombstone - wrapping TX buffer\n");
                if(GetTailIndex() < GetBufferSize()) {
                    // If there was abandoned space at the end of the buffer, set the tombstone
                    SetTombstone();
                }
                UpdateTail(0);
            }
        }
    }

    return (needed <= sizeAtTail);
}

void TxBuffer::UpdateTail(const uint32_t index)
{
    SetTailIndex(index);
}

void TxBuffer::PushTail(const uint32_t len)
{
    SetTailIndex(GetTailIndex() + len);
}

