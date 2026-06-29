/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief SCMI protocol generic functions and structures
 */

#ifndef SCMI_PROTOCOL_H
#define SCMI_PROTOCOL_H

#include <stdint.h>
#include <errno.h>
#include "common.h"

#define CHAR_BIT 8
#define BITS_PER_LONG (sizeof(long) * CHAR_BIT)

/**
 * @brief Create a contiguous bitmask starting at bit position @p l
 *        and ending at position @p h.
 */
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/**
 * @brief Create an SCMI message field
 *
 * Data might not necessarily be encoded in the first
 * x bits of an SCMI message parameter/return value.
 * This comes in handy when building said parameters/
 * return values.
 *
 * @param x value to encode
 * @param mask value to perform bitwise-and with `x`
 * @param shift value to left-shift masked `x`
 */
#define SCMI_FIELD_MAKE(x, mask, shift)\
	(((uint32_t)(x) & (mask)) << (shift))

/**
 * @brief Build an SCMI message header
 *
 * Builds an SCMI message header based on the
 * fields that make it up.
 *
 * @param id message ID
 * @param type message type
 * @param proto protocol ID
 * @param token message token
 */
#define SCMI_MESSAGE_HDR_MAKE(id, type, proto, token)	\
	(SCMI_FIELD_MAKE(id, GENMASK(7, 0), 0)     |	\
	 SCMI_FIELD_MAKE(type, GENMASK(1, 0), 8)   |	\
	 SCMI_FIELD_MAKE(proto, GENMASK(7, 0), 10) |	\
	 SCMI_FIELD_MAKE(token, GENMASK(9, 0), 18))

/**
 * @brief Parse fields from an SCMI message
 *
 */
#define SCMI_FIELD_PARSE(hdr, mask, shift) \
	(((uint32_t)(hdr) >> (shift)) & mask)
#define SCMI_MESSAGE_ID_PARSE(hdr) \
	SCMI_FIELD_PARSE(hdr, GENMASK(7, 0), 0)

#define SCMI_MESSAGE_TYPE_PARSE(hdr) \
	SCMI_FIELD_PARSE(hdr, GENMASK(1, 0), 8)

#define SCMI_MESSAGE_PROTO_PARSE(hdr) \
	SCMI_FIELD_PARSE(hdr, GENMASK(7, 0), 10)

#define SCMI_MESSAGE_TOKEN_PARSE(hdr) \
	SCMI_FIELD_PARSE(hdr, GENMASK(9, 0), 18)

struct scmi_channel;

/**
 * @brief SCMI message type
 */
enum scmi_message_type {
	/** command message */
	SCMI_COMMAND = 0x0,
	/** delayed reply message */
	SCMI_DELAYED_REPLY = 0x2,
	/** notification message */
	SCMI_NOTIFICATION = 0x3,
};

/**
 * @brief SCMI status codes
 */
enum scmi_status_code {
	SCMI_SUCCESS = 0,
	SCMI_NOT_SUPPORTED = -1,
	SCMI_INVALID_PARAMETERS = -2,
	SCMI_DENIED = -3,
	SCMI_NOT_FOUND = -4,
	SCMI_OUT_OF_RANGE = -5,
	SCMI_BUSY = -6,
	SCMI_COMMS_ERROR = -7,
	SCMI_GENERIC_ERROR = -8,
	SCMI_HARDWARE_ERROR = -9,
	SCMI_PROTOCOL_ERROR = -10,
	SCMI_IN_USE = -11,
};

/**
 * @brief SCMI protocol IDs
 *
 * Each SCMI protocol is identified by an ID. Each
 * of these IDs needs to be in decimal since they
 * might be used to build protocol and static channel
 * names.
 */
#define SCMI_PROTOCOL_BASE 16
#define SCMI_PROTOCOL_POWER_DOMAIN 17
#define SCMI_PROTOCOL_SYSTEM 18
#define SCMI_PROTOCOL_PERF 19
#define SCMI_PROTOCOL_CLOCK 20
#define SCMI_PROTOCOL_SENSOR 21
#define SCMI_PROTOCOL_RESET_DOMAIN 22
#define SCMI_PROTOCOL_VOLTAGE_DOMAIN 23
#define SCMI_PROTOCOL_PCAP_MONITOR 24
#define SCMI_PROTOCOL_PINCTRL 25
#define SCMI_PROTOCOL_VENDOR 128 // 0x80

#define MAX_SCMI_PROTOCOLS \
	(SCMI_PROTOCOL_PINCTRL - SCMI_PROTOCOL_BASE + 1)

/**
 * @struct scmi_protocol
 *
 * @brief SCMI protocol structure
 */
struct scmi_protocol {
	/** protocol ID */
	uint32_t id;
	/** TX channel */
	struct scmi_channel *tx;
	/** transport layer device */
	const struct scmi_dev *transport;
	/** protocol private data */
	void *data;
};

/**
 * @struct scmi_message
 *
 * @brief SCMI message structure
 */
struct scmi_message {
	uint32_t hdr;
	uint32_t len;
	void *content;
};

/**
 * @brief Convert an SCMI status code to its Linux equivalent (if possible)
 *
 * @param scmi_status SCMI status code as shown in `enum scmi_status_code`
 *
 * @retval Linux equivalent status code
 */
int scmi_status_to_errno(int scmi_status);

/**
 * @brief Send an SCMI message and wait for its reply
 *
 * Blocking function used to send an SCMI message over
 * a given channel and wait for its reply
 *
 * @param proto pointer to SCMI protocol
 * @param msg pointer to SCMI message to send
 * @param reply pointer to SCMI message in which the reply is to be
 * written
 *
 * @retval 0 if successful
 * @retval negative errno if failure
 */
int scmi_send_message(struct scmi_protocol *proto,
		      struct scmi_message *msg, struct scmi_message *reply);

#endif /* SCMI_PROTOCOL_H */
