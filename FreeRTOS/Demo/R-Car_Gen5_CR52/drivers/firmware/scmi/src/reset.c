/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <string.h>
#include "scmi/inc/protocol.h"
#include "scmi/inc/util.h"
#include "scmi/inc/common.h"
#include "scmi/inc/reset.h"

SCMI_PROTOCOL_DEFINE_NODEV(SCMI_PROTOCOL_RESET_DOMAIN, NULL);

enum reset_protocol_cmd {
	RESET_DOMAIN_ATTRIBUTES = 0x3,
	RESET = 0x4,
	RESET_NOTIFY = 0x5,
	RESET_DOMAIN_NAME_GET = 0x6,
};

struct reset_protocol_attributes {
	int32_t status;
	uint32_t attributes;
};

struct reset_domain_attributes {
	int32_t status;
	uint32_t attributes;
	uint32_t latency;
	uint8_t name[SCMI_SHORT_NAME_MAX_SIZE];
};

struct reset_notify {
	uint32_t id;
	uint32_t event_control;
};

struct reset_issued_notify_payld {
	uint32_t agent_id;
	uint32_t domain_id;
	uint32_t reset_state;
};

struct reset_domain_info {
	bool async_reset;
	bool reset_notify;
	uint32_t latency_us;
	char name[SCMI_MAX_STR_SIZE];
};

struct reset_protocol_info {
	uint32_t version;
	int num_domains;
	struct reset_domain_info *dom_info;
};

int scmi_reset_version_get(uint32_t *version)
{
	struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_RESET_DOMAIN);
	struct scmi_msg_resp_version reply_buffer;
	struct scmi_message msg, reply;
	int ret;

	/* sanity checks */
	if (!version) {
		return -EINVAL;
	}

	if (proto->id != SCMI_PROTOCOL_RESET_DOMAIN) {
		return -EINVAL;
	}

	msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_PROTOCOL_VERSION, SCMI_COMMAND,
					proto->id, 0x0);
	msg.len = 0;
	msg.content = NULL;

	reply.hdr = msg.hdr;
	reply.len = sizeof(reply_buffer);
	reply.content = &reply_buffer;

	ret = scmi_send_message(proto, &msg, &reply);
	if (ret < 0) {
		return ret;
	}

	if (reply_buffer.status != SCMI_SUCCESS) {
		return scmi_status_to_errno(reply_buffer.status);
	}

	*version = (uint32_t)reply_buffer.version;

	return 0;
}

int scmi_reset_protocol_attributes(uint32_t *attributes)
{
	struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_RESET_DOMAIN);
	struct reset_protocol_attributes reply_buffer;
	struct scmi_message msg, reply;
	int ret;

	/* sanity checks */
	if (!attributes) {
		return -EINVAL;
	}

	if (proto->id != SCMI_PROTOCOL_RESET_DOMAIN) {
		return -EINVAL;
	}

	msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_PROTOCOL_ATTRIBUTES, SCMI_COMMAND,
					proto->id, 0x0);
	msg.len = 0;
	msg.content = NULL;

	reply.hdr = msg.hdr;
	reply.len = sizeof(reply_buffer);
	reply.content = &reply_buffer;

	ret = scmi_send_message(proto, &msg, &reply);
	if (ret < 0) {
		return ret;
	}

	if (reply_buffer.status != SCMI_SUCCESS) {
		return scmi_status_to_errno(reply_buffer.status);
	}

	*attributes = (uint32_t)reply_buffer.attributes;

	return 0;
}

int scmi_reset_domain_attributes(uint32_t domain_id, uint32_t *attributes,
								 uint32_t *latency, uint8_t *name)
{
	(void)domain_id;
	struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_RESET_DOMAIN);
	struct reset_domain_attributes reply_buffer;
	struct scmi_message msg, reply;
	int ret;

	/* sanity checks */
	if (!attributes || !latency || !name) {
		return -EINVAL;
	}

	if (proto->id != SCMI_PROTOCOL_RESET_DOMAIN) {
		return -EINVAL;
	}

	msg.hdr = SCMI_MESSAGE_HDR_MAKE(SCMI_PROTOCOL_ATTRIBUTES, SCMI_COMMAND,
					proto->id, 0x0);
	msg.len = 0;
	msg.content = NULL;

	reply.hdr = msg.hdr;
	reply.len = sizeof(reply_buffer);
	reply.content = &reply_buffer;

	ret = scmi_send_message(proto, &msg, &reply);
	if (ret < 0) {
		return ret;
	}

	if (reply_buffer.status != SCMI_SUCCESS) {
		return scmi_status_to_errno(reply_buffer.status);
	}

	*attributes = (uint32_t)reply_buffer.attributes;
	*latency = (uint32_t)reply_buffer.latency;
	strncpy(name, reply_buffer.name, SCMI_SHORT_NAME_MAX_SIZE);

	return 0;
}

int scmi_reset_domain_request(struct scmi_reset_domain_request_config cfg)
{
	struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_RESET_DOMAIN);
	struct scmi_message msg, reply;
	int status, ret;

	/* sanity checks */
	if (!proto) {
		return -EINVAL;
	}

	if (proto->id != SCMI_PROTOCOL_RESET_DOMAIN) {
		return -EINVAL;
	}

	msg.hdr = SCMI_MESSAGE_HDR_MAKE(RESET, SCMI_COMMAND, proto->id, 0x0);
	msg.len = sizeof(cfg);
	msg.content = &cfg;

	reply.hdr = msg.hdr;
	reply.len = sizeof(status);
	reply.content = &status;

	ret = scmi_send_message(proto, &msg, &reply);
	if (ret < 0) {
		return ret;
	}

	if (status != SCMI_SUCCESS) {
		return scmi_status_to_errno(status);
	}

	return 0;
}

