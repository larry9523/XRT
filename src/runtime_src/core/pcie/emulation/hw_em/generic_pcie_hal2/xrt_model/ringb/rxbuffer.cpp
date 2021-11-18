/*
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "rbuffer.h"
#include "rxbuffer.h"
#include "ipuhenv.h"
#include "mgmt_msg.h"
#include <cstdio>

bool RxBuffer::IsEmpty()
{
    return (GetHeadIndex() == GetTailIndex());
}

void RxBuffer::DebugPrint(const char *name)
{
    printf("%s RX::Head(0x%X)|", name, m_Head);
    printf("Tail(0x%X)|", m_Tail);
    printf("Buffer(0x%X)|", m_Buffer);
    printf("HeadV(0x%X)|", GetHeadIndex());
    printf("TailV(0x%X)|", GetTailIndex());
    printf("Size(0x%X)\n", m_Size);
}

uint32_t RxBuffer::ReadData(uint32_t index)
{
    return GetU32(index);
}

void RxBuffer::ClearTombstone()
{
    SetU32(GetHeadIndex(), 0);
}

bool RxBuffer::WaitForData(uint32_t needed, uint32_t wait)
{
    uint32_t maxWait = wait;
    printf("WaitForData(needed=0x%X, wait=%u) ... Start\n", needed, wait);
    if(wait) {
        while(GetHeadIndex() == GetTailIndex() && wait--);

        while(GetAvailable() < needed && wait--);
    } else {
        // Wait FOREVER
        while(GetHeadIndex() == GetTailIndex());
    }

    printf("WaitForData(wait=%u) ... Done Head(0x%X)|Tail(0x%X)|Waited(%us)\n"
        , wait, GetHeadIndex(), GetTailIndex(), ( maxWait - wait ));
    return (GetHeadIndex() != GetTailIndex());
}

void RxBuffer::PopHead(uint32_t len)
{
    if(len & 0x3) {
        printf("[RING] !!! Unaligned head value: 0x%X\n", GetHeadIndex() + len);
    }
    printf("PopHead(len=0x%X)\n", len);
    SetHeadIndex(GetHeadIndex() + len);
}

void RxBuffer::CheckHead()
{
    if(IPU_MSG_TOMBSTONE == ReadData(GetHeadIndex())) {
        printf("[RING ***** ##### @@@@@ !!!!! *****] Found Tombstone - wrapping RX buffer\n");
        ClearTombstone();
        SetHeadIndex(0);
    } else if(GetBufferSize() == GetHeadIndex()) {
        printf("[RING ***** ##### @@@@@ !!!!! *****] End of buffer - wrapping RX buffer\n");
        SetHeadIndex(0);
    }
}

uint32_t RxBuffer::GetAvailable()
{
    uint32_t sizeAtHead = 0;

    // after this check, head should never point to the tombstone
    CheckHead();
    if(GetTailIndex() < GetHeadIndex()) {
        sizeAtHead = GetBufferSize() - GetHeadIndex();
        for(uint32_t bufferOffset = 0; bufferOffset < sizeAtHead; bufferOffset += 4) {
            if(IPU_MSG_TOMBSTONE == ReadData(GetHeadIndex() + bufferOffset)) {
                sizeAtHead = bufferOffset;
            }
        }
    } else {
        sizeAtHead = GetTailIndex() - GetHeadIndex();
    }

    return sizeAtHead;
}

