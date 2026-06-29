/*************************************************************************************************************
* Copyright (c) [2025] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
* SPDX-License-Identifier: MIT
*************************************************************************************************************/
#ifndef R_WWDT_API_H
#define R_WWDT_API_H

/**
 * @defgroup WWDT_Module WWDT Module
 * @{
 * @brief Window watchdog
 *
 * Window watchdog
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#pragma once
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @brief List of WWDT channels.
 */
typedef enum {
    R_WWDT0 = 0,    ///< Watchdog Timer Unit 0
    R_WWDT1,        ///< Watchdog Timer Unit 1
    R_WWDT2,        ///< Watchdog Timer Unit 2
    R_WWDT3,        ///< Watchdog Timer Unit 3
    R_WWDT4,        ///< Watchdog Timer Unit 4
    R_WWDT5,        ///< Watchdog Timer Unit 5
    R_WWDT6,        ///< Watchdog Timer Unit 6
    R_WWDT7,        ///< Watchdog Timer Unit 7
    R_WWDT8,        ///< Watchdog Timer Unit 8
    R_WWDT9,        ///< Watchdog Timer Unit 9
    R_WWDT10,       ///< Watchdog Timer Unit 10
    R_WWDT11,       ///< Watchdog Timer Unit 11
    R_WWDT12,       ///< Watchdog Timer Unit 12
    R_WWDT13,       ///< Watchdog Timer Unit 13
    R_WWDT14,       ///< Watchdog Timer Unit 14
    R_WWDT15,       ///< Watchdog Timer Unit 15
    R_WWDT16,       ///< Watchdog Timer Unit 16
    R_WWDT17,       ///< Watchdog Timer Unit 17
    R_WWDT18,       ///< Watchdog Timer Unit 18
    R_WWDT19,       ///< Watchdog Timer Unit 19
    R_WWDT20,       ///< Watchdog Timer Unit 20
    R_WWDT_LAST		///< Delimiter indicating the end of the Watchdog Timer Unit enumeration
} wwdt_unit_t;

/**
 * @brief Enum representing the window size percentage for the Watchdog Timer.
 */
typedef enum {
    WINDOW_25P = 0,     ///< 25% window size for the Watchdog Timer
    WINDOW_50P = 1,     ///< 50% window size for the Watchdog Timer
    WINDOW_75P = 2,     ///< 75% window size for the Watchdog Timer
    WINDOW_100P = 3     ///< 1000% window size for the Watchdog Timer
} wwdt_wsize_t;

/**
 * @brief Enum representing the error recovery mode for the Watchdog Timer.
 */
typedef enum {
    ERM_NMI_MODE = 0,   ///< Non-Maskable Interrupt (NMI) mode for error recovery
    ERM_RESET_MODE = 1  ///< Reset mode for error recovery
} wwdt_erm_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Initialize the Windowed Watchdog Timer (WWDT) with the specified configuration.
 *
 * @param[in] unit         - WWDT unit number, see @ref r_wwdt_Unit_t for available units.
 * @param[in] wsize        - Window size value, which defines the percentage of open window.
 *                           Valid values:
 *                           - WINDOW_25P:  25% of open window
 *                           - WINDOW_50P:  50% of open window
 *                           - WINDOW_75P:  75% of open window
 *                           - WINDOW_100P:  100% of open window
 * @param[in] timeout_msec - Timeout value in milliseconds. This determines the time before the WWDT triggers.
 *                           For WWDT0-19, the timeout values are:
 *                           - 15ms, 31ms, 62ms, 124ms, 250ms, 500ms, 1000ms, and 2000ms.
 *                           For WWDT20, the timeout values are:
 *                           - 2ms, 4ms, 8ms, 17ms, 34ms, 68ms, 136ms, and 273ms.
 * @param[in] irq_75p      - Enable or disable the 75% interrupt function. Set to `true` to enable, `false` to disable.
 * @param[in] err_mode     - Error handling mode. Choose how to handle WWDT errors:
 *                           - `true` (default): Reset mode, the system will reset when an error occurs.
 *                           - `false`: NMI mode, the system will generate a Non-Maskable Interrupt (NMI) on error.
 *
 */
void R_WWDT_Init(wwdt_unit_t unit, wwdt_wsize_t wsize, uint32_t timeout_msec, bool irq_75p, wwdt_erm_t err_mode);

/**
 * @brief Refresh the Windowed Watchdog Timer (WWDT) to prevent it from triggering a reset or NMI.
 *
 * @param[in] unit - WWDT unit number, see @ref r_wwdt_Unit_t for available units. This defines the channel
 *                  that needs to be refreshed.
 *
 * @retval 0 if successful, a non-zero value if there is an error.
 */
uint32_t R_WWDT_Refresh(wwdt_unit_t unit);

#ifdef __cplusplus
}
#endif

/** @} */ // end of GPIO_Module

#endif /* R_WWDT_API_H */
