/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_HDMA_H_
#define R_UCIE_HDMA_H_

#include "ucie_common.h"

/* HDMA_EN */
#define HDMA_EN_ENABLE      BIT_MASK(0)
#define HDMA_EN_DISABLE     (0U)

/* HDMA_INT_SETUP */
#define HDMA_INT_SETUP_RSIE         BIT_MASK(3)
#define HDMA_INT_SETUP_LSIE         BIT_MASK(4)
#define HDMA_INT_SETUP_RAIE         BIT_MASK(5)
#define HDMA_INT_SETUP_LAIE         BIT_MASK(6)

/* HDMA_DOORBELL */
#define HDMA_DOORBELL_DB_START      BIT_MASK(0) /* HDMA Write Channel Doorbell Start */
#define HDMA_DOORBELL_DB_STOP       BIT_MASK(1) /* HDMA Write Channel Doorbell Stop */

/* HDMA_STATUS_OFF */
#define HDMA_STATUS_RUNNING         (0x1U)
#define HDMA_STATUS_STOPPED         (0x3U)

/* HDMA_INT_CLEAR_OFF */
#define HDMA_INT_ABORT_CLEAR        BIT_MASK(2) /* Setting this field clears the ABORT and ERROR fields of the HDMA_INT_STATUS_OFF_WRCH_i register. */
#define HDMA_INT_WATERMARK_CLEAR    BIT_MASK(1) /* Setting this field clears the WATERMARK field of the HDMA_INT_STATUS_OFF_WRCH_i register */
#define HDMA_INT_STOP_CLEAR         BIT_MASK(0) /* Setting this field clears the STOP field of the HDMA_INT_STATUS_OFF_WRCH_i register. */
#define HDMA_INT_CLR_ABORT_WATERMARK_STOP_FIELD   (0x7U) /* Clear ABORT, ERROR, WATERMARK, and STOP fields */

#endif // R_UCIE_HDMA_H_
