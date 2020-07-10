#include "aie_v1.h"

void XAieTile_CoreControl(XAieGbl_Tile *TileInstPtr, u8 Enable, u8 Reset)
{
}

u64 XAieTile_CoreReadTimer(XAieGbl_Tile *TileInstPtr)
{
	return 0;
}

void XAieTile_DmWriteWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal)
{
}

u8 XAieTile_CoreReadStatusDone(XAieGbl_Tile *TileInstPtr)
{
	return 0;
}

u8 XAieTile_LockAcquire(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
                                                                u32 TimeOut)
{
	return 0;
}

u8 XAieTile_LockRelease(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
                                                                u32 TimeOut)
{
	return 0;
}

u8 XAieDma_ShimPendingBdCount(XAieDma_Shim *DmaInstPtr, u32 ChNum)
{
	return 0;
}

void XAieGbl_HwInit(XAieGbl_HwCfg *CfgPtr)
{
}

void XAieGbl_CfgInitialize(XAieGbl *InstancePtr, XAieGbl_Tile *TileInstPtr,
                                                XAieGbl_Config *ConfigPtr)
{
}

XAieGbl_Config *XAieGbl_LookupConfig(u16 DeviceId)
{
	return 0;
}

u32 XAieDma_ShimSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr)
{
	return 0;
}

void XAieDma_ShimBdClearAll(XAieDma_Shim *DmaInstPtr)
{
}

void XAieDma_ShimBdSetAxi(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid,
                                u8 BurstLen, u8 Qos, u8 Cache, u8 Secure)
{
}

u32 XAieLib_NPIRead32(u64 Addr)
{
	return 0;
}

void XAieLib_NPIWrite32(u64 Addr, u32 Data)
{
}

int XAieTile_EventsHandlingInitialize(XAieGbl *AieInst)
{
	return 0;
}

int XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module, u8 Error,
                                       XAieTile_ErrorCallBack Cb, void *Arg)
{
	return 0;
}

int XAieTile_EventsEnableInterrupt(XAieGbl *AieInst)
{
	return 0;
}

u32 XAieTile_DmReadWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset)
{
	return 0;
}

void XAieDma_ShimBdSetAddr(XAieDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh,
                                                u32 AddrLow, u32 Length)
{
}

void XAieDma_ShimBdSetLock(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId,
                u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal)
{
}

void XAieDma_ShimBdWrite(XAieDma_Shim *DmaInstPtr, u8 BdNum)
{
}

u8 XAieDma_ShimWaitDone(XAieDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut)
{
	return 0;
}

u8 XAieLib_NpiAieArrayReset(u8 Reset)
{
	return 0;
}

u8 XAieTile_CoreWaitStatus(XAieGbl_Tile *TileInstPtr, u32 TimeOut, u32 Status)
{
	return 0;
}

u8 XAieTile_CoreWaitCycles(XAieGbl_Tile *TileInstPtr, u32 CycleCnt)
{
	return 0;
}
