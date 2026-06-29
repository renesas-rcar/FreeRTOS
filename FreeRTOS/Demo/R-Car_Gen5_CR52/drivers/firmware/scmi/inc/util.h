/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ARM SCMI utility header
 *
 * Contains various utility macros and macros used for protocol and
 * transport "registration".
 */

#ifndef SCMI_UTIL_H
#define SCMI_UTIL_H

#include <stdint.h>

struct scmi_protocol;

#undef BIT
/**
 * @brief BIT
 */
#define BIT(nr)                   (1UL << (nr))

/** @brief Cast @p x, a pointer, to an unsigned integer. */
#define POINTER_TO_UINT(x) ((uintptr_t) (x))

#define CONCAT(a, b) a ## b
#define CONCAT3(a, b, c) a ## b ## c
#define CONCAT4(a, b, c, d) a ## b ## c ## d

/**
 * @brief Build protocol name from its ID
 *
 * Given a protocol ID, this macro builds the protocol
 * name. This is done by concatenating the scmi_protocol_
 * construct with the given protocol ID.
 *
 * @param proto protocol ID in decimal format
 *
 * @return protocol name
 */
#define SCMI_PROTOCOL_NAME(proto) CONCAT(scmi_protocol_, proto)

#define STRUCT_SECTION(struct_type, varname) \
	struct struct_type varname

/** @brief Check if a protocol node has an associated channel
 *
 * This macro, when applied to a protocol node, checks if
 * the node has a dedicated static channel allocated to it.
 * This definition is specific to the mailbox driver and
 * each new transport layer driver should define its own
 * version of this macro based on the devicetree properties
 * that indicate the presence of a dedicated channel.
 *
 * @param node_id protocol node identifier
 * @idx channel index. Should be 0 for TX channels and 1 for
 * RX channels
 */
#define SCMI_TRANSPORT_PROTO_HAS_CHAN(node_id, idx)\
	PROP_HAS_IDX(node_id, shmem, idx)

#define SCMI_TRANSPORT_CHAN_NAME(proto, idx) CONCAT4(scmi_channel_, proto, _, idx)

/**
 * @brief Declare a TX SCMI channel
 *
 * Given a node_id for a protocol, this macro declares the SCMI
 * TX channel statically bound to said protocol via the "extern"
 * qualifier. This is useful when the transport layer driver
 * supports static channels since all channel structures are
 * defined inside the transport layer driver.
 *
 * @param node_id protocol node identifier
 */
#define SCMI_TRANSPORT_TX_CHAN_DECLARE()				\
		    extern struct scmi_channel					\
		     SCMI_TRANSPORT_CHAN_NAME(SCMI_PROTOCOL_BASE, 0);		\

/**
 * @brief Declare SCMI TX/RX channels
 *
 * Given a node_id for a protocol, this macro declares the
 * SCMI TX and RX channels statically bound to said protocol via
 * the "extern" qualifier. Since RX channels are currently not
 * supported, this is equivalent to SCMI_TRANSPORT_TX_CHAN_DECLARE().
 * Despite this, users should opt for this macro instead of the TX-specific
 * one.
 *
 * @param node_id protocol node identifier
 */
#define SCMI_TRANSPORT_CHANNELS_DECLARE()				\
	SCMI_TRANSPORT_TX_CHAN_DECLARE()				\

/**
 * @brief Get a reference to a protocol's SCMI TX channel
 *
 * Given a node_id for a protocol, this macro returns a
 * reference to an SCMI TX channel statically bound to said
 * protocol.
 *
 * @param node_id protocol node identifier
 *
 * @return reference to the struct scmi_channel of the TX channel
 * bound to the protocol identifier by node_id
 */
#define SCMI_TRANSPORT_TX_CHAN() \
		    &SCMI_TRANSPORT_CHAN_NAME(SCMI_PROTOCOL_BASE, 0)

/**
 * @brief Define an SCMI channel for a protocol
 *
 * This macro defines a struct scmi_channel for a given protocol.
 * This should be used by the transport layer driver to statically
 * define SCMI channels for the protocols.
 *
 * @param node_id protocol node identifier
 * @param idx channel index. Should be 0 for TX channels and 1
 * for RX channels
 * @param proto protocol ID in decimal format
 */
#define SCMI_TRANSPORT_CHAN_DEFINE(idx, proto, pdata)		\
	STRUCT_SECTION(scmi_channel, SCMI_TRANSPORT_CHAN_NAME(proto, idx)) = \
	{									\
		.data = pdata,							\
	}

extern struct scmi_dev transport_dev;
#define TRANSPORT_GET() &transport_dev
/**
 * @brief Define an SCMI protocol's data
 *
 * Each SCMI protocol is identified by a struct scmi_protocol
 * placed in a linker section called scmi_protocol. Each protocol
 * driver is required to use this macro for "registration". Using
 * this macro directly is higly discouraged and users should opt
 * for macros such as SCMI_PROTOCOL_DEFINE_NODEV() or
 * SCMI_PROTOCOL_DEFINE(), which also takes care of the static
 * channel declaration (if applicable).
 *
 * @param node_id protocol node identifier
 * @param proto protocol ID in decimal format
 * @param pdata protocol private data
 */
#define SCMI_PROTOCOL_DATA_DEFINE(proto, pdata)			\
	STRUCT_SECTION(scmi_protocol, SCMI_PROTOCOL_NAME(proto)) = \
	{									\
		.id = proto,							\
		.tx = SCMI_TRANSPORT_TX_CHAN(),			\
		.data = pdata,							\
		.transport = TRANSPORT_GET() \
	}

/**
 * @brief Define an SCMI protocol with no device
 *
 * Variant of SCMI_PROTOCOL_DEFINE(), but no `struct device` is
 * created and no initialization function is called during system
 * initialization. This is useful for protocols that are not really
 * part of a subsystem with an API (e.g: pinctrl).
 *
 * @param node_id protocol node identifier
 * @param data protocol private data
 */
#define SCMI_PROTOCOL_DEFINE_NODEV(proto, data)	\
	SCMI_TRANSPORT_CHANNELS_DECLARE()			\
	SCMI_PROTOCOL_DATA_DEFINE(proto, data)

#endif /* SCMI_UTIL_H */

