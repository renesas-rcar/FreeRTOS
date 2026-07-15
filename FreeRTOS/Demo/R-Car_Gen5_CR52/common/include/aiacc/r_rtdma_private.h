/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup RTDMA_PRIVATE RTDMA PRIVATE
 * @{
 * @brief Work around for RTDMA issue on AIACC platform.
 *
 */

#ifndef R_RTDMA_PRIVATE_H
#define R_RTDMA_PRIVATE_H

#include "dmac_ctrl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RTDMA Workaround functions */
static inline uint16_t R_RTDMAC_WA_16_BYTES(rDmacCfg_t *cfg, rDmacDescCfg_t *descCfg)
{
    (void) cfg;
    (void) descCfg;
    return drv_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* R_RTDMA_PRIVATE_H */
