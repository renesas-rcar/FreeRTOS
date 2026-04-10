/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ucie/r_ucie.h"
#include "ucie_rfs/ucie_common.h"

e_ucie_linkup_status_t R_UCIE_Setup(e_ucie_ch_t ch, e_ucie_mode_t mode,
                                                    e_ucie_linkspeed_t speed)
{
    e_ucie_linkup_status_t ret;
    
    if (ch != UCIE_CH0 && ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return LINKUP_ERROR;
    }

    if (mode != UCIE_MODE_EP && mode != UCIE_MODE_RC) {
        printf("ERROR: Invalid UCIe mode\n");
        return LINKUP_ERROR;
    }

    if (speed < LINKSPEED_4GTPS || speed > LINKSPEED_16GTPS){
        printf("ERROR: Invalid UCIe link speed\n");
        return LINKUP_ERROR;
    }

    ret = LINKUP_SUCCESS;
    
    return ret;
}

e_ucie_linkup_status_t R_UCIE_Retry_Linkup(e_ucie_ch_t ch, e_ucie_mode_t mode,
                                           e_ucie_linkspeed_t speed, uint16_t retry)
{
    e_ucie_linkup_status_t ret;

    while(retry--) {
        ret = R_UCIE_Setup(ch, mode, speed);
        if (ret != LINKUP_TIMEOUT) {
            return ret;
        }
    }

    return LINKUP_TIMEOUT;
}

uint32_t R_UCIE_HDMA_SetConfig(st_ucie_hdma_cfg_t *cfg)
{
    (void)cfg;
    printf("ERROR: This feature isn't supported in this environment\n");
    return 0;
}

uint32_t R_UCIE_HDMA_Start(st_ucie_hdma_cfg_t *cfg)
{
    (void)cfg;
    printf("ERROR: This feature isn't supported in this environment\n");
    return 0;
}

uint32_t R_UCIE_HDMA_WaitStop(st_ucie_hdma_cfg_t *cfg)
{
    (void)cfg;
    printf("ERROR: This feature isn't supported in this environment\n");
    return 0;
}

uint32_t R_UCIE_HDMA_Stop(st_ucie_hdma_cfg_t *cfg)
{
    (void)cfg;
    printf("ERROR: This feature isn't supported in this environment\n");
    return 0;
}

uint32_t R_UCIE_IATU_SetRegion(st_ucie_iatu_cfg_t *cfg)
{
    e_ucie_ch_t ucie_ch = cfg->ucie_ch;
    e_ucie_iatu_region_t rgn = cfg->rgn;
    uint64_t mSrcAddr = cfg->mSrcAddr;
    uint64_t mDestAddr = cfg->mDestAddr;
    uint32_t size = cfg->size;

    uint32_t base;
    uint32_t val;
    uint32_t timeout;

    if (ucie_ch != UCIE_CH0 && ucie_ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (rgn > IATU_RGN31) {
        printf("ERROR: Invalid iATU region\n");
        return 1;
    }

    if ((mSrcAddr & UCIE_ADDR_MASK) || (mDestAddr & UCIE_ADDR_MASK) || 
                                                    (size & UCIE_ADDR_MASK)) {
        printf("ERROR: Src addr, dest addr and size must be aligned with 4KB\n");
        return 1;
    }

    base = UCIE_APB_BASE(ucie_ch) +  ((uint32_t)rgn * IATU_RGN_OFFSET);

    val = *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF);
    if (val & (1 << UCIE_MAPPING_STATUS_BIT)) {
        printf("ERROR: This region has been used\n");
        return 1;
    }

    *(volatile uint32_t*)(base + UCIE_LOWER_SRC_ADDR_OFF) = (uint32_t)(mSrcAddr & 0xFFFFFFFF);
    *(volatile uint32_t*)(base + UCIE_UPPER_SRC_ADDR_OFF) = (uint32_t)(mSrcAddr >> 32);
    *(volatile uint32_t*)(base + UCIE_LOWER_DST_ADDR_OFF) = (uint32_t)(mDestAddr & 0xFFFFFFFF);
    *(volatile uint32_t*)(base + UCIE_UPPER_DST_ADDR_OFF) = (uint32_t)(mDestAddr >> 32);
    *(volatile uint32_t*)(base + UCIE_MAPPING_SIZE_OFF)   = size;
    *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF)     = (1 << UCIE_MAPPING_EN_BIT);
    
    timeout = 1000000;
    while(timeout) {
        val = *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF);
        if (val & (1 << UCIE_MAPPING_STATUS_BIT)) {
            break;
        }
        timeout--;
    }
    if (!timeout) {
        return 1;
    }

    timeout = 1000000;
    while (timeout) {
        val = *(volatile uint32_t*)(base + UCIE_INT_STS_OFF);
        if (val & (1 << UCIE_MAPPING_INT_BIT)) {
            break;
        }
        timeout--;
    }
    if (!timeout) {
        return 1;
    }
    *(volatile uint32_t*)(base + UCIE_CLR_INT_OFF) = (1 << UCIE_MAPPING_INT_BIT);

    return 0;
}

uint32_t R_UCIE_IATU_UnsetRegion(st_ucie_iatu_cfg_t *cfg)
{
    e_ucie_ch_t ucie_ch = cfg->ucie_ch;
    e_ucie_iatu_region_t rgn = cfg->rgn;

    uint32_t base;
    uint32_t val;
    uint32_t timeout;

    if (ucie_ch != UCIE_CH0 && ucie_ch != UCIE_CH1) {
        printf("ERROR: Invalid UCIe channel\n");
        return 1;
    }

    if (rgn != IATU_RGN0) {
        printf("ERROR: Invalid iATU region. This environment only supports region 0\n");
        return 1;
    }

    base = UCIE_APB_BASE(ucie_ch);

    val = *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF);
    if ((val & (1 << UCIE_MAPPING_STATUS_BIT)) == 0) {
        return 0;
    }

    *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF) = val & (~(1 << UCIE_MAPPING_EN_BIT));
    
    timeout = 1000000;
    while(timeout) {
        val = *(volatile uint32_t*)(base + UCIE_MAPPING_EN_OFF);
        if ((val & (1 << UCIE_MAPPING_STATUS_BIT)) == 0) {
            break;
        }
        timeout--;
    }
    if (!timeout) {
        return 1;
    }
    
    timeout = 1000000;
    while (timeout) {
        val = *(volatile uint32_t*)(base + UCIE_INT_STS_OFF);
        if (val & (1 << UCIE_MAPPING_INT_BIT)) {
            break;
        }
        timeout--;
    }
    if (!timeout) {
        return 1;
    }
    *(volatile uint32_t*)(base + UCIE_CLR_INT_OFF) = (1 << UCIE_MAPPING_INT_BIT);

    return 0;
}

uint32_t R_UCIE_Config(e_ucie_ch_t ch, e_ucie_mode_t mode,
                       e_ucie_linkspeed_t speed, bool init_with_system)
{
    return 0;
}

e_ucie_linkup_status_t R_UCIE_Get_Linkup_Status(e_ucie_ch_t ch)
{
    return LINKUP_SUCCESS;
}
