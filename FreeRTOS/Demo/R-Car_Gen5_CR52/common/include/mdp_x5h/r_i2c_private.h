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
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"
#include "interrupt_id.h"

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

typedef enum {
    R_I2C_IF0 = 0,  /**< channel 0 */
    R_I2C_IF1,      /**< channel 1 */
    R_I2C_IF2,      /**< channel 2 */
    R_I2C_IF3,      /**< channel 3 */
    R_I2C_IF4,      /**< channel 4 */
    R_I2C_IF5,      /**< channel 5 */
    R_I2C_IF6,      /**< channel 6 */
    R_I2C_IF7,      /**< channel 7 */
    R_I2C_IF8,      /**< channel 8 */
    R_I2C_LAST      /**< delimiter */
} r_i2c_Unit_t;

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

static const uint32_t i2c_clock_domain_id[] =
{
    X5H_CLOCK_ID_MDLC_I2C0,
    X5H_CLOCK_ID_MDLC_I2C1,
    X5H_CLOCK_ID_MDLC_I2C2,
    X5H_CLOCK_ID_MDLC_I2C3,
    X5H_CLOCK_ID_MDLC_I2C4,
    X5H_CLOCK_ID_MDLC_I2C5,
    X5H_CLOCK_ID_MDLC_I2C6,
    X5H_CLOCK_ID_MDLC_I2C7,
    X5H_CLOCK_ID_MDLC_I2C8
};

static const uint32_t i2c_reset_domain_id[] =
{
    X5H_RESET_DOMAIN_ID_I2C0,
    X5H_RESET_DOMAIN_ID_I2C1,
    X5H_RESET_DOMAIN_ID_I2C2,
    X5H_RESET_DOMAIN_ID_I2C3,
    X5H_RESET_DOMAIN_ID_I2C4,
    X5H_RESET_DOMAIN_ID_I2C5,
    X5H_RESET_DOMAIN_ID_I2C6,
    X5H_RESET_DOMAIN_ID_I2C7,
    X5H_RESET_DOMAIN_ID_I2C8
};

static const uint32_t i2c_int_id[] =
{
    INTID_I2C_IF0,
    INTID_I2C_IF1,
    INTID_I2C_IF2,
    INTID_I2C_IF3,
    INTID_I2C_IF4,
    INTID_I2C_IF5,
    INTID_I2C_IF6,
    INTID_I2C_IF7,
    INTID_I2C_IF8
};

#ifdef __cplusplus
}
#endif

#endif /* R_I2C_PRIVATE_H */
