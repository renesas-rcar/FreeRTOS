/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_DISPATCH_H
#define RCAR_DISPATCH_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "rcar_msg.h"

/* Max number of framework drivers registered on one dispatch table. */
#define RCAR_MAX_DRIVERS  8U

/* Handler registered by a framework driver for one driver_id.
 * resp_payload/resp_payload_len let the handler attach a response payload
 * (e.g. a driver-specific result struct) - resp_payload is a caller-owned
 * buffer of RCAR_MSG_PAYLOAD_MAX_SIZE bytes, *resp_payload_len starts at 0
 * and the handler sets it to however many bytes it wrote, if any. */
typedef e_rcar_status_t (*rcar_rpmsg_handler_t)(const rcar_msg_t *msg, void *ctx,
                                                 void *resp_payload, uint16_t *resp_payload_len);

typedef struct {
    uint8_t               driver_id;
    rcar_rpmsg_handler_t  handler;
    void                  *ctx;
    bool                  in_use;
} rcar_dispatch_entry_t;

typedef struct {
    rcar_dispatch_entry_t entries[RCAR_MAX_DRIVERS];
    SemaphoreHandle_t     lock;
} rcar_dispatch_table_t;

/* Must be called once before register/unregister/dispatch. */
e_rcar_status_t rcar_dispatch_init(rcar_dispatch_table_t *table);

/* Fails with RCAR_ERR_ALREADY_REGISTERED if driver_id is already taken,
 * RCAR_ERR_BUFFER_FULL if the table has no free slot left. */
e_rcar_status_t rcar_rpmsg_register(rcar_dispatch_table_t *table, uint8_t driver_id,
                                     rcar_rpmsg_handler_t handler, void *ctx);

/* Fails with RCAR_ERR_NOT_REGISTERED if driver_id has no handler. */
e_rcar_status_t rcar_rpmsg_unregister(rcar_dispatch_table_t *table, uint8_t driver_id);

/* Looks up msg->hdr.driver_id and calls its handler. Returns
 * RCAR_ERR_NO_HANDLER (caller should drop the message) if none found. */
e_rcar_status_t rcar_dispatch(rcar_dispatch_table_t *table, const rcar_msg_t *msg);

#endif /* RCAR_DISPATCH_H */