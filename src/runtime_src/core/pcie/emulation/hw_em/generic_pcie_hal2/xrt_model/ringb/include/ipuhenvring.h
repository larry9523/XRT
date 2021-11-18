#pragma once

/*
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "rbuffer.h"
#include "txbuffer.h"
#include "rxbuffer.h"
#include "os.h"
#include "mgmt_msg.h"

class IpuHenvRing
{
private:
public:
    TxBuffer m_TxBuffer;
    RxBuffer m_RxBuffer;

public:
    IpuHenvRing(const os_ipu_mnmg_ch_t& txChan, const os_ipu_mnmg_ch_t& rxChan, uint64_t io_hdl)
     : m_TxBuffer(txChan.head_ptr, txChan.tail_ptr, txChan.buffer_ptr, txChan.buffer_size, io_hdl)
     , m_RxBuffer(rxChan.head_ptr, rxChan.tail_ptr, rxChan.buffer_ptr, rxChan.buffer_size, io_hdl)
    { }

    IpuHenvRing(const ipu_command_queue_info_t& txChan, const ipu_command_queue_info_t& rxChan, uint64_t io_hdl)
     : m_TxBuffer(txChan.mailbox_head_ptr_offset, txChan.mailbox_tail_ptr_offset, txChan.buffer_start_address
         , txChan.buffer_size, io_hdl)
     , m_RxBuffer(rxChan.mailbox_head_ptr_offset, rxChan.mailbox_tail_ptr_offset, rxChan.buffer_start_address
         , rxChan.buffer_size, io_hdl)
    { }

    void DebugPrint(const char *name);

    TxBuffer& Tx() { return m_TxBuffer; }

    RxBuffer& Rx() { return m_RxBuffer; }

    uint32_t WriteBuffer(uint32_t offset, void *pData, uint32_t len);

    /**
     * Copy data to pData
     *
     * @param offset offset into buffer to write the data
     * @param index  index of data to copy from buffer
     * @param wait   number of cycles to wait for tail to update before giving up
     *
     * @return uint32_t the amount of data copied
     */
    uint32_t ReadBuffer(uint32_t offset, void *pData, uint32_t len);
};

