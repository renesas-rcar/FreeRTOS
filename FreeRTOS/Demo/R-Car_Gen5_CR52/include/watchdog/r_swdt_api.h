/*************************************************************************************************************
* Copyright (c) [2025] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
* SPDX-License-Identifier: MIT
*************************************************************************************************************/
#ifndef R_SWDT_API_H
#define R_SWDT_API_H

/**
 * @defgroup SWDT_Module SWDT Module
 * @{
 * @brief This module <TDB>
 *
 * System watchdog.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Initialize the Watchdog Timer (WDT) with a specified timeout.
 *
 * @param[in] timeout_sec - Timeout value in seconds. This value will be used to set the
 *                          timeout period for the watchdog timer.
 *
 * This function initializes the watchdog timer with a timeout value in seconds,
 * converting it to the corresponding register value for the WDT.
 *
 * @retval 0 if successful, or a non-zero value if an error occurs.
 */
uint8_t R_SWDT_Init(uint8_t timeout_sec);

/**
 * @brief Reset the watchdog timer with a new timeout and specified number of pings.
 *
 * @param[in] timeout_new_sec - Timeout value in seconds.
 *
 * This function resets the watchdog timer with the specified timeout and
 * triggers the reset operation the specified number of times.
 *
 * @retval 0 if successful.
 */

uint8_t R_SWDT_Ping(uint8_t timeout_sec);

/**
 * @brief Start the watchdog timer.
 *
 * This function starts the watchdog timer, enabling it to monitor the system
 * and trigger a reset if the timeout expires.
 *
 * @retval 0 if successful.
 */
uint32_t R_SWDT_Start();

/**
 * @brief Stop the watchdog timer.
 *
 * @retval 0 if successful.
 *
 * This function stops the watchdog timer, disabling it from monitoring the
 * system and preventing it from triggering a reset.
 */
uint32_t R_SWDT_Stop();

#ifdef __cplusplus
}
#endif

/** @} */ // end of SWDT_Module

#endif /* R_SWDT_API_H */
