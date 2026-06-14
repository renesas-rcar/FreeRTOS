/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef _DEVICE_TREE_MDP_AIACC_H_
#define _DEVICE_TREE_MDP_AIACC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "board.h"

#define MODULE_CONFIG(module_id, is_enabled)    {module_id, is_enabled}

/**
 * @brief Superset list of all HW IPs.
 */
typedef enum {
    MODULE_INVALID = 0,     ///< Invalid module ID.
    MODULE_SCIF0,           ///< Module ID: SCIF0.
    MODULE_SCIF1,           ///< Module ID: SCIF1.
    MODULE_HSCIF0,          ///< Module ID: HSCIF0.
    MODULE_HSCIF1,          ///< Module ID: HSCIF1.
    MODULE_I2C0,            ///< Module ID: I2C0.
    MODULE_I2C1,            ///< Module ID: I2C1.
    MODULE_I2C2,            ///< Module ID: I2C2.
    MODULE_I2C3,            ///< Module ID: I2C3.
    // Add device_id
    MODULE_MAX = 255        ///< Max module ID.
} e_module_id_t;

#define MODULE_CONFIGS \
    MODULE_CONFIG(MODULE_SCIF1, 1), \
    MODULE_CONFIG(MODULE_I2C0,  1), \
    MODULE_CONFIG(MODULE_I2C1,  1), \
    MODULE_CONFIG(MODULE_I2C2,  1), \
    MODULE_CONFIG(MODULE_I2C3,  1), \
    /* Add module configs here */   \
    /* Sentinel guard: Do not remove */ \
    MODULE_CONFIG(MODULE_INVALID, 0) \

#ifdef __cplusplus
}
#endif

#endif // _DEVICE_TREE_MDP_AIACC_H_
