/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_COMMON_H
#define R_UCIE_COMMON_H

#include "ucie/r_ucie.h"

#define UCIE_AXI_BASE(n)    (0xD8000000U + (n) * 0x1000000)
#define UCIE_APB_BASE(n)    (0xDC000000U + (n) * 0x1000000)

/* Remote Interrupt Address & Data (T.B.D) */
#define MSI_STOP_BASE           (0x7FFF0000U)
#define MSI_ABORT_BASE          (0x7FFF0200U)
#define MSI_WATERMARK_BASE      (0x7FFF0400U)
#define MSI_DATA                (0xA5A5A5A5U)

#define LINKSPEED_MASK               (0xFUL)
#define LINKSPEED_OFFSET             (0x6UL)

/* UCIE APB power management control registers*/
#define UCIE_APB0_UCIEPWRMNGCTRL    (0xDCE00070)
#define UCIE_APB1_UCIEPWRMNGCTRL    (0xDDE00070)

#define HSCS_APB 0xDE200000

#define HSCS_APB_UCI0CORECKCR   (uintptr_t)(HSCS_APB  + 0x1080  )
#define HSCS_APB_UCI1CORECKCR   (uintptr_t)(HSCS_APB  + 0x1084  )
#define HSCS_APB_PLL9_0_CR0 (uintptr_t)(HSCS_APB  + 0x11EC  )
#define HSCS_APB_PLL9_0_CR1 (uintptr_t)(HSCS_APB  + 0x11F0  )
#define HSCS_APB_PLL9_0_CR2 (uintptr_t)(HSCS_APB  + 0x11F4  )
#define HSCS_APB_PLL9_0SCR  (uintptr_t)(HSCS_APB  + 0x1308  )
#define HSCS_APB_PLL9_0DCR  (uintptr_t)(HSCS_APB  + 0x130C  )
#define HSCS_APB_PLL9_1_CR0 (uintptr_t)(HSCS_APB  + 0x11F8  )
#define HSCS_APB_PLL9_1_CR1 (uintptr_t)(HSCS_APB  + 0x11FC  )
#define HSCS_APB_PLL9_1_CR2 (uintptr_t)(HSCS_APB  + 0x1200  )
#define HSCS_APB_PLL9_1SCR  (uintptr_t)(HSCS_APB  + 0x1310  )
#define HSCS_APB_PLL9_1DCR  (uintptr_t)(HSCS_APB  + 0x1314  )
#define HSCS_APB_CLKHSCSPKCPROT0    (uintptr_t)(HSCS_APB  + 0x1370  )

/**
 * @brief Struct for config UCIe
*/
typedef struct st_ucie_ctrl {
    e_ucie_mode_t mode;         /* UCIe mode */
    e_ucie_linkspeed_t speed;   /* UCIe transfer speed */
    bool init_with_system;      /* Init and linkup when system available */
} st_ucie_ctrl_t;

typedef struct{
    uint32_t PLL9_CR0;
    uint32_t PLL9_CR1;
    uint32_t PLL9_DCR;
}PLL9Param;

typedef struct
{
    uint32_t div_sel;
    uint32_t v2i_mode;
    uint32_t vco_low_freq;
    uint32_t cp_prop_cntrl;
    uint32_t cp_int_cntrl;
    uint32_t cp_prop_gs_cntrl;
    uint32_t cp_int_gs_cntrl;
    uint64_t upll_prog;
} PLLParam;

typedef struct
{
    uint32_t AcsmWaitDly0;
    uint32_t AcsmWaitDly1;
    uint32_t zcalcompstartuptime;
    uint32_t zcalsampletime;
    uint32_t zcaloffsetsampletime;
    uint32_t dwdcdsettletime;
    uint32_t dwdcdsettletimeDW1;
    uint32_t dwdcasettletime;
    uint32_t dwdcasettletimeDW1;
    uint32_t dwdcdsampletime;
    uint32_t dwdcdsampletimeDW1;
    uint32_t acsmpmaborttimeout;
    uint32_t acsmpmentrytimeout;
    uint32_t acsmltsmstatetimeout;
    uint32_t acsmltsmmsgtimeout;
    uint32_t acsmlinkerrtimeout;
} FreqDepPrm;

/* PF0_HDMA_CAP */
#define HDMA_CH_BLOCK_SIZE          (0x800)
#define HDMA_RW_BLOCK_SIZE          (0x400)

#define HDMA_EN_OFF                 (0x00)
#define HDMA_DOORBELL_OFF           (0x04)
#define HDMA_ELEM_PF_OFF            (0x08)
#define HDMA_LLP_LOW_OFF            (0x10)
#define HDMA_LLP_HIGH_OFF           (0x14)
#define HDMA_CYCLE_OFF              (0x18)
#define HDMA_XFERSIZE_OFF           (0x1C)
#define HDMA_SAR_LOW_OFF            (0x20)
#define HDMA_SAR_HIGH_OFF           (0x24)
#define HDMA_DAR_LOW_OFF            (0x28)
#define HDMA_DAR_HIGH_OFF           (0x2C)
#define HDMA_WATERMARK_EN_OFF       (0x30)
#define HDMA_CONTROL1_OFF           (0x34)
#define HDMA_FUNC_NUM_OFF           (0x38)
#define HDMA_QOS_OFF                (0x3C)
#define HDMA_STATUS_OFF             (0x80)
#define HDMA_INT_STATUS_OFF         (0x84)
#define HDMA_INT_SETUP_OFF          (0x88)
#define HDMA_INT_CLEAR_OFF          (0x8C)
#define HDMA_MSI_STOP_LOW_OFF       (0x90)
#define HDMA_MSI_STOP_HIGH_OFF      (0x94)
#define HDMA_MSI_WATERMARK_LOW_OFF  (0x98)
#define HDMA_MSI_WATERMARK_HIGH_OFF (0x9C)
#define HDMA_MSI_ABORT_LOW_OFF      (0xA0)
#define HDMA_MSI_ABORT_HIGH_OFF     (0xA4)
#define HDMA_MSI_MSGD_OFF           (0xA8)

/* PF0_ATU_CAP */
#define IATU_BLOCK_SIZE             (0x200)
#define IATU_INBOUND_OFFSET         (0x100)

#define IATU_REGION_CTRL_1_OFF      (0x00)
#define IATU_REGION_CTRL_2_OFF      (0x04)
#define IATU_LWR_BASE_ADDR_OFF      (0x08)
#define IATU_UPPER_BASE_ADDR_OFF    (0x0C)
#define IATU_LIMIT_ADDR_OFF         (0x10)
#define IATU_LWR_TARGET_ADDR_OFF    (0x14)
#define IATU_UPPER_TARGET_ADDR_OFF  (0x18)
#define IATU_REGION_CTRL_3_OFF      (0x1C)
#define IATU_UPPR_LIMIT_ADDR_OFF    (0x20)

#define IATU_ADDR_MASK              (0xFFF)

void Ucie_PowerOFF(e_ucie_ch_t ch);
void Ucie_PowerOn(e_ucie_ch_t ucie_ch);
void Ucie_Setup_Pre(e_ucie_ch_t ch, e_ucie_mode_t mode);
void Ucie_Start_Linkup(e_ucie_ch_t ch, e_ucie_mode_t mode, e_ucie_linkspeed_t speed);
uint32_t Ucie_Wait_FreqChange_Req(e_ucie_ch_t ch);
uint32_t Ucie_Ack_FreqChange(e_ucie_ch_t ch, e_ucie_linkspeed_t speed);
uint32_t Ucie_Wait_Linkup(e_ucie_ch_t ch);
void Ucie_Setup_PCIE_Pre(e_ucie_ch_t ch, e_ucie_mode_t mode);
void Ucie_Setup_PCIE_Start_LinkUp(e_ucie_ch_t ch, e_ucie_mode_t mode);
uint32_t Ucie_Setup_PCIE_Wait_LinkUp(e_ucie_ch_t ch);
void Ucie_Setup_PCIE_Post(e_ucie_ch_t ch, e_ucie_mode_t mode);

st_ucie_ctrl_t ucie_get_config(e_ucie_ch_t ch);
void ucie_set_setup_flag(e_ucie_ch_t ch);

#endif /* R_UCIE_COMMON_H */
