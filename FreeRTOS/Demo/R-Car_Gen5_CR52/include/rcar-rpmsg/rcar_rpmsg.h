/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_RPMSG_H
#define RCAR_RPMSG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

/* rcar_rpmsg return codes. */
typedef enum {
    RCAR_OK = 0,
    RCAR_ERR_INVALID_PARAM,
    RCAR_ERR_NOT_INITIALIZED,
    RCAR_ERR_ALREADY_REGISTERED,  /* rcar_dispatch only */
    RCAR_ERR_NOT_REGISTERED,      /* rcar_dispatch only */
    RCAR_ERR_NO_HANDLER,          /* rcar_dispatch only */
    RCAR_ERR_PAYLOAD_TOO_LARGE,
    RCAR_ERR_BUFFER_FULL,
    RCAR_ERR_TIMEOUT,
    RCAR_ERR_TRANSPORT,
    RCAR_ERR_INTERNAL,
    RCAR_ERR_PEER_DISCONNECTED,
    RCAR_ERR_NOT_CONNECTED,       /* peer not bound yet */
} e_rcar_status_t;

#define RCAR_SUCCEEDED(s)   ((s) == RCAR_OK)
#define RCAR_FAILED(s)      ((s) != RCAR_OK)

/* Driver IDs. Route new ones in rcar_rpmsg.c. */
typedef enum {
    RCAR_RPMSG_ID_0        = 0,
    RCAR_RPMSG_ID_1        = 1,
    RCAR_RPMSG_ID_2        = 2,
    RCAR_RPMSG_ID_3        = 3,
    RCAR_RPMSG_ID_4        = 4,
    RCAR_RPMSG_ID_5        = 5,

    /* Same services on the other chip, over UCIe CH0: 0x10 + local id. */
    RCAR_RPMSG_ID_6       = 0x10,
} e_rcar_driver_id_t;

/*
 * rcar_rpmsg - inter-core messaging per driver_id over RPMsg.
 * One channel = one endpoint = one driver_id. Messages are processed
 * inside send()/receive(); no task, no RTOS call.
 *
 * Rules:
 *  1. One task calls receive() per channel; any task may send().
 *  2. Keep calling send()/receive() while a channel is open, or the
 *     peer's send() ends with RCAR_ERR_BUFFER_FULL.
 *  3. One-message mailbox: a newer message overwrites an unread one.
 *  4. on_unbind may run in another task: keep it short.
 *  5. On BE, open() blocks until FE is up.
 */

/* Max payload per message, in bytes. */
#define RCAR_RPMSG_MAX_PAYLOAD   (120U)

/* rcar_rpmsg_receive() timeout_ms values. */
#define RCAR_RPMSG_NO_WAIT       (0U)
#define RCAR_RPMSG_WAIT_FOREVER  (0xFFFFFFFFU)

/* Role of this core for a driver_id: FE on one core, BE on the other. */
typedef enum {
    RCAR_RPMSG_SIDE_FE = 0,
    RCAR_RPMSG_SIDE_BE,
} e_rcar_rpmsg_side_t;

/* Opaque channel. Allocate statically. */
typedef struct rcar_rpmsg_channel rcar_rpmsg_channel_t;

/* Channel private storage size. */
#define RCAR_RPMSG_CH_STORAGE_SIZE  (512U)

struct rcar_rpmsg_channel {
    alignas(8) uint8_t priv[RCAR_RPMSG_CH_STORAGE_SIZE];
};

/* Peer destroyed its endpoint. `ctx` is cfg.ctx. */
typedef void (*rcar_rpmsg_unbind_handler_t)(rcar_rpmsg_channel_t *ch, void *ctx);

/* Config for rcar_rpmsg_open(). */
typedef struct {
    e_rcar_driver_id_t             driver_id;
    e_rcar_rpmsg_side_t            side;
    const char                    *name;       /* NULL: default. < 32 bytes. Same on peer. */
    rcar_rpmsg_unbind_handler_t    on_unbind;  /* optional */
    void                          *ctx;        /* passed to on_unbind */
} rcar_rpmsg_open_cfg_t;

/*
 * Open a channel. On BE, blocks until FE is up.
 * RCAR_ERR_INVALID_PARAM: bad driver_id, side or name.
 * RCAR_ERR_NOT_INITIALIZED: driver_id released by an earlier last close().
 */
e_rcar_status_t rcar_rpmsg_open(rcar_rpmsg_channel_t *ch,
                                 rcar_rpmsg_open_cfg_t cfg);

/*
 * Close a channel. Closing the last channel of a driver_id releases it
 * until reboot. RCAR_ERR_NOT_INITIALIZED: not open.
 */
e_rcar_status_t rcar_rpmsg_close(rcar_rpmsg_channel_t *ch);

/*
 * Send `len` bytes. Never blocks.
 *   RCAR_ERR_NOT_CONNECTED     peer not bound yet, retry
 *   RCAR_ERR_BUFFER_FULL       peer not reading, retry
 *   RCAR_ERR_PEER_DISCONNECTED peer closed
 *   RCAR_ERR_PAYLOAD_TOO_LARGE len > RCAR_RPMSG_MAX_PAYLOAD
 */
e_rcar_status_t rcar_rpmsg_send(rcar_rpmsg_channel_t *ch,
                                 const void *data, uint16_t len);

/*
 * Receive one message into `buf`; `*out_len` = bytes copied (may be NULL).
 * RCAR_OK only when a message was received.
 *   RCAR_ERR_TIMEOUT           nothing within timeout_ms (also NO_WAIT)
 *   RCAR_ERR_PAYLOAD_TOO_LARGE truncated to buf_size
 *   RCAR_ERR_PEER_DISCONNECTED peer closed
 */
e_rcar_status_t rcar_rpmsg_receive(rcar_rpmsg_channel_t *ch,
                                    void *buf, uint16_t buf_size,
                                    uint16_t *out_len, uint32_t timeout_ms);

#endif /* RCAR_RPMSG_H */
