/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_MSG_H
#define RCAR_MSG_H

#include <stdint.h>
#include <stddef.h>

#include "rcar-rpmsg/rcar_rpmsg.h"

/* Wire frame (internal): header + payload. Reserved fields are sent as 0. */
#define RCAR_MSG_PAYLOAD_MAX_SIZE  120U

typedef struct {
    uint8_t  driver_id;    /* e_rcar_driver_id_t */
    uint8_t  msg_type;     /* reserved */
    uint16_t payload_len;
    uint32_t seq_id;       /* reserved */
    int32_t  status;       /* reserved */
} rcar_msg_header_t;

typedef struct {
    rcar_msg_header_t hdr;
    uint8_t            payload[RCAR_MSG_PAYLOAD_MAX_SIZE] __attribute__((aligned(8)));
} rcar_msg_t;

#define RCAR_MSG_WIRE_SIZE(n)      ((size_t)offsetof(rcar_msg_t, payload) + (size_t)(n))

#endif /* RCAR_MSG_H */