/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * @brief SCMI reset protocol helpers
 */

#ifndef SCMI_PROTOCOL_RESET_DOMAIN_H
#define SCMI_PROTOCOL_RESET_DOMAIN_H

#include "util.h"
#include "protocol.h"

/* Common reset macros */
#define NUM_RESET_DOMAIN_MASK	0xffff
#define RESET_NOTIFY_ENABLE	BIT(0)

/* Reset domain attributes */
#define RESET_DOMAIN_ATTR_ASYNC(x)	((x) & BIT(31))
#define RESET_DOMAIN_ATTR_NOTIF(x)	((x) & BIT(30))
#define RESET_DOMAIN_ATTR_EXTEND(x)	((x) & BIT(29))

#define RESET_DOMAIN_ATTR_LATENCY_UNSUPPORTED	(0xFFFFFFFFU)

/* Reset domain request flags */
#define RESET_DOMAIN_FLAGS_AUTO		BIT(0)
#define RESET_DOMAIN_FLAGS_EXPLICIT	BIT(1)
#define RESET_DOMAIN_FLAGS_ASYNC	BIT(2)
#define RESET_DOMAIN_FLAGS_MASK		(RESET_DOMAIN_FLAGS_AUTO | \
									 RESET_DOMAIN_FLAGS_EXPLICIT | \
									 RESET_DOMAIN_FLAGS_ASYNC)

/**
 * @struct scmi_reset_request_config
 *
 * @brief Describes the parameters for the RESET
 * command
 */
struct scmi_reset_domain_request_config {
	uint32_t domain_id;
	uint32_t flags;
	uint32_t reset_state;
};

/**
 * @brief Reset protocol version get
 *
 * @param version pointer to version
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_reset_version_get(uint32_t *version);

/**
 * @brief Send the PROTOCOL_ATTRIBUTES command and get its reply
 *
 * @param attributes pointer to attributes to be set via
 * this command
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_reset_protocol_attributes(uint32_t *attributes);

/**
 * @brief Send the RESET_DOMAIN_ATTRIBUTES command and get its reply
 *
 * @param domain_id domain id to get attributes
 * @param attributes pointer to attributes of the requested
 * domain id
 * @param latency pointer to latency of the requested domain id
 * @param name pointer to name of the requested domain id
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_reset_domain_attributes(uint32_t domain_id, uint32_t *attributes,
								 uint32_t *latency, uint8_t *name);

/**
 * @brief Send the RESET command and get its reply
 *
 * @param cfg pointer to structure containing configuration
 * to be set
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_reset_domain_request(struct scmi_reset_domain_request_config cfg);

#endif /* SCMI_PROTOCOL_RESET_DOMAIN_H */
