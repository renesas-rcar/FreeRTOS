/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup PFC_COMMON_PRIVATE PFC COMMON PRIVATE
 * @{
 * @brief PFC COMMON PRIVATE platform.
 *
 */

#ifndef R_PFC_COMMON_PRVIATE_H
#define R_PFC_COMMON_PRVIATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "device_tree.h"

/* Mask field for enum id: to store PFC information */
#define REG_TYPE_POS        20
#define GROUP_START_POS     12
#define PIN_START_POS       4
#define FUNC_START_POS      0

/** Pin function ids */
typedef enum e_rcar_pfc_func_id
{
    RCAR_PFC_FUNC_0 = 0,            ///< Function 0
    RCAR_PFC_FUNC_1,                ///< Function 1
    RCAR_PFC_FUNC_2,                ///< Function 2
    RCAR_PFC_FUNC_3,                ///< Function 3
    RCAR_PFC_FUNC_4,                ///< Function 4
    RCAR_PFC_FUNC_5,                ///< Function 5
    RCAR_PFC_FUNC_6,                ///< Function 6
    RCAR_PFC_FUNC_7,                ///< Function 7
    RCAR_PFC_FUNC_8,                ///< Function 8
    RCAR_PFC_FUNC_9,                ///< Function 9
    RCAR_PFC_FUNC_10,               ///< Function 10
    RCAR_PFC_FUNC_11,               ///< Function 11
    RCAR_PFC_FUNC_12,               ///< Function 12
    RCAR_PFC_FUNC_13,               ///< Function 13
    RCAR_PFC_FUNC_14,               ///< Function 14
    RCAR_PFC_FUNC_15,               ///< Function 15

    /* Sentinel */
    INVALID_RCAR_PFC_FUNC,
    /* Do not add anything below */
} rcar_pfc_func_id_t;

/* Register PFC ids */
typedef enum e_reg_pfc
{
    REG_ALTSEL = 0,
    REG_DRVCTRL,
    REG_TDSEL,
    REG_MODSEL,

    /* Sentinel */
    INVALID_REG,
    /* Do not add anything below */
} reg_pfc_t;

/* Register PFC ids */
typedef enum e_modsel_func
{
    MODSEL_TAUJ_OUTPUT  = 0,
    MODSEL_TAUJ_INPUT   = 1,
    MODSEL_GPIO_MODE    = 0,
    MODSEL_I2C_MODE     = 1,

    /* Sentinel */
    INVALID_MODSEL,
    /* Do not add anything below */
} modsel_func_t;

#define GEN_ID(reg_pfc_t, grp, pin, fid)   \
(((uint32_t)(reg_pfc_t)<<REG_TYPE_POS) + ((uint32_t)(grp)<<GROUP_START_POS) + ((uint32_t)(pin)<<PIN_START_POS) + ((uint32_t)(fid)<<FUNC_START_POS))

/******************* Define pin functions *******************/
#define MAX_ITEM_IN_GROUP   16
#define CREATE_GROUP(name, ...) \
static int name[MAX_ITEM_IN_GROUP] = {__VA_ARGS__, -1};

typedef struct {
    e_module_id_t module_id;
    int *group;
} st_driver_group_t;

#define ADD_GROUP(name)     name

#ifdef __cplusplus
}
#endif

#endif /* R_PFC_COMMON_PRVIATE_H */
