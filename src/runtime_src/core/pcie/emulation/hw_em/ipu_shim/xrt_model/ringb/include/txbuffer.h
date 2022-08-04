#pragma once

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "hostenv_types.h"

class TxBuffer : public RBuffer
{
private:
    void UpdateTail(const uint32_t index);
    uint32_t ReadData(uint32_t index);

public:
    TxBuffer(uint32_t head, uint32_t tail, uint32_t buffer, uint32_t bufferSize, uint64_t io_hdl)
     : RBuffer(head, tail, buffer, bufferSize, io_hdl)
    { }

    bool IsEmpty();

    void DebugPrint(const char *name);

    bool CheckTail(const uint32_t needed);

    void WriteData(uint32_t index, uint32_t data);

    void SetTombstone();

    void PushTail(const uint32_t len);
};

