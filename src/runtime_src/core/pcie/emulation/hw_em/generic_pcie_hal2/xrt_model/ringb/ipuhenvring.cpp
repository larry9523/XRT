/*
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "ipuhenv.h"
#include "rbuffer.h"
#include "host_env.h"
#include "ipuhenvring.h"

#include <cstring> // for memcpy
#include <algorithm> // for std::min

#ifndef CONFIG_RING_DEBUG_PRINT
#define CONFIG_RING_DEBUG_PRINT 0
#endif // CONFIG_RING_DEBUG_PRINT

void IpuHenvRing::DebugPrint(const char *name)
{
#if CONFIG_RING_DEBUG_PRINT
    m_RxBuffer.DebugPrint(name);
    m_TxBuffer.DebugPrint(name);
#endif
}

uint32_t IpuHenvRing::WriteBuffer(uint32_t offset, void *pData, uint32_t len)
{
    const uint32_t index = m_TxBuffer.GetTailIndex() + offset;
    uint8_t *pData8 = (uint8_t *)pData;
    uint32_t txCount = 0;

    while(txCount < len) {
        const uint32_t fit = std::min((uint32_t)sizeof( uint32_t ), ( len - txCount ));
        uint32_t u32Val = 0;
            memcpy(& u32Val, & pData8[txCount], fit);
        m_TxBuffer.WriteData(index + txCount, u32Val);
        txCount += fit;
    }

    return txCount;
}

uint32_t IpuHenvRing::ReadBuffer(uint32_t offset, void *pData, uint32_t len)
{
    const uint32_t index = m_RxBuffer.GetHeadIndex() + offset;
    uint8_t *pData8 = (uint8_t *)pData;
    uint32_t rxCount = 0;

    while(rxCount < len) {
        const uint32_t fit = std::min((uint32_t)sizeof( uint32_t ), ( len - rxCount ));
        uint32_t u32Val = m_RxBuffer.ReadData(index + rxCount);
        memcpy(&pData8[rxCount], &u32Val, fit);
        rxCount += fit;
    }

    return rxCount;
}

