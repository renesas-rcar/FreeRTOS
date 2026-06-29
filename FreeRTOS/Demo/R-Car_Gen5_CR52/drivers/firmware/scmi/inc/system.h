/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SCMI_PROTOCOL_SYSTEM_H
#define SCMI_PROTOCOL_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup SCMI_Protocol_System_Module SCMI Protocol System Module
 * @{
 * @brief Functions to use SCMI system protocol
 */

/**
 * @brief SCMI System Power State Notifier.
 */
typedef struct {
    uint32_t agent_id;
    uint32_t flags;
    uint32_t system_state;
	uint32_t timeout;
} scmi_syspower_state_notifier_t;

/**
 * @brief Graceful flags.
 */
typedef enum {
    FLAGS_FORCEFUL              = 0,    /**< Forceful request */
    FLAGS_GRACEFUL              = 1     /**< Graceful request */
} e_flags_t;

/**
 * @brief SCMI System State.
 */
typedef enum {
    SYSTEM_STATE_SHUTDOWN       = 0,    /**< Shutdown State */
    SYSTEM_STATE_COLD_RESET     ,       /**< Cold Reset State */
    SYSTEM_STATE_WARM_RESET     ,       /**< Warm Reset State */
    SYSTEM_STATE_POWER_UP       ,       /**< Power Up State */
    SYSTEM_STATE_SUSPEND        ,       /**< Suspend State */
    SYSTEM_STATE_MAX                    /**< Number of System States */
} e_system_state_t;

struct scmi_protocol *scmi_system_proto_get(void);

/**
 * @brief This function returns the version of the system protocol
 *
 * @param[out] version Return the version
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int scmi_system_version_get(uint32_t *version);

/**
 * @brief This function returns the attributes of the system protocol
 *
 * @param[out] warm_reset_support Pointer to store the warm reset support
 * @param[out] system_suspend_support Pointer to store the system suspend support
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int scmi_system_message_attributes(bool *warm_reset_support,
								   bool *system_suspend_support);

/**
 * @brief This function enable/disable system nofitication
 *
 * @param[in] enable Enable (true) or Disable (false)
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int scmi_system_request_notify(bool enable);

/**
 * @brief This function returns the system power state
 *
 * @param[out] state Pointer to store the system power state
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int scmi_system_power_state_get(uint32_t *state);

/**
 * @brief This function sets the system power state
 *
 * @param[in] flags Specify setting gracefully or forcefully
 * @param[in] state Power state to set
 *
 * @return 0 if all went fine, else return appropriate error.
 */
int scmi_system_power_state_set(uint32_t flags, uint32_t state);

/** @} */ // end of group SCMI_Protocol_System_Module

#endif // SCMI_PROTOCOL_SYSTEM_H

