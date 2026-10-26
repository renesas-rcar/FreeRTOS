/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <openamp/open_amp.h>
#include <metal/atomic.h>
#include <metal/sleep.h>
#include <metal/time.h>
#include <metal/utilities.h>

#include "rcar-rpmsg/rcar_rpmsg.h"
#include "rcar_msg.h"
#include "rcar_rpmsg_os.h"
#include "platform_rcar_rpmsg.h"

#include <errno.h>
#include <string.h>

/* Logging Function include. */
#include "logging_stack.h"

/*
 * No RTOS calls: waits use metal_sleep_usec() or the OS hooks, locks are
 * atomics. No background task: send()/receive() drain their vdev first.
 */

/* Poll interval while waiting. */
#define RCAR_RPMSG_POLL_INTERVAL_US   1000U

/* MFIS vdev: max time between drains, max event wait. */
#define RCAR_RPMSG_SAFETY_DRAIN_MS    10U

/* Channels per vdev with a wake-up event; others poll. */
#define RCAR_RPMSG_MAX_CH_PER_VDEV    4U

/* driver_id without MFIS channel. */
#define RCAR_RPMSG_NO_MFIS            PLATFORM_NO_MFIS

/* Max finite timeout (32-bit usec timestamp). */
#define RCAR_RPMSG_MAX_TIMEOUT_MS     (0xFFFFFFFFU / 1000U)

/* Endpoint addressing. Same on both cores. */
typedef enum {
    RCAR_RPMSG_ADDR_NS = 0,      /* name service      */
    RCAR_RPMSG_ADDR_FIXED,       /* fixed addresses   */
} e_rcar_rpmsg_addr_mode_t;

/* Fixed addresses: one (BE, FE) pair per vdev. */
#define RCAR_RPMSG_FIXED_ADDR_BASE    0x100U
#define RCAR_RPMSG_FIXED_BE_ADDR(ch)  (RCAR_RPMSG_FIXED_ADDR_BASE + ((uint32_t)(ch) * 2U))
#define RCAR_RPMSG_FIXED_FE_ADDR(ch)  (RCAR_RPMSG_FIXED_BE_ADDR(ch) + 1U)

typedef struct {
    unsigned int              vdev;
    unsigned int              mfis_ch;       /* or RCAR_RPMSG_NO_MFIS */
    e_rcar_rpmsg_addr_mode_t  addr_mode;
    const char               *default_name;
    platform_link_t           link;          /* omitted: PLATFORM_LINK_LOCAL */
} rcar_rpmsg_route_t;

/* driver_id routing table. MFIS channels are shared with Linux: give one
 * only to drivers that need fast response. */
static e_rcar_status_t lookup_driver_route(e_rcar_driver_id_t driver_id,
                                            rcar_rpmsg_route_t *out)
{
    switch (driver_id) {
        case RCAR_RPMSG_ID_0:
            /* Matches rpmsg-echo.c on ch0. */
            *out = (rcar_rpmsg_route_t){ 0U, 0U, RCAR_RPMSG_ADDR_NS, "rcar-smmu" };
            return RCAR_OK;
        case RCAR_RPMSG_ID_1:
            *out = (rcar_rpmsg_route_t){ 1U, 1U, RCAR_RPMSG_ADDR_NS, "rcar-gpio" };
            return RCAR_OK;
        case RCAR_RPMSG_ID_2:
            *out = (rcar_rpmsg_route_t){ 2U, RCAR_RPMSG_NO_MFIS, RCAR_RPMSG_ADDR_NS, "rcar-i2c" };
            return RCAR_OK;
        case RCAR_RPMSG_ID_3:
            *out = (rcar_rpmsg_route_t){ 3U, RCAR_RPMSG_NO_MFIS, RCAR_RPMSG_ADDR_FIXED, "rcar-camera" };
            return RCAR_OK;
        case RCAR_RPMSG_ID_6:
            /* Other chip: vdev 0 of the UCIe rsc table, no MFIS across chips. */
            *out = (rcar_rpmsg_route_t){ 0U, RCAR_RPMSG_NO_MFIS, RCAR_RPMSG_ADDR_NS, "rcar-smmu-ucie0",
                                         PLATFORM_LINK_UCIE0 };
            return RCAR_OK;
        default:
            return RCAR_ERR_INVALID_PARAM;
    }
}

/* vdev state. CLOSED is final: the peer is not reset. */
enum {
    VDEV_NONE = 0,
    VDEV_CREATING,
    VDEV_READY,
    VDEV_CLOSING,
    VDEV_CLOSED,
};

/* ev_used[] values. */
enum {
    EV_FREE = 0,
    EV_CLAIMED,
    EV_ACTIVE,       /* signalled by the ISR */
};

typedef struct {
    atomic_int            state;
    atomic_int            drain_lock;
    struct remoteproc    *platform;
    struct rpmsg_device  *rdev;
    unsigned int          open_count;    /* under drain_lock */

    bool                  has_mfis;
    atomic_int            pending;       /* set by the ISR */
    uint32_t              last_drain_us; /* under drain_lock */

    /* Wake-up events (OS hooks only). Never destroyed: the ISR may still
     * signal a released slot. */
    void                 *ev[RCAR_RPMSG_MAX_CH_PER_VDEV];
    atomic_int            ev_used[RCAR_RPMSG_MAX_CH_PER_VDEV];
} rcar_rpmsg_vdev_t;

static rcar_rpmsg_vdev_t s_vdev[PLATFORM_LINK_MAX][2][PLATFORM_VDEV_MAX];   /* [link][side][vdev] */
static atomic_int        s_platform_lock[PLATFORM_LINK_MAX];
static const rcar_rpmsg_os_hooks_t *const s_hooks = &rcar_rpmsg_os_hooks_freertos;

/* Channel state, stored in rcar_rpmsg_channel_t::priv[]. */
typedef struct {
    struct rpmsg_endpoint          ept;
    rcar_rpmsg_channel_t          *ch;         /* owner, for on_unbind */
    platform_link_t                link;
    e_rcar_rpmsg_side_t            side;
    unsigned int                   vdev;
    int                            ev_slot;    /* or -1 */
    e_rcar_driver_id_t             driver_id;
    rcar_rpmsg_unbind_handler_t    on_unbind;
    void                          *ctx;
    volatile bool                  is_open;
    volatile bool                  peer_gone;
    /* One-slot mailbox, under drain_lock. Newest message wins. */
    rcar_msg_t                     rx_buf;
    bool                           rx_pending;
} rpmsg_ch_impl_t;

_Static_assert(sizeof(rpmsg_ch_impl_t) <= sizeof(((rcar_rpmsg_channel_t *)0)->priv),
               "rcar_rpmsg_channel_t::priv[] too small for rpmsg_ch_impl_t");
_Static_assert((size_t)alignof(rpmsg_ch_impl_t) <= (size_t)alignof(rcar_rpmsg_channel_t),
               "rcar_rpmsg_channel_t::priv[] alignment too small for rpmsg_ch_impl_t");
_Static_assert(RCAR_RPMSG_MAX_PAYLOAD <= RCAR_MSG_PAYLOAD_MAX_SIZE,
               "RCAR_RPMSG_MAX_PAYLOAD exceeds the wire frame payload");

static inline rpmsg_ch_impl_t *ch_impl(rcar_rpmsg_channel_t *ch)
{
    return (rpmsg_ch_impl_t *)ch->priv;
}

static inline rcar_rpmsg_vdev_t *ch_vdev(const rpmsg_ch_impl_t *c)
{
    return &s_vdev[c->link][c->side][c->vdev];
}

/* Locks: waiters sleep, never spin. */
static inline bool try_lock(atomic_int *lock)
{
    return atomic_exchange_explicit(lock, 1, memory_order_acquire) == 0;
}

static inline void lock(atomic_int *lock)
{
    while (!try_lock(lock)) {
        metal_sleep_usec(RCAR_RPMSG_POLL_INTERVAL_US);
    }
}

static inline void unlock(atomic_int *lock)
{
    atomic_store_explicit(lock, 0, memory_order_release);
}

/* ms since start_us, wrap-safe. */
static inline uint32_t elapsed_ms(uint32_t start_us)
{
    return ((uint32_t)metal_get_timestamp() - start_us) / 1000U;
}

/* One lock per link: a UCIe bring-up (seconds) does not hold up local ones. */
static e_rcar_status_t get_platform(platform_link_t link, e_rcar_rpmsg_side_t side,
                                    struct remoteproc **out)
{
    virtio_type_t type = (side == RCAR_RPMSG_SIDE_BE) ? VIRTIO_BACK_END : VIRTIO_FRONT_END;

    lock(&s_platform_lock[link]);
    int ret = rcar_platform_init(link, type, out);
    unlock(&s_platform_lock[link]);

    return (ret == 0) ? RCAR_OK : RCAR_ERR_TRANSPORT;
}

/* MFIS ISR: flag the vdev, wake its channels. */
static void vdev_notify_isr(void *arg)
{
    rcar_rpmsg_vdev_t *v = (rcar_rpmsg_vdev_t *)arg;
    const rcar_rpmsg_os_hooks_t *hooks = s_hooks;

    atomic_store_explicit(&v->pending, 1, memory_order_release);
    if (hooks == NULL) {
        return;
    }
    for (unsigned int i = 0; i < RCAR_RPMSG_MAX_CH_PER_VDEV; i++) {
        if (atomic_load_explicit(&v->ev_used[i], memory_order_acquire) == EV_ACTIVE) {
            hooks->event_signal(v->ev[i]);
        }
    }
}

/* Bring up the vdev once; other callers wait. Blocks on BE until FE is up. */
static e_rcar_status_t ensure_vdev(e_rcar_rpmsg_side_t side, const rcar_rpmsg_route_t *route)
{
    rcar_rpmsg_vdev_t *v = &s_vdev[route->link][side][route->vdev];

    for (;;) {
        int st = atomic_load_explicit(&v->state, memory_order_acquire);
        if (st == VDEV_READY) {
            return RCAR_OK;
        }
        if ((st == VDEV_CLOSING) || (st == VDEV_CLOSED)) {
            return RCAR_ERR_NOT_INITIALIZED;
        }

        int expected = VDEV_NONE;
        if ((st == VDEV_NONE) &&
            atomic_compare_exchange_strong(&v->state, &expected, VDEV_CREATING)) {
            struct remoteproc   *platform = NULL;
            struct rpmsg_device *rdev = NULL;

            v->has_mfis = (route->mfis_ch != RCAR_RPMSG_NO_MFIS);

            e_rcar_status_t rc = get_platform(route->link, side, &platform);
            if (rc == RCAR_OK) {
                int ret = platform_add_vdev(platform, route->vdev, route->mfis_ch,
                                            vdev_notify_isr, v, &rdev);
                if (ret == -ENODEV) {
                    rc = RCAR_ERR_INVALID_PARAM;   /* not in rsc table */
                } else if (ret != 0) {
                    rc = RCAR_ERR_TRANSPORT;
                } else {
                    LogDebug(("Platform added vdev successfully\n"));
                }
            }

            if (rc != RCAR_OK) {
                atomic_store_explicit(&v->state, VDEV_NONE, memory_order_release);
                return rc;
            }
            v->platform = platform;
            v->rdev     = rdev;
            atomic_store_explicit(&v->state, VDEV_READY, memory_order_release);
            return RCAR_OK;
        }

        metal_sleep_usec(RCAR_RPMSG_POLL_INTERVAL_US);
    }
}

/* Release the vdev if it has no open channel. Takes every drain_lock of
 * the side of the link, in index order: drains walk the vdev list being
 * edited. */
static void release_vdev(platform_link_t link, e_rcar_rpmsg_side_t side, unsigned int vdev)
{
    rcar_rpmsg_vdev_t *v = &s_vdev[link][side][vdev];

    for (unsigned int i = 0; i < PLATFORM_VDEV_MAX; i++) {
        lock(&s_vdev[link][side][i].drain_lock);
    }

    /* An open() may have come in meanwhile. */
    if ((atomic_load_explicit(&v->state, memory_order_acquire) == VDEV_READY) &&
        (v->open_count == 0U)) {
        atomic_store_explicit(&v->state, VDEV_CLOSING, memory_order_release);
        (void)platform_remove_vdev(v->platform, vdev);
        v->rdev = NULL;
        atomic_store_explicit(&v->state, VDEV_CLOSED, memory_order_release);
    }

    for (unsigned int i = PLATFORM_VDEV_MAX; i > 0U; i--) {
        unlock(&s_vdev[link][side][i - 1U].drain_lock);
    }
}

/* Caller holds drain_lock. MFIS vdev: drain only on interrupt or every
 * RCAR_RPMSG_SAFETY_DRAIN_MS. */
static void drain_locked(rcar_rpmsg_vdev_t *v, unsigned int vdev)
{
    if (v->has_mfis) {
        bool pending = atomic_exchange_explicit(&v->pending, 0, memory_order_acquire) != 0;
        uint32_t now_us = (uint32_t)metal_get_timestamp();
        if (!pending && ((now_us - v->last_drain_us) < (RCAR_RPMSG_SAFETY_DRAIN_MS * 1000U))) {
            return;
        }
        v->last_drain_us = now_us;
    }
    (void)platform_drain_vdev(v->platform, vdev);
}

/* Give the channel a wake-up event (MFIS vdev + OS hooks), else it polls. */
static void attach_event(rpmsg_ch_impl_t *c, rcar_rpmsg_vdev_t *v)
{
    const rcar_rpmsg_os_hooks_t *hooks = s_hooks;

    c->ev_slot = -1;
    if (!v->has_mfis || (hooks == NULL)) {
        return;
    }
    for (unsigned int i = 0; i < RCAR_RPMSG_MAX_CH_PER_VDEV; i++) {
        int expected = EV_FREE;
        if (!atomic_compare_exchange_strong(&v->ev_used[i], &expected, EV_CLAIMED)) {
            continue;
        }
        if (v->ev[i] == NULL) {
            v->ev[i] = hooks->event_create();
        }
        if (v->ev[i] == NULL) {
            atomic_store_explicit(&v->ev_used[i], EV_FREE, memory_order_release);
            return;
        }
        c->ev_slot = (int)i;
        atomic_store_explicit(&v->ev_used[i], EV_ACTIVE, memory_order_release);
        return;
    }
}

static void detach_event(rpmsg_ch_impl_t *c, rcar_rpmsg_vdev_t *v)
{
    if (c->ev_slot >= 0) {
        atomic_store_explicit(&v->ev_used[c->ev_slot], EV_FREE, memory_order_release);
        c->ev_slot = -1;
    }
}

/* Endpoint callbacks: run inside a drain, in the draining task. */
static int rcar_rpmsg_rx_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                             uint32_t src, void *priv)
{
    (void)src;
    (void)priv;

    rpmsg_ch_impl_t *c = metal_container_of(ept, rpmsg_ch_impl_t, ept);
    if ((data == NULL) || (len < sizeof(rcar_msg_header_t))) {
        return RPMSG_SUCCESS;
    }

    size_t n = metal_min(len, sizeof(c->rx_buf));
    (void)memcpy(&c->rx_buf, data, n);

    /* Clamp payload_len: the peer may not send an rcar_msg_t frame. */
    size_t max_payload = 0U;
    if (n > (size_t)offsetof(rcar_msg_t, payload)) {
        max_payload = n - (size_t)offsetof(rcar_msg_t, payload);
    }
    if (c->rx_buf.hdr.payload_len > max_payload) {
        c->rx_buf.hdr.payload_len = (uint16_t)max_payload;
    }

    c->rx_pending = true;
    return RPMSG_SUCCESS;
}

static void rcar_rpmsg_unbind_cb(struct rpmsg_endpoint *ept)
{
    rpmsg_ch_impl_t *c = metal_container_of(ept, rpmsg_ch_impl_t, ept);

    c->peer_gone = true;
    if (c->on_unbind != NULL) {
        c->on_unbind(c->ch, c->ctx);
    }
}

/* Public API */
e_rcar_status_t rcar_rpmsg_open(rcar_rpmsg_channel_t *ch,
                                 rcar_rpmsg_open_cfg_t cfg)
{
    if ((ch == NULL) ||
        ((cfg.side != RCAR_RPMSG_SIDE_FE) && (cfg.side != RCAR_RPMSG_SIDE_BE))) {
        return RCAR_ERR_INVALID_PARAM;
    }

    rcar_rpmsg_route_t route;
    e_rcar_status_t rc = lookup_driver_route(cfg.driver_id, &route);
    if (rc != RCAR_OK) {
        return rc;
    }

    const char *name = (cfg.name != NULL) ? cfg.name : route.default_name;
    if ((name[0] == '\0') || (strlen(name) >= RPMSG_NAME_SIZE)) {
        return RCAR_ERR_INVALID_PARAM;
    }

    rc = ensure_vdev(cfg.side, &route);
    if (rc != RCAR_OK) {
        return rc;
    }

    rpmsg_ch_impl_t *c = ch_impl(ch);
    (void)memset(c, 0, sizeof(*c));
    c->ch        = ch;
    c->link      = route.link;
    c->side      = cfg.side;
    c->vdev      = route.vdev;
    c->ev_slot   = -1;
    c->driver_id = cfg.driver_id;
    c->on_unbind = cfg.on_unbind;
    c->ctx       = cfg.ctx;

    uint32_t src = RPMSG_ADDR_ANY;
    uint32_t dst = RPMSG_ADDR_ANY;
    if (route.addr_mode == RCAR_RPMSG_ADDR_FIXED) {
        bool be = (cfg.side == RCAR_RPMSG_SIDE_BE);
        src = be ? RCAR_RPMSG_FIXED_BE_ADDR(route.vdev) : RCAR_RPMSG_FIXED_FE_ADDR(route.vdev);
        dst = be ? RCAR_RPMSG_FIXED_FE_ADDR(route.vdev) : RCAR_RPMSG_FIXED_BE_ADDR(route.vdev);
    }

    /* Under drain_lock: a concurrent last close() cannot release the vdev. */
    rcar_rpmsg_vdev_t *v = ch_vdev(c);
    lock(&v->drain_lock);
    if (atomic_load_explicit(&v->state, memory_order_acquire) != VDEV_READY) {
        unlock(&v->drain_lock);
        return RCAR_ERR_NOT_INITIALIZED;
    }

    int ret = rpmsg_create_ept(&c->ept, v->rdev, name, src, dst,
                               rcar_rpmsg_rx_cb, rcar_rpmsg_unbind_cb);
    if (ret != 0) {
        unlock(&v->drain_lock);
        return RCAR_ERR_TRANSPORT;
    }

    v->open_count++;
    attach_event(c, v);
    c->is_open = true;
    unlock(&v->drain_lock);
    return RCAR_OK;
}

e_rcar_status_t rcar_rpmsg_close(rcar_rpmsg_channel_t *ch)
{
    if (ch == NULL) {
        return RCAR_ERR_INVALID_PARAM;
    }
    rpmsg_ch_impl_t *c = ch_impl(ch);
    if (!c->is_open) {
        return RCAR_ERR_NOT_INITIALIZED;
    }

    /* Under drain_lock: no rx_cb runs during destroy. */
    rcar_rpmsg_vdev_t *v = ch_vdev(c);
    lock(&v->drain_lock);
    rpmsg_destroy_ept(&c->ept);
    detach_event(c, v);
    c->is_open = false;
    bool last = (--v->open_count == 0U);
    unlock(&v->drain_lock);

    if (last) {
        release_vdev(c->link, c->side, c->vdev);
    }
    return RCAR_OK;
}

e_rcar_status_t rcar_rpmsg_send(rcar_rpmsg_channel_t *ch,
                                 const void *data, uint16_t len)
{
    if (ch == NULL) {
        return RCAR_ERR_INVALID_PARAM;
    }
    rpmsg_ch_impl_t *c = ch_impl(ch);
    if (!c->is_open) {
        return RCAR_ERR_NOT_INITIALIZED;
    }
    if (len > RCAR_RPMSG_MAX_PAYLOAD) {
        return RCAR_ERR_PAYLOAD_TOO_LARGE;
    }
    if ((len > 0U) && (data == NULL)) {
        return RCAR_ERR_INVALID_PARAM;
    }

    /* Drain first so a send-only channel still gets bound. */
    rcar_rpmsg_vdev_t *v = ch_vdev(c);
    if (try_lock(&v->drain_lock)) {
        drain_locked(v, c->vdev);
        unlock(&v->drain_lock);
    }

    if (c->peer_gone) {
        return RCAR_ERR_PEER_DISCONNECTED;
    }
    if (c->ept.dest_addr == RPMSG_ADDR_ANY) {
        return RCAR_ERR_NOT_CONNECTED;
    }

    rcar_msg_t msg;
    (void)memset(&msg, 0, offsetof(rcar_msg_t, payload));
    msg.hdr.driver_id   = (uint8_t)c->driver_id;
    msg.hdr.payload_len = len;
    if (len > 0U) {
        (void)memcpy(msg.payload, data, len);
    }

    size_t wire_size = RCAR_MSG_WIRE_SIZE(len);
    int ret = rpmsg_trysend(&c->ept, &msg, (int)wire_size);
    if (ret >= 0) {
        return RCAR_OK;
    }
    switch (ret) {
        case RPMSG_ERR_NO_BUFF: return RCAR_ERR_BUFFER_FULL;
        case RPMSG_ERR_ADDR:    return RCAR_ERR_NOT_CONNECTED;
        default:                return RCAR_ERR_TRANSPORT;
    }
}

e_rcar_status_t rcar_rpmsg_receive(rcar_rpmsg_channel_t *ch,
                                    void *buf, uint16_t buf_size,
                                    uint16_t *out_len, uint32_t timeout_ms)
{
    if (ch == NULL) {
        return RCAR_ERR_INVALID_PARAM;
    }
    rpmsg_ch_impl_t *c = ch_impl(ch);
    if (!c->is_open) {
        return RCAR_ERR_NOT_INITIALIZED;
    }
    if ((buf == NULL) && (buf_size > 0U)) {
        return RCAR_ERR_INVALID_PARAM;
    }

    bool forever = (timeout_ms == RCAR_RPMSG_WAIT_FOREVER);
    if (!forever && (timeout_ms > RCAR_RPMSG_MAX_TIMEOUT_MS)) {
        timeout_ms = RCAR_RPMSG_MAX_TIMEOUT_MS;
    }
    uint32_t start_us = (uint32_t)metal_get_timestamp();
    rcar_rpmsg_vdev_t *v = ch_vdev(c);

    for (;;) {
        bool locked = try_lock(&v->drain_lock);
        if (locked) {
            drain_locked(v, c->vdev);

            if (c->rx_pending) {
                uint16_t n = c->rx_buf.hdr.payload_len;
                uint16_t copied = metal_min(n, buf_size);
                if (copied > 0U) {
                    (void)memcpy(buf, c->rx_buf.payload, copied);
                }
                c->rx_pending = false;
                unlock(&v->drain_lock);

                if (out_len != NULL) {
                    *out_len = copied;
                }
                return (copied < n) ? RCAR_ERR_PAYLOAD_TOO_LARGE : RCAR_OK;
            }
            unlock(&v->drain_lock);
        }

        if (c->peer_gone) {
            return RCAR_ERR_PEER_DISCONNECTED;
        }
        if (timeout_ms == RCAR_RPMSG_NO_WAIT) {
            return RCAR_ERR_TIMEOUT;
        }
        uint32_t waited_ms = elapsed_ms(start_us);
        if (!forever && (waited_ms >= timeout_ms)) {
            return RCAR_ERR_TIMEOUT;
        }

        /* Lock busy: the holder may be filling our mailbox, retry soon. */
        const rcar_rpmsg_os_hooks_t *hooks = s_hooks;
        if (locked && (c->ev_slot >= 0) && (hooks != NULL)) {
            uint32_t wait_ms = RCAR_RPMSG_SAFETY_DRAIN_MS;
            if (!forever && ((timeout_ms - waited_ms) < wait_ms)) {
                wait_ms = timeout_ms - waited_ms;
            }
            (void)hooks->event_wait(v->ev[c->ev_slot], wait_ms);
        } else {
            metal_sleep_usec(RCAR_RPMSG_POLL_INTERVAL_US);
        }
    }
}
