/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef _DEVICE_TREE_H_
#define _DEVICE_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include"board.h"

#if(BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
#include "device_tree_x5h.h"
#else   // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
#include "device_tree_mdp_aiacc.h"
#endif

/**
 * @brief Configuration PFC for module HW IP.
 */
typedef struct {
    e_module_id_t module_id;    ///< Module id.
    uint32_t is_enabled;        ///< Enable PFC for module or not.
} st_module_config_t;

static inline st_module_config_t* getModuleConfigs() {
    static st_module_config_t MODULE_CONFIG_LIST[] = {
        MODULE_CONFIGS
    };

    return MODULE_CONFIG_LIST;
}

#ifdef __cplusplus
}
#endif

#endif // _DEVICE_TREE_H_
