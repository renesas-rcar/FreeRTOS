/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_RPMSG_H
#define RCAR_RPMSG_H

/**
 * @defgroup RCAR_OVER_RPMSG_Module RCAR OVER RPMSG Module
 * @{
 * @brief This module provides inter-core messaging per driver_id over RPMsg.
 *
 */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @brief rcar_rpmsg return codes.
 */
typedef enum {
    RCAR_OK = 0,                  ///< Success.
    RCAR_ERR_INVALID_PARAM,       ///< Invalid parameter.
    RCAR_ERR_NOT_INITIALIZED,     ///< Channel not open, or driver_id already released.
    RCAR_ERR_ALREADY_REGISTERED,  ///< rcar_dispatch only.
    RCAR_ERR_NOT_REGISTERED,      ///< rcar_dispatch only.
    RCAR_ERR_NO_HANDLER,          ///< rcar_dispatch only.
    RCAR_ERR_PAYLOAD_TOO_LARGE,   ///< Payload exceeds the maximum or the receive buffer.
    RCAR_ERR_BUFFER_FULL,         ///< No free transport buffer, peer not reading.
    RCAR_ERR_TIMEOUT,             ///< No message within the timeout.
    RCAR_ERR_TRANSPORT,           ///< RPMsg/virtio transport error.
    RCAR_ERR_INTERNAL,            ///< Internal error.
    RCAR_ERR_PEER_DISCONNECTED,   ///< Peer closed its endpoint.
    RCAR_ERR_NOT_CONNECTED,       ///< Peer not bound yet.
} e_rcar_status_t;

/** @brief True if status `s` is RCAR_OK. */
#define RCAR_SUCCEEDED(s)   ((s) == RCAR_OK)
/** @brief True if status `s` is an error. */
#define RCAR_FAILED(s)      ((s) != RCAR_OK)

/**
 * @brief Driver IDs. Route new ones in rcar_rpmsg.c.
 */
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

/** @brief Max payload per message, in bytes. */
#define RCAR_RPMSG_MAX_PAYLOAD   (120U)

/** @brief rcar_rpmsg_receive() timeout_ms value: return immediately. */
#define RCAR_RPMSG_NO_WAIT       (0U)
/** @brief rcar_rpmsg_receive() timeout_ms value: wait without limit. */
#define RCAR_RPMSG_WAIT_FOREVER  (0xFFFFFFFFU)

/**
 * @brief Role of this core for a driver_id: FE on one core, BE on the other.
 */
typedef enum {
    RCAR_RPMSG_SIDE_FE = 0,  ///< Frontend side.
    RCAR_RPMSG_SIDE_BE,      ///< Backend side.
} e_rcar_rpmsg_side_t;

/***********************************************************************************************************************
 * Struct definitions
 **********************************************************************************************************************/
/**
 * @brief Opaque channel. Allocate statically.
 */
typedef struct rcar_rpmsg_channel rcar_rpmsg_channel_t;

/** @brief Channel private storage size. */
#define RCAR_RPMSG_CH_STORAGE_SIZE  (512U)

/**
 * @brief Channel storage.
 */
struct rcar_rpmsg_channel {
    alignas(8) uint8_t priv[RCAR_RPMSG_CH_STORAGE_SIZE];  ///< Private driver data.
};

/**
 * @brief Called when the peer destroyed its endpoint.
 *
 * @param[in]  ch  - Pointer to the channel whose peer is gone.
 * @param[in]  ctx - cfg.ctx given to rcar_rpmsg_open().
 */
typedef void (*rcar_rpmsg_unbind_handler_t)(rcar_rpmsg_channel_t *ch, void *ctx);

/**
 * @brief Config for rcar_rpmsg_open().
 */
typedef struct {
    e_rcar_driver_id_t             driver_id;  ///< Driver ID of the channel.
    e_rcar_rpmsg_side_t            side;       ///< Role of this core.
    const char                    *name;       ///< NULL: default. < 32 bytes. Same on peer.
    rcar_rpmsg_unbind_handler_t    on_unbind;  ///< Optional.
    void                          *ctx;        ///< Passed to on_unbind.
} rcar_rpmsg_open_cfg_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Open a channel.
 *
 * @param[out] ch  - Pointer to the channel storage.
 * @param[in]  cfg - Channel configuration.
 *
 * @retval RCAR_OK                  Channel opened.
 * @retval RCAR_ERR_INVALID_PARAM   ch is NULL, or bad driver_id, side or name.
 * @retval RCAR_ERR_NOT_INITIALIZED driver_id released by an earlier last close().
 * @retval RCAR_ERR_TRANSPORT       Transport or endpoint setup failed.
 */
e_rcar_status_t rcar_rpmsg_open(rcar_rpmsg_channel_t *ch,
                                 rcar_rpmsg_open_cfg_t cfg);

/**
 * @brief Close a channel.
 *
 * Closing the last channel of a driver_id releases it until reboot.
 *
 * @param[in]  ch - Pointer to the channel.
 *
 * @retval RCAR_OK                  Channel closed.
 * @retval RCAR_ERR_INVALID_PARAM   ch is NULL.
 * @retval RCAR_ERR_NOT_INITIALIZED Not open.
 */
e_rcar_status_t rcar_rpmsg_close(rcar_rpmsg_channel_t *ch);

/**
 * @brief Send `len` bytes.
 *
 * @param[in]  ch   - Pointer to the channel.
 * @param[in]  data - Pointer to the data to send. May be NULL when len is 0.
 * @param[in]  len  - Number of bytes to send, at most RCAR_RPMSG_MAX_PAYLOAD.
 *
 * @retval RCAR_OK                    Message sent.
 * @retval RCAR_ERR_INVALID_PARAM     ch is NULL, or data is NULL with len > 0.
 * @retval RCAR_ERR_NOT_INITIALIZED   Not open.
 * @retval RCAR_ERR_NOT_CONNECTED     Peer not bound yet, retry.
 * @retval RCAR_ERR_BUFFER_FULL       Peer not reading, retry.
 * @retval RCAR_ERR_PEER_DISCONNECTED Peer closed.
 * @retval RCAR_ERR_PAYLOAD_TOO_LARGE len > RCAR_RPMSG_MAX_PAYLOAD.
 * @retval RCAR_ERR_TRANSPORT         Transport error.
 */
e_rcar_status_t rcar_rpmsg_send(rcar_rpmsg_channel_t *ch,
                                 const void *data, uint16_t len);

/**
 * @brief Receive one message into `buf`.
 *
 * @param[in]  ch         - Pointer to the channel.
 * @param[out] buf        - Pointer to the buffer for the received data. May be NULL when buf_size is 0.
 * @param[in]  buf_size   - Size of buf, in bytes.
 * @param[out] out_len    - Bytes copied (may be NULL).
 * @param[in]  timeout_ms - Timeout in milliseconds, or RCAR_RPMSG_NO_WAIT / RCAR_RPMSG_WAIT_FOREVER.
 *
 * @retval RCAR_OK                    A message was received.
 * @retval RCAR_ERR_INVALID_PARAM     ch is NULL, or buf is NULL with buf_size > 0.
 * @retval RCAR_ERR_NOT_INITIALIZED   Not open.
 * @retval RCAR_ERR_TIMEOUT           Nothing within timeout_ms (also NO_WAIT).
 * @retval RCAR_ERR_PAYLOAD_TOO_LARGE Truncated to buf_size.
 * @retval RCAR_ERR_PEER_DISCONNECTED Peer closed.
 */
e_rcar_status_t rcar_rpmsg_receive(rcar_rpmsg_channel_t *ch,
                                    void *buf, uint16_t buf_size,
                                    uint16_t *out_len, uint32_t timeout_ms);

/** @} */ // end of RCAR_OVER_RPMSG_Module

#endif /* RCAR_RPMSG_H */
