/******************************************************************************
 *  Module Name    FW_Public.hpp
 *  Project        IPU Kernel Mode Driver
 *
 *  Description    Framework layer. This class does the following:
 *                 a) Contains common declaration shared by driver and user
 *                    application
 *
 *  Copyright (C) 2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 ******************************************************************************/

#include <stdint.h>

// IPU Driver interface GUID
// Define an Interface Guid so that apps can find the device and talk to it.
// Interface GUID - 7b525d69-1f66-4935-a141-74d0d2503c7c
DEFINE_GUID (GUID_DEVINTERFACE_KIPUDRV,
    0x7b525d69,0x1f66,0x4935,0xa1,0x41,0x74,0xd0,0xd2,0x50,0x3c,0x7c);

// Macro to extract function type
#define FUNCTION_FROM_CTL_CODE(ctrlCode) (((ULONG)(ctrlCode)) >> 2)

// Define a file device
#define FILE_DEVICE_KIPUDRV 0x40011

// Maximum number of bytes supported in IPC message
#define IPC_DATA_SIZE 256

// Number of message stitching tests to return results for
#define MESSAGE_STITCH_TEST_COUNT 7

// Number of bytes supported by File Read/Write OSAL IOCTL Operations
#define FILE_OSAL_DATA_SIZE 512

// Maximum length of OSAL file path
#define FILE_PATH_LEN 512

// Number of bytes supported by RegKey Read/Write OSAL IOCTL Operations
#define REGKEY_OSAL_DATA_SIZE 512

// Maximum length of OSAL registry key path
#define REGKEY_PATH_LEN 512

// IOCTL code
// Define an IOCTL code so that the test apps can use this for ioctl calls
#define IOCTL_KIPUDRV_HELLO_WORLD \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_FILE_CREATE \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x910, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_FILE_WRITE \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x911, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_FILE_READ \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x912, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_FILE_CLOSE \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x913, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_MESSAGE_STITCH_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x914, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_SHARED_XCLBIN_DOWNLOAD_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x915, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_SHARED_XCLBIN_UNLOAD_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x916, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_REGKEY_OPEN \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x917, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_REGKEY_GETSIZE \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x918, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_REGKEY_READ \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x919, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_OSAL_REGKEY_CLOSE \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x91A, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_SHARED_CREATE_DESTROY_CTX_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x920, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_SHARED_IPU_SELF_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x921, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_SHARED_QUERRY_ERROR_TEST \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0x922, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_IPC_INIT \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA00, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_IPC_SEND \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA01, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_IPC_RECV \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA02, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_IPC_DEINIT \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA03, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_MEM_MAP \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA10, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_KIPUDRV_MEM_UNMAP \
    CTL_CODE(FILE_DEVICE_KIPUDRV, 0xA11, METHOD_BUFFERED, FILE_ANY_ACCESS)

// IOCTL codes for XRT Core
#define IOCTL_KIPUDRV_XRT_CORE_FUNCTION 0xB00

#define IOCTL_KIPUDRV_DOWNLOAD_XCLBIN \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x1), \
        METHOD_IN_DIRECT, FILE_READ_DATA)
#define IOCTL_KIPUDRV_CTX \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x2), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_BOARD_INFO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x3), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_SENSOR_INFO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x4), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_CREATE_BO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x5), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_USERPTR_BO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x6), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_MAP_BO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x7), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_SYNC_BO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x8), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_INFO_BO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x9), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_EXECBUF \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xA), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_EXECPOLL \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xB), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_ALLOC_HOST_MEM \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xC), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_FREE_HOST_MEM \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xD), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_ERROR_INFO \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xE), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_ERROR_INJECT \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0xF), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_STAT \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x10), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KIPUDRV_HW_CTX \
    CTL_CODE(FILE_DEVICE_KIPUDRV, (IOCTL_KIPUDRV_XRT_CORE_FUNCTION | 0x11), \
        METHOD_BUFFERED, FILE_ANY_ACCESS)

enum class DRIVER_STATUS
{
    // Generic errors.
    S_SUCCESS = 0,
    E_UNSUCCESSFUL,
    E_INVALID_PARAMETER,
    E_BUFFER_OVERFLOW,
    E_NO_MEMORY,

    UNDEFINED
};

typedef struct PmPowerOnOutput
{
    DRIVER_STATUS status;
} PmPowerOnOutput_t;

typedef struct PmPowerOffOutput
{
    DRIVER_STATUS status;
} PmPowerOffOutput_t;

typedef struct PmSetLevelInput
{
    uint32_t frequency;
} PmSetLevelInut_t;

typedef struct PmSetLevelOutput
{
    DRIVER_STATUS status;
} PmSetLevelOutput_t;

typedef struct OsalFileCreateInput
{
    wchar_t  filePath[FILE_PATH_LEN];
} OsalFileCreateInput_t;

typedef struct OsalFileCreateOutput
{
    HANDLE        fileOpHandler;
    DRIVER_STATUS status;
} OsalFileCreateOutput_t;

typedef struct OsalFileWriteInput
{
    HANDLE   fileOpHandler;
    uint32_t inputDataSize;
    char     data[FILE_OSAL_DATA_SIZE];
} OsalFileWriteInput_t;

typedef struct OsalFileWriteOutput
{
    DRIVER_STATUS status;
} OsalFileWriteOutput_t;

typedef struct OsalFileReadInput
{
    HANDLE   fileOpHandler;
    uint32_t dataSize;
} OsalFileReadInput_t;

typedef struct OsalFileReadOutput
{
    DRIVER_STATUS status;
    char          data[FILE_OSAL_DATA_SIZE];
} OsalFileReadOutput_t;

typedef struct OsalFileCloseInput
{
    HANDLE fileOpHandler;
} OsalFileCloseInput_t;

typedef struct OsalFileCloseOutput
{
    DRIVER_STATUS status;
} OsalFileCloseOutput_t;

typedef struct MessageStitchTestOutput
{
    DRIVER_STATUS status[MESSAGE_STITCH_TEST_COUNT];
} MessageStitchTestOutput_t;

typedef struct SharedBackendOutput
{
    DRIVER_STATUS status;
} SharedBackendOutput_t;

typedef struct OsalRegKeyOpenInput
{
    wchar_t regKeyPath[FILE_PATH_LEN];
} OsalRegKeyOpenInput_t;

typedef struct OsalRegKeyOpenOutput
{
    HANDLE        regKeyOpHandler;
    DRIVER_STATUS status;
} OsalRegKeyOpenOutput_t;

typedef struct OsalRegKeyGetSizeInput
{
    HANDLE  regKeyOpHandler;
    wchar_t regValueName[REGKEY_PATH_LEN];
} OsalRegKeyGetSizeInput_t;

typedef struct OsalRegKeyGetSizeOutput
{
    DRIVER_STATUS status;
    ULONG         size;
} OsalRegKeyGetSizeOutput_t;

typedef struct OsalRegKeyReadInput
{
    HANDLE  regKeyOpHandler;
    ULONG   bufferSize;
    wchar_t regValueName[REGKEY_PATH_LEN];
} OsalRegKeyReadInput_t;

typedef struct OsalRegKeyReadOutput
{
    DRIVER_STATUS status;
    BYTE          data[REGKEY_OSAL_DATA_SIZE];
} OsalRegKeyReadOutput_t;

typedef struct OsalRegKeyCloseInput
{
    HANDLE regKeyOpHandler;
} OsalRegKeyCloseInput_t;

typedef struct OsalRegKeyCloseOutput
{
    DRIVER_STATUS status;
} OsalRegKeyCloseOutput_t;

typedef struct IpcInitOutput
{
    DRIVER_STATUS status;
    HANDLE        ipcHandler;
} IpcInitOutput_t;

typedef struct IpcSendInput
{
    HANDLE   ipcHandler;
    uint32_t opCode;
    char     data[IPC_DATA_SIZE];
    uint32_t dataSize;
} IpcSendInput_t;

typedef struct IpcSendOutput
{
    DRIVER_STATUS status;
    uint32_t      messageId;
} IpcSendOutput_t;

typedef struct IpcRecvInput
{
    HANDLE   ipcHandler;
    uint32_t dataSize;
    uint32_t messageId;
} IpcRecvInput_t;

typedef struct IpcRecvOutput
{
    DRIVER_STATUS status;
    char          data[IPC_DATA_SIZE];
    uint32_t      dataSize;
} IpcRecvOutput_t;

typedef struct IpcDeinitInput
{
    HANDLE ipcHandler;
} IpcDeinitInput_t;

typedef struct IpcDeinitOutput
{
    DRIVER_STATUS status;
} IpcDeinitOutput_t;