/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup R_PFC_CFG PFC CONFIG
 * @{
 * @brief PFC CONFIG of MDP X5H platform.
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
    RCAR_PFC_GROUP_03 = 0x03,           ///< PFC Group 03
    RCAR_PFC_GROUP_04 = 0x04,           ///< PFC Group 04
    RCAR_PFC_GROUP_05 = 0x05,           ///< PFC Group 05
    RCAR_PFC_GROUP_06 = 0x06,           ///< PFC Group 06
    RCAR_PFC_GROUP_07 = 0x07,           ///< PFC Group 07
    RCAR_PFC_GROUP_08 = 0x08,           ///< PFC Group 08
    RCAR_PFC_GROUP_09 = 0x09,           ///< PFC Group 09
    RCAR_PFC_GROUP_10 = 0x0A,           ///< PFC Group 10
    RCAR_PFC_GROUP_MAX,
} rcar_pfc_group_t;

#ifdef __cplusplus
}
#endif

/** @} */ // end of R_PFC_CFG

#endif /* R_PFC_CFG_H */
