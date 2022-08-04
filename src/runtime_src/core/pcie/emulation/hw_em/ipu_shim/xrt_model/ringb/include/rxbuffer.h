#pragma once

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright(C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 */

class RxBuffer : public RBuffer
{
private:

public:
    RxBuffer(uint32_t head, uint32_t tail, uint32_t buffer, uint32_t bufferSize, uint64_t io_hdl)
     : RBuffer(head, tail, buffer, bufferSize, io_hdl)
    { }

    bool IsEmpty();

    void DebugPrint(const char *name);

    uint32_t ReadData(uint32_t index);

    void CheckHead();

    void PopHead(uint32_t len);

    void ClearTombstone();

    bool WaitForData(uint32_t needed, uint32_t wait = 5000);

    uint32_t GetAvailable();
};

