#ifndef __AIE_V1_H__
#define __AIE_V1_H__

#include "xaiengine/xaiegbl.h"

#include <stdint.h>
/************************** Constant Definitions *****************************/
typedef uint8_t                 u8;
typedef uint16_t                u16;
typedef int32_t                 s32;
typedef uint32_t                u32;
typedef uint64_t                u64;

#define XAIETILE_CORE_STATUS_DONE                1U
#define XAIETILE_CORE_STATUS_DISABLE             0U

#define XAIEDMA_SHIM_MAX_NUM_CHANNELS           4U
#define XAIEDMA_SHIM_MAX_NUM_DESCRS             16U

#define XAIEDMA_SHIM_ADDRLOW_ALIGN_MASK         0xFU    /* 128-bit aligned */
#define XAIEDMA_SHIM_TXFER_LEN32_MASK           3U

#define XPAR_AIE_DEVICE_ID      1

#define XAIEGBL_MODULE_CORE                     0x1U
#define XAIEGBL_MODULE_PL                       0x2U
#define XAIEGBL_MODULE_MEM                      0x4U
#define XAIEGBL_MODULE_ALL                      (XAIEGBL_MODULE_CORE |\
                                                 XAIEGBL_MODULE_PL |\
                                                 XAIEGBL_MODULE_MEM)

#define XAIEGBL_MODULE_EVENTS_NUM               128U
#define XAIEGBL_CORE_ERROR_NUM                  22U
#define XAIEGBL_MEM_ERROR_NUM                   14U
#define XAIEGBL_PL_ERROR_NUM                    11U
#define XAIEGBL_MODULE_ALL                      (XAIEGBL_MODULE_CORE |\
                                                 XAIEGBL_MODULE_PL |\
                                                 XAIEGBL_MODULE_MEM)
#define XAIETILE_ERROR_ALL              0x0U /**< All errors except those are
                                                  set as poll only (no logging) */


#define XAIEGBL_NOC_DMASTA_STA_IDLE             0x0U
#define XAIEGBL_NOC_DMASTA_STARTQ_MAX           0x4U

#define XAIE_NPI_ISR                                    (0x30U)

#define XAIEDMA_SHIM_LKACQRELVAL_INVALID                0xFFU   /* Invalid value for lock acq/rel */

/*
 * Core module events
 */
#define XAIETILE_EVENT_CORE_NONE                                0U
#define XAIETILE_EVENT_CORE_TRUE                                1U
#define XAIETILE_EVENT_CORE_GROUP_0                     2U
#define XAIETILE_EVENT_CORE_TIMER_SYNC                  3U
#define XAIETILE_EVENT_CORE_TIMER_VALUE_REACHED         4U
#define XAIETILE_EVENT_CORE_PERF_CNT0                   5U
#define XAIETILE_EVENT_CORE_PERF_CNT1                   6U
#define XAIETILE_EVENT_CORE_PERF_CNT2                   7U
#define XAIETILE_EVENT_CORE_PERF_CNT3                   8U
#define XAIETILE_EVENT_CORE_COMBO_EVENT_0               9U
#define XAIETILE_EVENT_CORE_COMBO_EVENT_1               10U
#define XAIETILE_EVENT_CORE_COMBO_EVENT_2               11U
#define XAIETILE_EVENT_CORE_COMBO_EVENT_3               12U
#define XAIETILE_EVENT_CORE_GROUP_PC_EVENT              15U
#define XAIETILE_EVENT_CORE_PC_0                                16U
#define XAIETILE_EVENT_CORE_PC_1                                17U
#define XAIETILE_EVENT_CORE_PC_2                                18U
#define XAIETILE_EVENT_CORE_PC_3                                19U
#define XAIETILE_EVENT_CORE_PC_RANGE_0_1                        20U
#define XAIETILE_EVENT_CORE_PC_RANGE_2_3                        21U
#define XAIETILE_EVENT_CORE_GROUP_CORE_STALL            22U
#define XAIETILE_EVENT_CORE_MEMORY_STALL                        23U
#define XAIETILE_EVENT_CORE_STREAM_STALL                        24U
#define XAIETILE_EVENT_CORE_CASCADE_STALL               25U
#define XAIETILE_EVENT_CORE_LOCK_STALL                  26U
#define XAIETILE_EVENT_CORE_DEBUG_HALTED                        27U
#define XAIETILE_EVENT_CORE_ACTIVE                      28U
#define XAIETILE_EVENT_CORE_DISABLED                    29U
#define XAIETILE_EVENT_CORE_ECC_ERROR_STALL             30U
#define XAIETILE_EVENT_CORE_ECC_SCRUBBING_STALL         31U
#define XAIETILE_EVENT_CORE_GROUP_CORE_PROGRAM_FLOW     32U
#define XAIETILE_EVENT_CORE_INSTR_EVENT_0               33U
#define XAIETILE_EVENT_CORE_INSTR_EVENT_1               34U
#define XAIETILE_EVENT_CORE_INSTR_CALL                  35U
#define XAIETILE_EVENT_CORE_INSTR_RETURN                        36U
#define XAIETILE_EVENT_CORE_INSTR_VECTOR                        37U
#define XAIETILE_EVENT_CORE_INSTR_LOAD                  38U
#define XAIETILE_EVENT_CORE_INSTR_STORE                 39U
#define XAIETILE_EVENT_CORE_INSTR_STREAM_GET            40U
#define XAIETILE_EVENT_CORE_INSTR_STREAM_PUT            41U
#define XAIETILE_EVENT_CORE_INSTR_CASCADE_GET           42U
#define XAIETILE_EVENT_CORE_INSTR_CASCADE_PUT           43U
#define XAIETILE_EVENT_CORE_INSTR_LOCK_ACQUIRE_REQ      44U
#define XAIETILE_EVENT_CORE_INSTR_LOCK_RELEASE_REQ      45U
#define XAIETILE_EVENT_CORE_GROUP_ERRORS0               46U
#define XAIETILE_EVENT_CORE_GROUP_ERRORS1               47U
#define XAIETILE_EVENT_CORE_SRS_SATURATE                        48U
#define XAIETILE_EVENT_CORE_UPS_SATURATE                        49U
#define XAIETILE_EVENT_CORE_FP_OVERFLOW                 50U
#define XAIETILE_EVENT_CORE_FP_UNDERFLOW                        51U
#define XAIETILE_EVENT_CORE_FP_INVALID                  52U
#define XAIETILE_EVENT_CORE_FP_DIV_BY_ZERO              53U
#define XAIETILE_EVENT_CORE_TLAST_IN_WSS_WORDS_0_2      54U
#define XAIETILE_EVENT_CORE_PM_REG_ACCESS_FAILURE       55U
#define XAIETILE_EVENT_CORE_STREAM_PKT_PARITY_ERROR     56U
#define XAIETILE_EVENT_CORE_CONTROL_PKT_ERROR           57U
#define XAIETILE_EVENT_CORE_AXI_MM_SLAVE_ERROR          58U
#define XAIETILE_EVENT_CORE_INSTRUCTION_DECOMPRESSION_ERROR     59U
#define XAIETILE_EVENT_CORE_DM_ADDRESS_OUT_OF_RANGE     60U
#define XAIETILE_EVENT_CORE_PM_ECC_ERROR_SCRUB_CORRECTED        61U
#define XAIETILE_EVENT_CORE_PM_ECC_ERROR_SCRUB_2BIT     62U
#define XAIETILE_EVENT_CORE_PM_ECC_ERROR_1BIT           63U
#define XAIETILE_EVENT_CORE_PM_ECC_ERROR_2BIT           64U
#define XAIETILE_EVENT_CORE_PM_ADDRESS_OUT_OF_RANGE     65U
#define XAIETILE_EVENT_CORE_DM_ACCESS_TO_UNAVAILABLE    66U
#define XAIETILE_EVENT_CORE_LOCK_ACCESS_TO_UNAVAILABLE  67U
#define XAIETILE_EVENT_CORE_INSTR_WARNING               68U
#define XAIETILE_EVENT_CORE_INSTR_ERROR                 69U
#define XAIETILE_EVENT_CORE_GROUP_STREAM_SWITCH         73U
#define XAIETILE_EVENT_CORE_PORT_IDLE_0                 74U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_0              75U
#define XAIETILE_EVENT_CORE_PORT_STALLED_0              76U
#define XAIETILE_EVENT_CORE_PORT_TLAST_0                        77U
#define XAIETILE_EVENT_CORE_PORT_IDLE_1                 78U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_1              79U
#define XAIETILE_EVENT_CORE_PORT_STALLED_1              80U
#define XAIETILE_EVENT_CORE_PORT_TLAST_1                        81U
#define XAIETILE_EVENT_CORE_PORT_IDLE_2                 82U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_2              83U
#define XAIETILE_EVENT_CORE_PORT_STALLED_2              84U
#define XAIETILE_EVENT_CORE_PORT_TLAST_2                        85U
#define XAIETILE_EVENT_CORE_PORT_IDLE_3                 86U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_3              87U
#define XAIETILE_EVENT_CORE_PORT_STALLED_3              88U
#define XAIETILE_EVENT_CORE_PORT_TLAST_3                        89U
#define XAIETILE_EVENT_CORE_PORT_IDLE_4                 90U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_4              91U
#define XAIETILE_EVENT_CORE_PORT_STALLED_4              92U
#define XAIETILE_EVENT_CORE_PORT_TLAST_4                        93U
#define XAIETILE_EVENT_CORE_PORT_IDLE_5                 94U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_5              95U
#define XAIETILE_EVENT_CORE_PORT_STALLED_5              96U
#define XAIETILE_EVENT_CORE_PORT_TLAST_5                        97U
#define XAIETILE_EVENT_CORE_PORT_IDLE_6                 98U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_6              99U
#define XAIETILE_EVENT_CORE_PORT_STALLED_6              100U
#define XAIETILE_EVENT_CORE_PORT_TLAST_6                        101U
#define XAIETILE_EVENT_CORE_PORT_IDLE_7                 102U
#define XAIETILE_EVENT_CORE_PORT_RUNNING_7              103U
#define XAIETILE_EVENT_CORE_PORT_STALLED_7              104U
#define XAIETILE_EVENT_CORE_PORT_TLAST_7                        105U
#define XAIETILE_EVENT_CORE_GROUP_BROADCAST             106U
#define XAIETILE_EVENT_CORE_BROADCAST_0                 107U
#define XAIETILE_EVENT_CORE_BROADCAST_1                 108U
#define XAIETILE_EVENT_CORE_BROADCAST_2                 109U
#define XAIETILE_EVENT_CORE_BROADCAST_3                 110U
#define XAIETILE_EVENT_CORE_BROADCAST_4                 111U
#define XAIETILE_EVENT_CORE_BROADCAST_5                 112U
#define XAIETILE_EVENT_CORE_BROADCAST_6                 113U
#define XAIETILE_EVENT_CORE_BROADCAST_7                 114U
#define XAIETILE_EVENT_CORE_BROADCAST_8                 115U
#define XAIETILE_EVENT_CORE_BROADCAST_9                 116U
#define XAIETILE_EVENT_CORE_BROADCAST_10                        117U
#define XAIETILE_EVENT_CORE_BROADCAST_11                        118U
#define XAIETILE_EVENT_CORE_BROADCAST_12                        119U
#define XAIETILE_EVENT_CORE_BROADCAST_13                        120U
#define XAIETILE_EVENT_CORE_BROADCAST_14                        121U
#define XAIETILE_EVENT_CORE_BROADCAST_15                        122U
#define XAIETILE_EVENT_CORE_GROUP_USER_EVENT            123U
#define XAIETILE_EVENT_CORE_USER_EVENT_0                        124U
#define XAIETILE_EVENT_CORE_USER_EVENT_1                        125U
#define XAIETILE_EVENT_CORE_USER_EVENT_2                        126U
#define XAIETILE_EVENT_CORE_USER_EVENT_3                        127U


/*
 * Memory module events
 */
#define XAIETILE_EVENT_MEM_NONE                         0U
#define XAIETILE_EVENT_MEM_TRUE                         1U
#define XAIETILE_EVENT_MEM_GROUP_0                      2U
#define XAIETILE_EVENT_MEM_TIMER_SYNC                   3U
#define XAIETILE_EVENT_MEM_TIMER_VALUE_REACHED          4U
#define XAIETILE_EVENT_MEM_PERF_CNT0_EVENT              5U
#define XAIETILE_EVENT_MEM_PERF_CNT1_EVENT              6U
#define XAIETILE_EVENT_MEM_COMBO_EVENT_0                        7U
#define XAIETILE_EVENT_MEM_COMBO_EVENT_1                        8U
#define XAIETILE_EVENT_MEM_COMBO_EVENT_2                        9U
#define XAIETILE_EVENT_MEM_COMBO_EVENT_3                        10U
#define XAIETILE_EVENT_MEM_GROUP_WATCHPOINT             15U
#define XAIETILE_EVENT_MEM_WATCHPOINT_0                 16U
#define XAIETILE_EVENT_MEM_WATCHPOINT_1                 17U
#define XAIETILE_EVENT_MEM_GROUP_DMA_ACTIVITY           20U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_START_BD          21U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_START_BD          22U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_START_BD          23U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_START_BD          24U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_FINISHED_BD       25U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_FINISHED_BD       26U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_FINISHED_BD       27U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_FINISHED_BD       28U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_GO_TO_IDLE                29U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_GO_TO_IDLE                30U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_GO_TO_IDLE                31U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_GO_TO_IDLE                32U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_STALLED_LOCK_ACQUIRE      33U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_STALLED_LOCK_ACQUIRE      34U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_STALLED_LOCK_ACQUIRE      35U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_STALLED_LOCK_ACQUIRE      36U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_MEMORY_CONFLICT   37U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_MEMORY_CONFLICT   38U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_MEMORY_CONFLICT   39U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_MEMORY_CONFLICT   40U
#define XAIETILE_EVENT_MEM_GROUP_LOCK                   43U
#define XAIETILE_EVENT_MEM_LOCK_0_ACQUIRED              44U
#define XAIETILE_EVENT_MEM_LOCK_0_RELEASE               45U
#define XAIETILE_EVENT_MEM_LOCK_1_ACQUIRED              46U
#define XAIETILE_EVENT_MEM_LOCK_1_RELEASE               47U
#define XAIETILE_EVENT_MEM_LOCK_2_ACQUIRED              48U
#define XAIETILE_EVENT_MEM_LOCK_2_RELEASE               49U
#define XAIETILE_EVENT_MEM_LOCK_3_ACQUIRED              50U
#define XAIETILE_EVENT_MEM_LOCK_3_RELEASE               51U
#define XAIETILE_EVENT_MEM_LOCK_4_RELEASE               53U
#define XAIETILE_EVENT_MEM_LOCK_5_ACQUIRED              54U
#define XAIETILE_EVENT_MEM_LOCK_5_RELEASE               55U
#define XAIETILE_EVENT_MEM_LOCK_6_ACQUIRED              56U
#define XAIETILE_EVENT_MEM_LOCK_6_RELEASE               57U
#define XAIETILE_EVENT_MEM_LOCK_7_ACQUIRED              58U
#define XAIETILE_EVENT_MEM_LOCK_7_RELEASE               59U
#define XAIETILE_EVENT_MEM_LOCK_8_ACQUIRED              60U
#define XAIETILE_EVENT_MEM_LOCK_8_RELEASE               61U
#define XAIETILE_EVENT_MEM_LOCK_9_ACQUIRED              62U
#define XAIETILE_EVENT_MEM_LOCK_9_RELEASE               63U
#define XAIETILE_EVENT_MEM_LOCK_10_ACQUIRED             64U
#define XAIETILE_EVENT_MEM_LOCK_10_RELEASE              65U
#define XAIETILE_EVENT_MEM_LOCK_11_ACQUIRED             66U
#define XAIETILE_EVENT_MEM_LOCK_11_RELEASE              67U
#define XAIETILE_EVENT_MEM_LOCK_12_ACQUIRED             68U
#define XAIETILE_EVENT_MEM_LOCK_12_RELEASE              69U
#define XAIETILE_EVENT_MEM_LOCK_13_ACQUIRED             70U
#define XAIETILE_EVENT_MEM_LOCK_13_RELEASE              71U
#define XAIETILE_EVENT_MEM_LOCK_14_ACQUIRED             72U
#define XAIETILE_EVENT_MEM_LOCK_14_RELEASE              73U
#define XAIETILE_EVENT_MEM_LOCK_15_ACQUIRED             74U
#define XAIETILE_EVENT_MEM_LOCK_15_RELEASE              75U
#define XAIETILE_EVENT_MEM_GROUP_MEMORY_CONFLICT                76U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_0           77U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_1           78U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_2           79U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_3           80U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_4           81U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_5           82U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_6           83U
#define XAIETILE_EVENT_MEM_CONFLICT_DM_BANK_7           84U
#define XAIETILE_EVENT_MEM_GROUP_ERRORS                 86U
#define XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED 87U
#define XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_2BIT      88U
#define XAIETILE_EVENT_MEM_DM_ECC_ERROR_1BIT            89U
#define XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT            90U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_2       91U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3       92U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_4       93U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_5       94U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_6       95U
#define XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_7       96U
#define XAIETILE_EVENT_MEM_DMA_S2MM_0_ERROR             97U
#define XAIETILE_EVENT_MEM_DMA_S2MM_1_ERROR             98U
#define XAIETILE_EVENT_MEM_DMA_MM2S_0_ERROR             99U
#define XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR             100U
#define XAIETILE_EVENT_MEM_GROUP_BROADCAST              106U
#define XAIETILE_EVENT_MEM_BROADCAST_0                  107U
#define XAIETILE_EVENT_MEM_BROADCAST_1                  108U
#define XAIETILE_EVENT_MEM_BROADCAST_2                  109U
#define XAIETILE_EVENT_MEM_BROADCAST_3                  110U
#define XAIETILE_EVENT_MEM_BROADCAST_4                  111U
#define XAIETILE_EVENT_MEM_BROADCAST_5                  112U
#define XAIETILE_EVENT_MEM_BROADCAST_6                  113U
#define XAIETILE_EVENT_MEM_BROADCAST_7                  114U
#define XAIETILE_EVENT_MEM_BROADCAST_8                  115U
#define XAIETILE_EVENT_MEM_BROADCAST_9                  116U
#define XAIETILE_EVENT_MEM_BROADCAST_10                 117U
#define XAIETILE_EVENT_MEM_BROADCAST_11                 118U
#define XAIETILE_EVENT_MEM_BROADCAST_12                 119U
#define XAIETILE_EVENT_MEM_BROADCAST_13                 120U
#define XAIETILE_EVENT_MEM_BROADCAST_14                 121U
#define XAIETILE_EVENT_MEM_BROADCAST_15                 122U
#define XAIETILE_EVENT_MEM_GROUP_USER_EVENT             123U
#define XAIETILE_EVENT_MEM_USER_EVENT_0                 124U
#define XAIETILE_EVENT_MEM_USER_EVENT_1                 125U
#define XAIETILE_EVENT_MEM_USER_EVENT_2                 126U
#define XAIETILE_EVENT_MEM_USER_EVENT_3                 127U

/*
 * Memory module events: Macros with _NOC is for NoC tile only, not for PL tile. 
 */
#define XAIETILE_EVENT_SHIM_NONE                                0U
#define XAIETILE_EVENT_SHIM_TRUE                                1U
#define XAIETILE_EVENT_SHIM_GROUP_0_                    2U
#define XAIETILE_EVENT_SHIM_TIMER_SYNC                  3U
#define XAIETILE_EVENT_SHIM_TIMER_VALUE_REACHED         4U
#define XAIETILE_EVENT_SHIM_PERF_CNT0_EVENT             5U
#define XAIETILE_EVENT_SHIM_PERF_CNT1_EVENT             6U
#define XAIETILE_EVENT_SHIM_COMBO_EVENT_0               7U
#define XAIETILE_EVENT_SHIM_COMBO_EVENT_1               8U
#define XAIETILE_EVENT_SHIM_COMBO_EVENT_2               9U
#define XAIETILE_EVENT_SHIM_COMBO_EVENT_3               10U
#define XAIETILE_EVENT_SHIM_GROUP_DMA_ACTIVITY_NOC      11U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_0_START_BD_NOC     12U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_1_START_BD_NOC     13U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_0_START_BD_NOC     14U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_1_START_BD_NOC     15U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_0_FINISHED_BD_NOC  16U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_1_FINISHED_BD_NOC  17U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_0_FINISHED_BD_NOC  18U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_1_FINISHED_BD_NOC  19U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_0_GO_TO_IDLE_NOC   20U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_1_GO_TO_IDLE_NOC   21U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_0_GO_TO_IDLE_NOC   22U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_1_GO_TO_IDLE_NOC   23U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_0_STALLED_LOCK_ACQUIRE_NOC 24U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_1_STALLED_LOCK_ACQUIRE_NOC 25U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_0_STALLED_LOCK_ACQUIRE_NOC 26U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_1_STALLED_LOCK_ACQUIRE_NOC 27U
#define XAIETILE_EVENT_SHIM_GROUP_LOCK_NOC              28U
#define XAIETILE_EVENT_SHIM_LOCK_0_ACQUIRED_NOC         29U
#define XAIETILE_EVENT_SHIM_LOCK_0_RELEASE_NOC          30U
#define XAIETILE_EVENT_SHIM_LOCK_1_ACQUIRED_NOC         31U
#define XAIETILE_EVENT_SHIM_LOCK_1_RELEASE_NOC          32U
#define XAIETILE_EVENT_SHIM_LOCK_2_ACQUIRED_NOC         33U
#define XAIETILE_EVENT_SHIM_LOCK_2_RELEASE_NOC          34U
#define XAIETILE_EVENT_SHIM_LOCK_3_ACQUIRED_NOC         35U
#define XAIETILE_EVENT_SHIM_LOCK_3_RELEASE_NOC          36U
#define XAIETILE_EVENT_SHIM_LOCK_4_ACQUIRED_NOC         37U
#define XAIETILE_EVENT_SHIM_LOCK_4_RELEASE_NOC          38U
#define XAIETILE_EVENT_SHIM_LOCK_5_ACQUIRED_NOC         39U
#define XAIETILE_EVENT_SHIM_LOCK_5_RELEASE_NOC          40U
#define XAIETILE_EVENT_SHIM_LOCK_6_ACQUIRED_NOC         41U
#define XAIETILE_EVENT_SHIM_LOCK_6_RELEASE_NOC          42U
#define XAIETILE_EVENT_SHIM_LOCK_7_ACQUIRED_NOC         43U
#define XAIETILE_EVENT_SHIM_LOCK_7_RELEASE_NOC          44U
#define XAIETILE_EVENT_SHIM_LOCK_8_ACQUIRED_NOC         45U
#define XAIETILE_EVENT_SHIM_LOCK_8_RELEASE_NOC          46U
#define XAIETILE_EVENT_SHIM_LOCK_9_ACQUIRED_NOC         47U
#define XAIETILE_EVENT_SHIM_LOCK_9_RELEASE_NOC          48U
#define XAIETILE_EVENT_SHIM_LOCK_10_ACQUIRED_NOC                49U
#define XAIETILE_EVENT_SHIM_LOCK_10_RELEASE_NOC         50U
#define XAIETILE_EVENT_SHIM_LOCK_11_ACQUIRED_NOC                51U
#define XAIETILE_EVENT_SHIM_LOCK_11_RELEASE_NOC         52U
#define XAIETILE_EVENT_SHIM_LOCK_12_ACQUIRED_NOC                53U
#define XAIETILE_EVENT_SHIM_LOCK_12_RELEASE_NOC         54U
#define XAIETILE_EVENT_SHIM_LOCK_13_ACQUIRED_NOC                55U
#define XAIETILE_EVENT_SHIM_LOCK_13_RELEASE_NOC         56U
#define XAIETILE_EVENT_SHIM_LOCK_14_ACQUIRED_NOC                57U
#define XAIETILE_EVENT_SHIM_LOCK_14_RELEASE_NOC         58U
#define XAIETILE_EVENT_SHIM_LOCK_15_ACQUIRED_NOC                59U
#define XAIETILE_EVENT_SHIM_LOCK_15_RELEASE_NOC         60U
#define XAIETILE_EVENT_SHIM_GROUP_ERRORS_               61U
#define XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR     62U
#define XAIETILE_EVENT_SHIM_CONTROL_PKT_ERROR           63U
#define XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC 64U
#define XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_NSU_ERROR_NOC  65U
#define XAIETILE_EVENT_SHIM_AXI_MM_UNSUPPORTED_TRAFFIC_NOC      66U
#define XAIETILE_EVENT_SHIM_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE_NOC   67U
#define XAIETILE_EVENT_SHIM_AXI_MM_BYTE_STROBE_ERROR_NOC        68U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_0_ERROR_NOC                69U
#define XAIETILE_EVENT_SHIM_DMA_S2MM_1_ERROR_NOC                70U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_0_ERROR_NOC                71U
#define XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC                72U
#define XAIETILE_EVENT_SHIM_GROUP_STREAM_SWITCH         73U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_0                 74U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_0              75U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_0              76U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_0                        77U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_1                 78U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_1              79U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_1              80U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_1                        81U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_2                 82U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_2              83U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_2              84U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_2                        85U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_3                 86U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_3              87U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_3              88U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_3                        89U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_4                 90U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_4              91U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_4              92U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_4                        93U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_5                 94U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_5              95U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_5              96U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_5                        97U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_6                 98U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_6              99U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_6              100U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_6                        101U
#define XAIETILE_EVENT_SHIM_PORT_IDLE_7                 102U
#define XAIETILE_EVENT_SHIM_PORT_RUNNING_7              103U
#define XAIETILE_EVENT_SHIM_PORT_STALLED_7              104U
#define XAIETILE_EVENT_SHIM_PORT_TLAST_7                        105U
#define XAIETILE_EVENT_SHIM_GROUP_BROADCAST_A           106U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_0               107U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_1               108U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_2               109U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_3               110U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_4               111U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_5               112U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_6               113U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_7               114U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_8               115U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_9               116U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_10              117U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_11              118U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_12              119U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_13              120U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_14              121U
#define XAIETILE_EVENT_SHIM_BROADCAST_A_15              122U
#define XAIETILE_EVENT_SHIM_GROUP_USER_EVENT            123U
#define XAIETILE_EVENT_SHIM_USER_EVENT_0                        124U
#define XAIETILE_EVENT_SHIM_USER_EVENT_1                        125U
#define XAIETILE_EVENT_SHIM_USER_EVENT_2                        126U
#define XAIETILE_EVENT_SHIM_USER_EVENT_3                        127U

/**
 * This typedef contains configuration information of the tiles.
 */
typedef struct XAieGbl_Tile
{
        u16 RowId;              /**< Row index of the tile in the AIE array */
        u16 ColId;              /**< Column index of the tile  in the AIE array */
        u8 TileType;            /**< AIE tile or Shim tile */
        u64 TileAddr;           /**< 48-bit Tile base address */
        u64 MemModAddr;         /**< 48-bit Memory module base address */
        u64 CoreModAddr;        /**< 48-bit Core module base address */
        u64 NocModAddr;         /**< 48-bit NoC module base address */
        u64 PlModAddr;          /**< 48-bit PL module base address */
        u64 LockAddr;           /**< 48-bit Lock config base address */
        u64 StrmSwAddr;         /**< 48-bit Stream switch config base address */ 
        u8 IsReady;             /**< Tile is initialized and ready */
        u8 IsUsed;              /**< Tile is in use(not gated) */
        u32 MemBCUsedMask;      /**< Memory module used broadcast event mask */
        u32 CoreBCUsedMask;     /**< Core module used broadcast event mask */
        u32 PlIntEvtUsedMask;   /**< PL module used internal event mask */
        void *Private;          /**< Private data */
} XAieGbl_Tile;

/**
 * This typedef contains the lock attributes for the BD.
 */
typedef struct
{
        u8 LockId;                      /**< Lock ID value, ranging from 0-15 */ 
        u8 LkRelEn;                     /**< Lock release enable */
        u8 LkRelVal;                    /**< Lock release value */
        u8 LkRelValEn;                  /**< Lock release value enable */
        u8 LkAcqEn;                     /**< Lock acquire enable */
        u8 LkAcqVal;                    /**< Lock acquire value */
        u8 LkAcqValEn;                  /**< Lock acquire value enable */
} XAieDma_ShimBdLk;

/**
 * This typedef contains the AXI attributes for the BD.
 */
typedef struct
{
        u8 Smid;                        /**< SMID value for the AXI-MM transfer */
        u8 BrstLen;                     /**< AXI Burst length (AxLEN) for the AXI-MM transfer*/
        u8 Qos;                         /**< AXI Qos bits (AxQOS) for the AXI-MM transfer */
        u8 Cache;                       /**< AxCACHE bits for the AXI-MM transfer */
        u8 Secure;                      /**< Secure/Non-secure AXI-MM transfer */
} XAieDma_ShimBdAxi;

#if 0
/*
 * This typedef contains attributes for a tile coordinate.
 */
typedef struct {
        u16 Row;
        u16 Col;
} XAie_LocType;
#endif

/**
 * This typedef contains all the attributes for the BD configuration.
 */
typedef struct
{
        XAieDma_ShimBdLk Lock;          /**< Lock attributes for the BD */
        XAieDma_ShimBdAxi Axi;          /**< AXI attributes for the BD */
        u32 AddrL;                      /**< Lower address (128-bit aligned) */
        u16 AddrH;                      /**< Upper 16-bit base address bits */
        u32 Length;                     /**< Transfer length (in 32-bit words) */
        u8 PktEn;                       /**< Add packet header to data */
        u8 PktType;                     /**< Packet type */
        u8 PktId;                       /**< ID value for the packet */
        u8 NextBdEn;                    /**< Continue with next BD after completion of current BD or stop */
        u8 NextBd;                      /**< Next BD to continue with */
} XAieDma_ShimBd;

/**
 * This typedef is the Shim DMA instance. User is required to allocate memory for this Shim DMA instance
 * and a pointer of the same is passed to the Shim DMA driver functions. Each Shim DMA in the array is
 * required to have its own and unique instance structure.
 */
typedef struct
{
        u64 BaseAddress;                                        /**< Shim DMA base address pointing to the BD0 location */
        u8 BdStart[XAIEDMA_SHIM_MAX_NUM_CHANNELS];              /**< Start BD value for all the 4 channels */
        XAieDma_ShimBd Descrs[XAIEDMA_SHIM_MAX_NUM_DESCRS];     /**< Data structure to hold the 16 descriptors of the Shim DMA */
}XAieDma_Shim;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
        u16 DeviceId;           /**< Unique ID of the AIE device */
        u32 ArrOffset;          /**< AIE array address offset in the system address map */
        u16 NumRows;            /**< Total number of rows including shim */
        u16 NumCols;            /**< Total number of columns */
} XAieGbl_Config;

typedef struct XAieGbl XAieGbl;
/**
 * This function type defines event callback.
 *
 * @param       AieInst - pointer to the AIE instance which the event is from.
 * @param       Loc - Tile location. Indicates which tile the event is from.
 * @param       Module - Module type
 *                      XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL, XAIEGBL_MODULE_MEM
 * @param       Error - Error event id
 *                      48 - 69 for Core module
 *                      87 - 100 for Memory module
 *                      62 - 72 for SHIM PL module
 * @param       Priv - Argument of the callback which has been registered by
 *                      applicaiton.
 * @return      Indicate if the error has been handled or not.
 *              XAIETILE_ERROR_HANDLED, XAIETILE_ERROR_NOTHANDLED
 *              If it returns XAIETILE_ERROR_HANDLED, AIE driver will not take
 *              further action, otherwise, AIE driver will trap the application. 
 */
typedef void (*XAieTile_EventCallBack)(XAieGbl *AieInst, XAie_LocType Loc, u8 Module, u8 Event, void *Priv);

/**
 * struct XAieGbl_EventHandler - AIE event handler
 *
 * @Refs: Reference count to indicate how many tiles registered
 *        for the handler
 * @Cb: Error event user callback
 * @Arg: User registered argument which will be passed to the callback
 */
typedef struct XAieGbl_EventHandlerSt {
        u32 Refs;
        XAieTile_EventCallBack Cb;
        void *Arg;
} XAieGbl_EventHandler;

#define XAIEGBL_HWCFG_SET_CONFIG(cfgptr, rows, cols, arrayoff)  cfgptr->NumRows = rows;\
                                                                cfgptr->NumCols = cols;\
                                                                cfgptr->ArrayOff = arrayoff
/**
 * enum XAieGbl_ErrorHandleStatus - error handled status
 *
 * @XAIETILE_ERROR_HANDLED: error has handled
 * @XAIETILE_ERROR_NOTHANDLED: error has not handled completely, expect driver
 *                              default action.
 */
typedef enum {
        XAIETILE_ERROR_HANDLED          = 0U,
        XAIETILE_ERROR_NOTHANDLED
} XAieGbl_ErrorHandleStatus;

/**
 * This function type defines the error callback.
 *
 * @param       AieInst - pointer to the AIE instance which the error is from.
 * @param       Loc - Tile location. Indicates which tile the error is from.
 * @param       Module - Module type
 *                      XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL, XAIEGBL_MODULE_MEM
 * @param       Error - Error event id
 *                      48 - 69 for Core module
 *                      87 - 100 for Memory module
 *                      62 - 72 for SHIM PL module
 * @param       Priv - Argument of the callback which has been registered by
 *                      applicaiton.
 * @return      Indicate if the error has been handled or not.
 *              XAIETILE_ERROR_HANDLED, XAIETILE_ERROR_NOTHANDLED
 *              If it returns XAIETILE_ERROR_HANDLED, AIE driver will not take
 *              further action, otherwise, AIE driver will trap the application. 
 */
typedef XAieGbl_ErrorHandleStatus (*XAieTile_ErrorCallBack)(XAieGbl *AieInst, XAie_LocType Loc, u8 Module, u8 Error, void *Priv);

/**
 * struct XAieGbl_ErrorHandler - AIE error handler
 *
 * @Pid: Linux only, pthread ID of the user thread which registers
 *       for the callback.
 * @Cb: Error event user callback
 * @Arg: User registered argument which will be passed to the callback
 */
typedef struct XAieGbl_ErrorHandlerSt {
#ifdef __linux__
        int Pid;
#endif
        XAieTile_ErrorCallBack Cb;
        void *Arg;
} XAieGbl_ErrorHandler;

/**
 * The XAie driver instance data. The user is required to allocate a
 * variable of this type for the AIE instance.
 */     
typedef struct XAieGbl
{       
        XAieGbl_Config *Config;  /**< Configuration table entry for the AIE device */
        u32 IsReady;            /**< Device is initialized and ready */
        XAieGbl_Tile *Tiles; /**< Pointer to tiles array */
        XAieGbl_EventHandler CoreEvtHandlers[XAIEGBL_MODULE_EVENTS_NUM]; /**< Core module events handler array */
        XAieGbl_EventHandler MemEvtHandlers[XAIEGBL_MODULE_EVENTS_NUM]; /**< Memory modle events handler array */
        XAieGbl_EventHandler ShimEvtHandlers[XAIEGBL_MODULE_EVENTS_NUM]; /**< Shim module events handler array */
        XAieGbl_ErrorHandler CoreErrHandlers[XAIEGBL_CORE_ERROR_NUM]; /**< Core module error handler array */
        XAieGbl_ErrorHandler MemErrHandlers[XAIEGBL_MEM_ERROR_NUM]; /**< Memory module error handler array */
        XAieGbl_ErrorHandler ShimErrHandlers[XAIEGBL_PL_ERROR_NUM]; /**< PL module error handler array */
        u32 CoreErrsDefaultTrap; /**< Core errors need to trap application */
        u32 MemErrsDefaultTrap; /**< Memory errors need to trap application */
        u32 ShimErrsDefaultTrap; /**< Shim errors need to trap application */
        u32 CoreErrsPollOnly; /**< Core errors polled only, no interrupt */
        u32 MemErrsPollOnly; /**< Memory errors polled only, no interrupt */
        u32 ShimErrsPollOnly; /**< Shim errors polled only, no interrupt */
        uint32_t BroadCastBitmap; /**< Broadcast Signal usage bitmap */
} XAieGbl;

/**
 * This typedef contains the HW configuration data for the AIE array.
 */
typedef struct {
        u8 NumRows;             /**< Number of rows */
        u8 NumCols;             /**< Number of columns */
        u32 ArrayOff;           /**< AIE array address offset in the system address map */
} XAieGbl_HwCfg;

/*****************************************************************************/
/**
*
* This API writes to the Core_control register to enable and/or reset the AIE
* core of the selected tile. This API bluntly writes to the register and any
* gracefullness required in enabling/disabling and/or resetting/unresetting
* the core are required to be handled by the application layer.
*
* @param        TileInstPtr - Pointer to the Tile instance.
* @param        Enable - Enable/Disable the core (1-Enable,0-Disable).
* @param        Reset - Reset/Unreset the core (1-Reset,0-Unreset).
*
* @return       None.
*
* @note         None.
*
*******************************************************************************/ 
void XAieTile_CoreControl(XAieGbl_Tile *TileInstPtr, u8 Enable, u8 Reset);

/*****************************************************************************/
/**
*
* This API returns the current value of the Core module 64-bit timer.
*
* @param        TileInstPtr - Pointer to the Tile instance.
*
* @return       64-bit timer value.
*
* @note         None.
*
*******************************************************************************/ 
u64 XAieTile_CoreReadTimer(XAieGbl_Tile *TileInstPtr);

/*****************************************************************************/
/**
*
* This API writes a 32-bit value to the specified data memory location for
* the selected tile.
*
* @param        TileInstPtr - Pointer to the Tile instance.
* @param        DmOffset - Data memory offset to write to.
* @param        DmVal - 32-bit Value to be written.
*
* @return       None.
*
* @note         None.
*
*******************************************************************************/ 
void XAieTile_DmWriteWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset, u32 DmVal);

/**
*
* This API returns the current value of the Core status done bit.
*
* @param        TileInstPtr - Pointer to the Tile instance.
*
* @return       Core_Done status bit.
*
* @note         None.
*
*******************************************************************************/ 
u8 XAieTile_CoreReadStatusDone(XAieGbl_Tile *TileInstPtr);

/**
*
* This API is used to acquire the specified lock with or without value. Lock
* acquired without value if LockVal==0xFF. This API can be blocking or non-
* blocking based on the TimeOut value. If TimeOut==0, then API behaves in a
* non-blocking fashion and returns immediately after the first read request.
* If TimeOut>0, then API becomes blocking and will issue the read request until
* the acquire is successful or time out happens, whichever occurs first.
*
* @param        TileInstPtr - Pointer to the Tile instance.
* @param        LockId - Lock value index, ranging from 0-15.
* @param        LockVal - Lock value used for acquire. If set to 0xFF, lock
*               acquired with no value.
* @param        TimeOut - Time-out value for which the read request needs to
*               be repeated. Value to be specified in usecs.
*
* @return       1 if acquire successful, else 0.
*
* @note         None.
*
*******************************************************************************/ 
u8 XAieTile_LockAcquire(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
                                                                u32 TimeOut);

/**
*
* This API is used to release the specified lock with or without value. Lock
* released without value if LockVal==0xFF. This API can be blocking or non-
* blocking based on the TimeOut value. If TimeOut==0, then API behaves in a
* non-blocking fashion and returns immediately after the first read request.
* If TimeOut>0, then API becomes blocking and will issue the read request until
* the release is successful or time out happens, whichever occurs first.
*
* @param        TileInstPtr - Pointer to the Tile instance.
* @param        LockId - Lock value index, ranging from 0-15.
* @param        LockVal - Lock value used for release. If set to 0xFF, lock
*               released with no value.
* @param        TimeOut - Time-out value for which the read request needs to
*               be repeated. Value to be specified in usecs.
*
* @return       1 if release successful, else 0.
*
* @note         None.
*
*******************************************************************************/ 
u8 XAieTile_LockRelease(XAieGbl_Tile *TileInstPtr, u8 LockId, u8 LockVal,
                                                                u32 TimeOut);

/**
*
* This API is used to get the count of scheduled BDs in pending
*
* @param        DmaInstPtr - Pointer to the Shim DMA instance.
* @param        ChNum - Should be one of XAIEDMA_SHIM_CHNUM_S2MM0,
* XAIEDMA_SHIM_CHNUM_S2MM1, XAIEDMA_SHIM_CHNUM_MM2S0, or XAIEDMA_SHIM_CHNUM_MM2S1.
*
* @return       Number of scheduled BDs in pending
*
* @note         This function checks the number of pending BDs in the queue
* as well as if there's any BD that the channel is currently operating on.
* If multiple BDs are chained, it's counted as one BD.
*
*******************************************************************************/ 
u8 XAieDma_ShimPendingBdCount(XAieDma_Shim *DmaInstPtr, u32 ChNum);

/*****************************************************************************/
/**
*
* This is the routine to initialize the HW configuration.
*
* @param        CfgPtr: Pointer to the HW configuration data structure.
*
* @return       None.           
*
* @note         None.
*       
*******************************************************************************/ 
void XAieGbl_HwInit(XAieGbl_HwCfg *CfgPtr);
 
/*****************************************************************************/
/**
*
* This is the global initialization function for all the tiles of the AIE array
* and also for the Shim tiles. The initialization involves programming the Tile
* instance data structure with the required parameters of the tile, like base
* addresses for Core module/Memory module/NoC module/Pl module, Stream switch
* configuration, Lock configuration etc.
*
* @param        InstancePtr - Global AIE instance structure.
* @param        ConfigPtr - Global AIE configuration pointer.
*
* @return       void.
*
* @note         None.           
*                               
******************************************************************************/
void XAieGbl_CfgInitialize(XAieGbl *InstancePtr, XAieGbl_Tile *TileInstPtr,
                                                XAieGbl_Config *ConfigPtr);

/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param        DeviceId is the unique identifier for a device.
*
* @return       A pointer to the XAieGbl configuration structure for the
*               specified device, or NULL if the device was not found.
*
* @note         None.
*
******************************************************************************/
XAieGbl_Config *XAieGbl_LookupConfig(u16 DeviceId);

u32 XAieDma_ShimSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr);

void XAieDma_ShimBdClearAll(XAieDma_Shim *DmaInstPtr);

/*****************************************************************************/
/**
*
* Macro to configure the selected Shim DMA channel conrol bits.
*
* @param        DmaInstPtr - Pointer to the Shim DMA instance.
* @param        ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
* @param        PauseStrm - When set, pauses the stream traffic.
* @param        PauseMm - When set, pauses the issuing of new AXI-MM commands.
* @param        Enable - Enable channel (1-Enable,0-Disable).
*
* @return       None.
*
* @note         None.
*
*******************************************************************************/ 
#define XAieDma_ShimChControl(DmaInstPtr, ChNum, PauseStrm, PauseMm, Enable)    

void XAieDma_ShimBdSetAxi(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid,
                                u8 BurstLen, u8 Qos, u8 Cache, u8 Secure);

u32 XAieLib_NPIRead32(u64 Addr);


void XAieLib_NPIWrite32(u64 Addr, u32 Data);

#define XAieGbl_NPIRead32                XAieLib_NPIRead32
#define XAieGbl_NPIWrite32               XAieLib_NPIWrite32


int XAieTile_EventsHandlingInitialize(XAieGbl *AieInst);


int XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module, u8 Error,
                                       XAieTile_ErrorCallBack Cb, void *Arg);

int XAieTile_EventsEnableInterrupt(XAieGbl *AieInst);

u32 XAieTile_DmReadWord(XAieGbl_Tile *TileInstPtr, u32 DmOffset);

void XAieDma_ShimBdSetAddr(XAieDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh,
                                                u32 AddrLow, u32 Length);

void XAieDma_ShimBdSetLock(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId,
                u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal);

void XAieDma_ShimBdWrite(XAieDma_Shim *DmaInstPtr, u8 BdNum);

#define XAieDma_ShimSetStartBd(DmaInstPtr, ChNum, StartBd)

u8 XAieDma_ShimWaitDone(XAieDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut);

u8 XAieLib_NpiAieArrayReset(u8 Reset);

u8 XAieTile_CoreWaitStatus(XAieGbl_Tile *TileInstPtr, u32 TimeOut, u32 Status);

u8 XAieTile_CoreWaitCycles(XAieGbl_Tile *TileInstPtr, u32 CycleCnt);

#endif
