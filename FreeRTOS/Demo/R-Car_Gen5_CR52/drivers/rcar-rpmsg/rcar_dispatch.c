/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "rcar_dispatch.h"

#include <string.h>

e_rcar_status_t rcar_dispatch_init(rcar_dispatch_table_t *table)
{
    if (table == NULL) {
        return RCAR_ERR_INVALID_PARAM;
    }

    (void)memset(table->entries, 0, sizeof(table->entries));

    table->lock = xSemaphoreCreateMutex();
    if (table->lock == NULL) {
        return RCAR_ERR_INTERNAL;
    }

    return RCAR_OK;
}

e_rcar_status_t rcar_rpmsg_register(rcar_dispatch_table_t *table, uint8_t driver_id,
                                     rcar_rpmsg_handler_t handler, void *ctx)
{
    if ((table == NULL) || (handler == NULL)) {
        return RCAR_ERR_INVALID_PARAM;
    }

    e_rcar_status_t status = RCAR_ERR_BUFFER_FULL;
    uint32_t        free_slot = RCAR_MAX_DRIVERS;

    xSemaphoreTake(table->lock, portMAX_DELAY);

    for (uint32_t i = 0U; i < RCAR_MAX_DRIVERS; i++) {
        if (table->entries[i].in_use && (table->entries[i].driver_id == driver_id)) {
            xSemaphoreGive(table->lock);
            return RCAR_ERR_ALREADY_REGISTERED;
        }
        if ((!table->entries[i].in_use) && (free_slot == RCAR_MAX_DRIVERS)) {
            free_slot = i;
        }
    }

    if (free_slot != RCAR_MAX_DRIVERS) {
        table->entries[free_slot].driver_id = driver_id;
        table->entries[free_slot].handler   = handler;
        table->entries[free_slot].ctx       = ctx;
        table->entries[free_slot].in_use    = true;
        status = RCAR_OK;
    }

    xSemaphoreGive(table->lock);
    return status;
}

e_rcar_status_t rcar_rpmsg_unregister(rcar_dispatch_table_t *table, uint8_t driver_id)
{
    if (table == NULL) {
        return RCAR_ERR_INVALID_PARAM;
    }

    e_rcar_status_t status = RCAR_ERR_NOT_REGISTERED;

    xSemaphoreTake(table->lock, portMAX_DELAY);

    for (uint32_t i = 0U; i < RCAR_MAX_DRIVERS; i++) {
        if (table->entries[i].in_use && (table->entries[i].driver_id == driver_id)) {
            table->entries[i].in_use = false;
            status = RCAR_OK;
            break;
        }
    }

    xSemaphoreGive(table->lock);
    return status;
}

e_rcar_status_t rcar_dispatch(rcar_dispatch_table_t *table, const rcar_msg_t *msg)
{
    if ((table == NULL) || (msg == NULL)) {
        return RCAR_ERR_INVALID_PARAM;
    }

    rcar_rpmsg_handler_t handler = NULL;
    void                  *ctx = NULL;

    /* Copy out handler+ctx under the lock, then release it before calling
     * the handler - never call out to FWK code while holding table->lock,
     * or a handler that (un)registers from within itself would deadlock. */
    xSemaphoreTake(table->lock, portMAX_DELAY);
    for (uint32_t i = 0U; i < RCAR_MAX_DRIVERS; i++) {
        if (table->entries[i].in_use && (table->entries[i].driver_id == msg->hdr.driver_id)) {
            handler = table->entries[i].handler;
            ctx     = table->entries[i].ctx;
            break;
        }
    }
    xSemaphoreGive(table->lock);

    if (handler == NULL) {
        return RCAR_ERR_NO_HANDLER;
    }

    uint8_t  resp_payload[RCAR_MSG_PAYLOAD_MAX_SIZE];
    uint16_t resp_payload_len = 0U;
    return handler(msg, ctx, resp_payload, &resp_payload_len);
}