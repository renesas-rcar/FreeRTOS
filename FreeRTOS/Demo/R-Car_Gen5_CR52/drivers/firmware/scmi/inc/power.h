/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCMI_PROTOCOL_POWER_H
#define SCMI_PROTOCOL_POWER_H

/**
 * @defgroup SCMI_Protocol_Power_Module SCMI Protocol Power Module
 * @{
 * @brief SCMI power domain protocol helpers
 */

#include "util.h"
#include <stdint.h>

/**
 * @brief Power State Set Flag Mask.
 */
#define SCMI_POWER_STATE_SET_FLAG_MASK  (uint32_t)(1UL)
/**
 * @brief SCMI power state set flags async.
 */
#define SCMI_POWER_STATE_SET_FLAGS_ASYNC BIT(0)

/**
 * @brief Power domain on state.
 */
#define SCMI_POWER_STATE_ON  (0x00000000U)
/**
 * @brief Power domain off state.
 */
#define SCMI_POWER_STATE_OFF (0x40000000U)

/**
 * @brief Describes the parameters for the POWER_STATE_SET command
 */
struct scmi_power_state_config {
	uint32_t flags;
	uint32_t domain_id;
	uint32_t power_state;
};

/**
 * @brief Power domain protocol command message IDs
 */
enum scmi_power_domain_message {
	SCMI_POWER_DOMAIN_MSG_PROTOCOL_VERSION = 0x0,
	SCMI_POWER_DOMAIN_MSG_PROTOCOL_ATTRIBUTES = 0x1,
	SCMI_POWER_DOMAIN_MSG_PROTOCOL_MESSAGE_ATTRIBUTES = 0x2,
	SCMI_POWER_DOMAIN_MSG_POWER_DOMAIN_ATTRIBUTES = 0x3,
	SCMI_POWER_DOMAIN_MSG_POWER_STATE_SET = 0x4,
	SCMI_POWER_DOMAIN_MSG_POWER_STATE_GET = 0x5,
	SCMI_POWER_DOMAIN_MSG_POWER_STATE_NOTIFY = 0x6,
	SCMI_POWER_DOMAIN_MSG_POWER_STATE_CHANGE_REQEUSTED_NOTIFY = 0x7,
	SCMI_POWER_DOMAIN_MSG_POWER_DOMAIN_NAME_GET = 0x8,
	SCMI_POWER_DOMAIN_MSG_NEGOTIATE_PROTOCOL_VERSION = 0x10,
};

/**
 * @brief Power version get
 *
 * @param version pointer to version
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_power_version_get(uint32_t *version);

/**
 * @brief Send the PROTOCOL_ATTRIBUTES command and get its reply
 *
 * @param attributes pointer to attributes to be set via
 * this command
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_power_protocol_attributes(uint32_t *attributes);

/**
 * @brief Send the POWER_STATE_SET command and get its reply
 *
 * @param cfg pointer to structure containing configuration
 * to be set
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_power_state_set(struct scmi_power_state_config *cfg);

/**
 * @brief Query the power domain state
 *
 * @param cfg pointer to structure containing configuration
 * to be retrieved
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_power_state_get(struct scmi_power_state_config *cfg);

/** @} */ // end of SCMI_Protocol_Power_Module

#endif /* SCMI_PROTOCOL_POWER_H */

