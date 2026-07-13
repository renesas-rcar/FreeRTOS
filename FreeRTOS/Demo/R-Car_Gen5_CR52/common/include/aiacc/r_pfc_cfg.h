/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup R_PFC_CFG PFC CONFIG
 * @{
 * @brief PFC CONFIG of AIACC platform.
 *
 */
#ifndef R_PFC_CFG_H
#define R_PFC_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief PFC groups enumeration.
 *
 * Defines groups of pins for pin function control.
 */
typedef enum e_rcar_pfc_group
{
    RCAR_PFC_GROUP_00 = 0x00,           ///< PFC Group 00
    RCAR_PFC_GROUP_01 = 0x01,           ///< PFC Group 01
    RCAR_PFC_GROUP_02 = 0x02,           ///< PFC Group 02
    RCAR_PFC_GROUP_MAX,
} rcar_pfc_group_t;

#ifdef __cplusplus
}
#endif

/** @} */ // end of R_PFC_CFG

#endif /* R_PFC_CFG_H */
