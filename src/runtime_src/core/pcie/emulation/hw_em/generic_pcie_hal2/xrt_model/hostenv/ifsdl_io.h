/*
 ******************************************************************************
 *
 * Copyright 2020 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
 *
 * AMD is granting you permission to use this software and documentation (if
 * any) (collectively, the “Materials”) pursuant to the terms and conditions of
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

#ifndef __IFSDL_IO_H__
#define __IFSDL_IO_H__

#include <stdint.h>

/* SystemVerilog: IN, OUT, INOUT port */
#define IN
#define OUT
#define INOUT

//! Abstract class for IO Layer functions
class IFSDLIO
{
public:
    /* SMN Requests */
    virtual unsigned int Read_SMN(IN unsigned long long SmnAddress) = 0;
    virtual void Write_SMN(IN unsigned long long SmnAddress, IN unsigned int Value) = 0;
    /* Timescale events */
    virtual void Wait_ns(IN unsigned int TimeToDelayInNanoseconds) = 0;
    virtual void Read_SYSHUB(IN unsigned long long SysHubAddress, OUT void *pData, unsigned num_bytes) = 0;
    virtual void Write_SYSHUB(IN unsigned long long SysHubAddress, IN const void *pData, unsigned num_bytes) = 0;
    /** IP specific callouts **/
    /* Data Fabric */
    // virtual void RibActionFunctionCallout(IN unsigned int ID, IN AMD_DF_CALLOUT_ARGUMENTS *Arguments) = 0;
    // virtual bool RibBooleanFunctionCallout(IN unsigned int ID, IN AMD_DF_CALLOUT_ARGUMENTS *Arguments) = 0;
    // virtual unsigned int RibNumericFunctionCallout(IN unsigned int ID, IN AMD_DF_CALLOUT_ARGUMENTS *Arguments) = 0;
};

#endif