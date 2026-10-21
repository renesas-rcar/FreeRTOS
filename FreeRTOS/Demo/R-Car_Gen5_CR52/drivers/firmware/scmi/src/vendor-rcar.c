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
#include "scmi/inc/vendor-rcar.h"

SCMI_PROTOCOL_DEFINE_NODEV(SCMI_PROTOCOL_VENDOR, NULL);

/**
 * @struct scmi_reset_get_status_config
 *
 * @brief Describes the parameters for the
 * getting RESET status command
 */
struct scmi_vendor_reset_domain_status_get_config {
    uint32_t status;
    uint32_t reset_status;
};

enum vendor_protocol_cmd {
    RESET_GET_STATUS = 0x080,
};

struct vendor_reset_protocol_attributes {
    int32_t status;
    uint32_t attributes;
};

int scmi_vendor_reset_domain_status_get(struct scmi_vendor_request_config *cfg) {
    struct scmi_protocol *proto = &SCMI_PROTOCOL_NAME(SCMI_PROTOCOL_VENDOR);
    struct scmi_vendor_reset_domain_status_get_config reply_buffer;
    struct scmi_message msg;
    struct scmi_message reply;
    int ret;

    ret = RET_OK;

    /* sanity checks */
    if ( (!proto) || (proto->id != SCMI_PROTOCOL_VENDOR) ) {
        ret = -EINVAL;
    } else {
        msg.hdr = SCMI_MESSAGE_HDR_MAKE(RESET_GET_STATUS, SCMI_COMMAND, proto->id, 0x0);
        msg.len = sizeof(cfg->domain_id);
        msg.content = &cfg->domain_id;

        reply.hdr = msg.hdr;
        reply.len = sizeof(reply_buffer);
        reply.content = &reply_buffer;

        ret = scmi_send_message(proto, &msg, &reply);
        if (ret < 0) {
            (void) LogError(("scmi_send_message failed, return %d \r\n", ret));
        } else {
            if (reply_buffer.status != SCMI_SUCCESS) {
                ret = scmi_status_to_errno(reply_buffer.status);
                (void) LogError(("scmi_status_to_errno, return %d \r\n", ret));
            }
        }

        cfg->reset_status = reply_buffer.reset_status;
    }

    return ret;
}

