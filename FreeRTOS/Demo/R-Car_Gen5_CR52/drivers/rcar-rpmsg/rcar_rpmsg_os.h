/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_RPMSG_OS_H
#define RCAR_RPMSG_OS_H

#include <stdint.h>
#include <stdbool.h>

/*
 * OS hooks (internal). With hooks, MFIS driver_ids wake receive() on the
 * interrupt; UCIe driver_ids poll every 1 ms.
 *
 *   event_create : new binary event, NULL on failure.
 *   event_wait   : wait for a signal or timeout_ms; consumes the signal.
 *   event_signal : latch a signal. May run in interrupt context.
 */
typedef struct {
    void *(*event_create)(void);
    bool  (*event_wait)(void *ev, uint32_t timeout_ms);
    void  (*event_signal)(void *ev);
} rcar_rpmsg_os_hooks_t;

/*
 * FreeRTOS hooks, used by rcar_rpmsg.c.
 * The MFIS interrupt priority must allow FreeRTOS FromISR calls.
 */
extern const rcar_rpmsg_os_hooks_t rcar_rpmsg_os_hooks_freertos;

#endif /* RCAR_RPMSG_OS_H */
