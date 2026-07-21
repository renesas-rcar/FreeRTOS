/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_STATE_MANAGER_H
#define R_STATE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @defgroup State_Manager State Manager Module
 * @{
 * @brief This module provides typedef to configure State Manager.
 *
 * The State Manager module contains functions which do the following tasks:
 *      ON/OFF System Power
 *      ON/OFF Power Domains
 *      ON/OFF Clock Domains and SET/GET clock rates
 *      ASSERT/DEASSERT Reset Domains
 */

/**
 * @brief Reset Domain Status.
 */
typedef enum 
{
    RESET_DOMAIN_ASSERTED = 0, 
    RESET_DOMAIN_RELEASED = 1
} e_reset_domain_status_t;

/**
 * @brief Power States.
 */
typedef enum {
    POWER_ON = 0,
    POWER_OFF,
} e_power_state_t;

/**
 * @brief This function inits State Manager
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_Init(void);

/**
 * @brief This function shows SCMI information
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_SCMI_Info_Show(void);

/**
 * @brief This function suspends system to RAM (DeepStop mode)
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_RequestDeepStop(void);

/**
 * @brief This function resets the system
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_SysReboot(void);

/**
 * @brief This function shutdowns the system
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_SysPowerOff(void);

/**
 * @brief This function gets power domain state
 *
 * @param[in] domain_id Power Domain ID
 * @param[out] state Current power state can be:
 *                   - POWER_ON
 *                   - POWER_OFF
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_Power_Get(int domain_id, e_power_state_t *state);

/**
 * @brief This function turns power domain off
 *
 * @param[in] domain_id Power Domain ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_PowerOff(int domain_id);

/**
 * @brief This function turns power domain off
 *
 * @param[in] domain_id Power Domain ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_PowerOn(int domain_id);

/**
 * @brief This function sets clock rates
 *
 * @param[in] clock_id Clock ID
 * @param[in] rates An array of two uint32_t values, where:
 *                 - rates[0]: Lower 32 bits of the physical rate in Hertz
 *                 - rates[1]: Upper 32 bits of the physical rate in Hertz
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_SetClock(int clock_id, uint32_t *rates);

/**
 * @brief This function returns current clock rates
 *
 * @param[in]  clock_id Clock ID
 * @param[out] rates An array of two uint32_t values, where:
 *                 - rates[0]: Lower 32 bits of the physical rate in Hertz
 *                 - rates[1]: Upper 32 bits of the physical rate in Hertz
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_GetClock(int clock_id, uint32_t *rates);

/**
 * @brief This function turns off clock
 *
 * @param[in]  clock_id Clock ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_ClockOff(int clock_id);

/**
 * @brief This function turns on clock
 *
 * @param[in]  clock_id Clock ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_ClockOn(int clock_id);

/**
 * @brief This function gets current clock status
 *
 * @param[in]  clock_id Clock ID
 * @param[out] status pointer to status
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_ClockStatusGet(int clock_id, bool *status);

/**
 * @brief This function asserts domain id
 *
 * @param[in]  domain_id Domain ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_ResetAssert(int domain_id);

/**
 * @brief This function deasserts domain id
 *
 * @param[in]  domain_id Domain ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_ResetDeassert(int domain_id);

/**
 * @brief This function resets domain id
 *
 * @param[in]  domain_id Domain ID
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int R_StateManager_Reset(int domain_id);

/**
 * @brief This function get reset status of domain id
 *
 * @param[in]   domain_id Domain ID
 * @param[out]  status Reset status of the Domain ID
 *
 * @return 0 if all went fine, else return appropriate error from SCMI.
 */
int R_StateManager_Reset_Status_Get(int domain_id, e_reset_domain_status_t *status);

#ifdef __cplusplus
}
#endif

/** @} */ // end of State_Manager

#endif /* R_STATE_MANAGER_H */
