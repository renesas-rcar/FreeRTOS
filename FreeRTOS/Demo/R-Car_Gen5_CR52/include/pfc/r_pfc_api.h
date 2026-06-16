/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*************************************************************************************************************/

#ifndef R_PFC_API_H
#define R_PFC_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)

#include "device_tree_x5h.h"
#else   // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)

#include "device_tree_mdp_aiacc.h"
#endif  // (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)

/**
* @defgroup PFC_Module Pin Function Control Module
* @{
* @brief Functions to configure pin functions and pull-up/pull-down settings for hardware modules.
*
* This module provides pin configuration for PFC mode and pull resistor settings.
* It includes initialization of pin groups and setting pins to pull-up, pull-down, or no pull states.
*/

/**
 * @brief PFC groups enumeration.
 * 
 * Defines groups of pins for pin function control.
 */
#if(BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
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
#else   // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
typedef enum e_rcar_pfc_group
{
    RCAR_PFC_GROUP_00 = 0x00,           ///< PFC Group 00
    RCAR_PFC_GROUP_01 = 0x01,           ///< PFC Group 01
    RCAR_PFC_GROUP_02 = 0x02,           ///< PFC Group 02
    RCAR_PFC_GROUP_MAX,
} rcar_pfc_group_t;
#endif

/** 
 * @brief PFC pins enumeration.
 * 
 * Defines pin numbers within each PFC group.
 */
typedef enum e_rcar_pfc_pin
{
    RCAR_PFC_PIN_00 = 0x00,           ///< PFC Pin 00
    RCAR_PFC_PIN_01 = 0x01,           ///< PFC Pin 01
    RCAR_PFC_PIN_02 = 0x02,           ///< PFC Pin 02
    RCAR_PFC_PIN_03 = 0x03,           ///< PFC Pin 03
    RCAR_PFC_PIN_04 = 0x04,           ///< PFC Pin 04
    RCAR_PFC_PIN_05 = 0x05,           ///< PFC Pin 05
    RCAR_PFC_PIN_06 = 0x06,           ///< PFC Pin 06
    RCAR_PFC_PIN_07 = 0x07,           ///< PFC Pin 07
    RCAR_PFC_PIN_08 = 0x08,           ///< PFC Pin 08
    RCAR_PFC_PIN_09 = 0x09,           ///< PFC Pin 09
    RCAR_PFC_PIN_10 = 0x0A,           ///< PFC Pin 10
    RCAR_PFC_PIN_11 = 0x0B,           ///< PFC Pin 11
    RCAR_PFC_PIN_12 = 0x0C,           ///< PFC Pin 12
    RCAR_PFC_PIN_13 = 0x0D,           ///< PFC Pin 13
    RCAR_PFC_PIN_14 = 0x0E,           ///< PFC Pin 14
    RCAR_PFC_PIN_15 = 0x0F,           ///< PFC Pin 15
    RCAR_PFC_PIN_16 = 0x10,           ///< PFC Pin 16
    RCAR_PFC_PIN_17 = 0x11,           ///< PFC Pin 17
    RCAR_PFC_PIN_18 = 0x12,           ///< PFC Pin 18
    RCAR_PFC_PIN_19 = 0x13,           ///< PFC Pin 19
    RCAR_PFC_PIN_20 = 0x14,           ///< PFC Pin 20
    RCAR_PFC_PIN_21 = 0x15,           ///< PFC Pin 21
    RCAR_PFC_PIN_22 = 0x16,           ///< PFC Pin 22
    RCAR_PFC_PIN_23 = 0x17,           ///< PFC Pin 23
    RCAR_PFC_PIN_24 = 0x18,           ///< PFC Pin 24
    RCAR_PFC_PIN_25 = 0x19,           ///< PFC Pin 25
    RCAR_PFC_PIN_26 = 0x1A,           ///< PFC Pin 26
    RCAR_PFC_PIN_27 = 0x1B,           ///< PFC Pin 27
    RCAR_PFC_PIN_28 = 0x1C,           ///< PFC Pin 28
    RCAR_PFC_PIN_29 = 0x1D,           ///< PFC Pin 29
    RCAR_PFC_PIN_30 = 0x1E,           ///< PFC Pin 30
    RCAR_PFC_PIN_31 = 0x1F,           ///< PFC Pin 31
} rcar_pfc_pin_t;

/** 
 * @brief Pin function pull settings.
 * 
 * Specifies the pull resistor configuration for a pin.
 */
typedef enum e_rcar_pfc_functions
{
    RCAR_PFC_PULL_DOWN = 0,         ///< Pull down
    RCAR_PFC_PULL_UP,               ///< Pull up
    RCAR_PFC_NO_PULL                ///< No pull
} rcar_pfc_functions_t;

/**
 * @brief Sets pin function for HW IP.
 * 
 * @param module - HW IP's module config.
 * @return 0 if successful 
 */
int pfcInitModule(st_module_config_t module);

/**
 * @brief Sets pin function for all defined HW IP's modules config.
 * 
 * @param module_list - Pointer to HW IP's module config.
 * @return 0 if successful 
 */
int pfcInitModules(st_module_config_t* module_list);

/**
 * @brief Sets a pin's function to gpio mode.
 * 
 * @param grp - GPIO group, see @ref rcar_pfc_group_t
 * @param pin - GPIO pin, see @ref rcar_pfc_pin_t
 * @return 0 if successful
 */
int pfcSetGPIO(rcar_pfc_group_t grp, rcar_pfc_pin_t pin);

/**
 * @brief Sets a pin's function to pull-up.
 * 
 * @param grp - GPIO group, see @ref rcar_pfc_group_t
 * @param pin - GPIO pin, see @ref rcar_pfc_pin_t
 * @return 0 if successful 
 */
int pfcSetPullUp(rcar_pfc_group_t grp, rcar_pfc_pin_t pin);

/**
 * @brief Sets a pin's function to pull-down.
 * 
 * @param grp - GPIO group, see @ref rcar_pfc_group_t
 * @param pin - GPIO pin, see @ref rcar_pfc_pin_t
 * @return 0 if successful 
 */
int pfcSetPullDown(rcar_pfc_group_t grp, rcar_pfc_pin_t pin);

/**
 * @brief Sets a pin's function to no pull.
 * 
 * @param grp - GPIO group, see @ref rcar_pfc_group_t
 * @param pin - GPIO pin, see @ref rcar_pfc_pin_t
 * @return 0 if successful 
 */
int pfcSetNoPull(rcar_pfc_group_t grp, rcar_pfc_pin_t pin);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* R_PFC_API_H */
