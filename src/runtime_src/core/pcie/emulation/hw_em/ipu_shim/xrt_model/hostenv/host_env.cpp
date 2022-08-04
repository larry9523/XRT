/*
 ******************************************************************************
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2020-2021 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 * 
 * AMD is granting you permission to use this software and documentation (if
 * any) (collectively, the �Materials�) pursuant to the terms and conditions of
 * the Software License Agreement included with the Materials.  If you do not
 * have a copy of the Software License Agreement, contact your AMD
 * representative for a copy.
 *
 * You agree that you will not reverse engineer or decompile the Materials, in
 * whole or in part, except as allowed by applicable law.
 *
 * WARRANTY DISCLAIMER:  THE MATERIALS ARE PROVIDED "AS IS" WITHOUT WARRANTY OF
 * ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
 * INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE
 * MATERIALS WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM
 * CUSTOM OF TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE
 * OF THE MATERIAL IS ASSUMED BY YOU.  Some jurisdictions do not allow the
 * exclusion of implied warranties, so the above exclusion may not apply to
 * You.
 *
 * LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
 * NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
 * INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
 * THE MATERIALS OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  In no event shall AMD's total
 * liability to You for all damages, losses, and causes of action (whether in
 * contract, tort (including negligence) or otherwise) exceed the amount of
 * $100 USD. You agree to defend, indemnify and hold harmless AMD and its
 * licensors, and any of their directors, officers, employees, affiliates or
 * agents from and against any and all loss, damage, liability and other
 * expenses (including reasonable attorneys' fees), resulting from Your use of
 * the Materials or violation of the terms and conditions of this Agreement.
 *
 * U.S. GOVERNMENT RESTRICTED RIGHTS:  The Materials are provided with
 * "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is
 * subject to the restrictions as set forth in FAR 52.227-14 and
 * DFAR252.227-7013, et seq., or its successor.  Use of the Materials by the
 * Government constitutes acknowledgment of AMD's proprietary rights in them.
 *
 * EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
 * stated in the Software License Agreement.
 *******************************************************************************
 */

/* Standard include: FILE, printf */
#include <stdio.h>
#include <stdlib.h>
/* Self Include */
#include "host_env.h"
/* FSDL IO Layer */
#include "ifsdl_io.h"

//! Pointer to IO Layer Abstract Class
IFSDLIO *g_FSDLIOLayer = NULL;

/**
 * @brief Portal to fsdl IP layer from stream device
 * @param cPtr Class pointer to fsdl IO Layer
 * @return None
 */
extern "C" void fsdlPortal(IFSDLIO *cPtr)
{
    g_FSDLIOLayer = cPtr;
}

/**
 * @brief Wrapper for Simnow's Read_SMN
 * @param SmnAddress SMN Address
 * @return unsigned int Read value
 */
unsigned int RD_SMN_ADDR(IN unsigned long long SmnAddress)   //TODO: template/upcast
{
    printf("Wrapper::RD_SMN_ADDR: 0x%llx\n", SmnAddress);
    if(g_FSDLIOLayer != nullptr)
        return g_FSDLIOLayer->Read_SMN(SmnAddress);
    return 0;
}

/**
 * @brief Wrapper for Simnow's Write_SMN routine
 * @param SmnAddress SMN Address
 * @param Value Value to be writen at SmnAddress
 * @return None
 */
void WR_SMN_ADDR(IN unsigned long long SmnAddress, IN unsigned int Value)  //TODO: smn_addr: upcast
{
    // printf("Wrapper::WR_SMN_ADDR: 0x%llx\n", SmnAddress);
    if(g_FSDLIOLayer != nullptr)
        g_FSDLIOLayer->Write_SMN(SmnAddress, Value);
}

/**
 * @brief Wrapper for Simnow's Wait_ns routine
 * @param TimeToDelayInNanoseconds Delay time in ns
 * @return None
 */
void WAIT_NS(IN unsigned int TimeToDelayInNanoseconds)
{
    // printf("Wrapper::WAIT_NS: 0x%x\n", TimeToDelayInNanoseconds);
    // if(g_FSDLIOLayer != nullptr)
    //   g_FSDLIOLayer->Wait_ns(TimeToDelayInNanoseconds);
}

/**
 * @brief Wrapper for Simnow's Read_SYSHUB
 * @param SysHubAddress SYSHUB Address to read
 * @param pData pointer to buffer that will be populated with syshub contents
 * @param num_bytes Number of bytes to read
 * @return no return value
 */
void RD_SYSHUB(IN unsigned long long SysHubAddress, OUT void *pData, unsigned num_bytes)
{
    // printf("Wrapper::RD_SYSHUB: 0x%llx\n", SysHubAddress);
    if(g_FSDLIOLayer != nullptr)
        g_FSDLIOLayer->Read_SYSHUB(SysHubAddress, pData, num_bytes);
}

/**
 * @brief Wrapper for Simnow's Write_SYSHUB
 * @param SysHubAddress SYSHUB Address to write
 * @param pData pointer to buffer that will be written to syshub
 * @param num_bytes Number of bytes to write
 * @return no return value
 */
void WR_SYSHUB(IN unsigned long long SysHubAddress, IN const void *pData, unsigned num_bytes)
{
    // printf("Wrapper::WR_SYSHUB: 0x%llx\n", SysHubAddress);
    if(g_FSDLIOLayer != nullptr)
        g_FSDLIOLayer->Write_SYSHUB(SysHubAddress, pData, num_bytes);
}
