/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup I2C_PRIVATE I2C PRIVATE
 * @{
 * @brief Base address of I2C AIACC platform.
 *
 */

#ifndef R_I2C_PRIVATE_H
#define R_I2C_PRIVATE_H

#include "stdint.h"
#include "dmac/rtdmac_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* I2C AIACC uses RTDMA */
#define RCAR_DMAC_CTRL_INIT     R_RTDMAC_RcarDmacCtrlInit
#define RCAR_DMAC_CALLBACK_SET  R_RTDMAC_RcarCallBackSet
#define RCAR_DMAC_EXEC          R_RTDMAC_RcarDmacExec

/* I2C base adrress */
#define R_I2C_IF0_BASE      0x38040000
#define R_I2C_IF1_BASE      0x38048000
#define R_I2C_IF2_BASE      0x38050000
#define R_I2C_IF3_BASE      0x38058000

#define I2C_CH_NUM          4 

static const uint32_t i2c_base[I2C_CH_NUM] =
{
    R_I2C_IF0_BASE,
    R_I2C_IF1_BASE,
    R_I2C_IF2_BASE,
    R_I2C_IF3_BASE
};

#ifdef __cplusplus
}
#endif

#endif /* R_I2C_PRIVATE_H */
