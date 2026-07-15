/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup I2C_PRIVATE I2C PRIVATE
 * @{
 * @brief Base address of I2C X5H platform.
 *
 */

#ifndef R_I2C_PRIVATE_H
#define R_I2C_PRIVATE_H

#include "stdint.h"
#include "dmac/sysdmac_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* I2C X5H uses SYSDMA */
#define RCAR_DMAC_CTRL_INIT     R_SYSDMAC_RcarDmacCtrlInit
#define RCAR_DMAC_CALLBACK_SET  R_SYSDMAC_RcarCallBackSet
#define RCAR_DMAC_EXEC          R_SYSDMAC_RcarDmacExec

/* I2C base adrress */
#define R_I2C_IF0_BASE      0xc11d0000
#define R_I2C_IF1_BASE      0xc06c0000
#define R_I2C_IF2_BASE      0xc06c8000
#define R_I2C_IF3_BASE      0xc06d0000
#define R_I2C_IF4_BASE      0xc06d8000
#define R_I2C_IF5_BASE      0xc06e0000
#define R_I2C_IF6_BASE      0xc06e8000
#define R_I2C_IF7_BASE      0xc06f0000
#define R_I2C_IF8_BASE      0xc06f8000

#define I2C_CH_NUM          9

static const uint32_t i2c_base[I2C_CH_NUM] =
{
    R_I2C_IF0_BASE,
    R_I2C_IF1_BASE,
    R_I2C_IF2_BASE,
    R_I2C_IF3_BASE,
    R_I2C_IF4_BASE,
    R_I2C_IF5_BASE,
    R_I2C_IF6_BASE,
    R_I2C_IF7_BASE,
    R_I2C_IF8_BASE
};

#ifdef __cplusplus
}
#endif

#endif /* R_I2C_PRIVATE_H */
