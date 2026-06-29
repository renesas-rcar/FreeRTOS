/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Base protocol describes the properties of the implementation
 * and provides generic error management.
 */
#ifndef SCMI_PROTOCOL_BASE_H
#define SCMI_PROTOCOL_BASE_H

/**
 * @defgroup SCMI_Protocol_Base_Module SCMI Protocol Base Module
 * @{
 * @brief Functions to use SCMI Base protocol
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief This function return the version of the base protocol
 * 
 * @param[out] version Return the version
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_version_get(uint32_t *version);

/**
 * @brief This function return the attributes of the base protocol
 * 
 * @param[out] num_protocol Return the number of protocol implemented, exclude Base protocol
 * @param[out] num_agent Return the number of agent in the system
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_attributes_get(uint8_t *num_protocols, uint8_t *num_agents);

/**
 * @brief This function provides the Vendor identifier in an ASCII string
 * 
 * @param[in] sub_vendor specify true if sub-vendor ID is needed
 * @param[out] vendor_id The ASCII string of Vendor ID, upto- 16 bytes.
 * 		Null character terminates the string.
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_vendorid_get(bool sub_vendor, char *vendor_id);

/**
 * @brief This function provides a vendor-specific 32-bit implementation version.
 * 
 * @param[out] impl_version Implementation version defined by Renesas
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_implementation_version_get(uint32_t *impl_version);

/**
 * @brief This function allow agent to discover which protocol is allowed to access.
 * 
 * @param[out] num_protocols Number of protocol can be accessed
 * @param[out] protocols Array to contain supported protocol ID excluding
 *		Base protocol. The list of ID is in numeric ascending order.
 *      4 protocol IDs are packed in an array element.
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_discover_list_protocols(uint32_t *num_protocols,
                                      uint8_t **protocols);

/**
 * @brief scmi_base_discover_agent_get() - discover the name of an agent 
 * 
 * @param[in] request_agent_id Agent id to be requested
 * @param[out] agent_id ID of the agent whose identity is found.
 *      This value is the ID of the called agent, if the `request_agent_id`
 *		is 0xFFFFFFFF. In all other cases, it is identical to `request_agent_id`
 * @param[out] name The NULL-terminated string with 16-byte maximum.
 *
 * @return 0 if all went fine, else return appropriate error. 
 */
int scmi_base_discover_agent_get(uint32_t request_agent_id,
								 uint32_t *agent_id, uint8_t *name);

/** @} */ // end of group SCMI_PROTOCOL_BASE_API

#endif // SCMI_PROTOCOL_BASE_H

