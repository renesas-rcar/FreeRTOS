/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_RCAR_V2_H
#define PLATFORM_RCAR_V2_H

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>
#include <openamp/rpmsg_virtio.h>
#include "mfis/mfis.h"

/* Cortex R52 memory attributes */
#define DEVICE_SHARED       ((uint32_t)0x00000001U) /* device, shareable */
#define DEVICE_NONSHARED    ((uint32_t)0x00000010U) /* device, non shareable */
#define NORM_NSHARED_NCACHE ((uint32_t)0x00000008U) /* Non cacheable  non shareable */
#define NORM_SHARED_NCACHE  ((uint32_t)0x0000000CU) /* Non cacheable shareable */
#define PRIV_RW_USER_RW     ((uint32_t)0x00000003U<<8U) /* Full Access */
#define ATTR_NORM_SHARED_NCACHE_PRIV_RW_USER_RW     (NORM_SHARED_NCACHE | PRIV_RW_USER_RW)

/* Max vdev per side (= rsc table vdev entries). */
#define PLATFORM_VDEV_MAX   4U

/* mfis_ch value: no MFIS, both sides poll. */
#define PLATFORM_NO_MFIS    0xFFU

/* Max vrings per vdev. */
#define PLATFORM_VRING_MAX  2U

#if defined __cplusplus
extern "C" {
#endif

typedef enum virtio_type
{
    VIRTIO_FRONT_END = VIRTIO_DEV_DRIVER,
    VIRTIO_BACK_END = VIRTIO_DEV_DEVICE,
} virtio_type_t;
/*
 * Link of a driver_id route: one rsc table + shared memory each, placed by
 * its rsc table linker script. Also the rsc table index of rsc_table_rpmsg.h.
 */
typedef enum {
    PLATFORM_LINK_LOCAL = 0,   /* same chip: lscript_rsc_table_rcar_rpmsg.ld, MFIS */
    PLATFORM_LINK_UCIE0,       /* other chip over UCIe CH0: lscript_rsc_table_rcar_rpmsg_ucie0.ld, polled */
    PLATFORM_LINK_MAX
} platform_link_t;

/* Called from the MFIS ISR on peer notification. Must not block. */
typedef void (*platform_notify_cb_t)(void *arg);

/* Per-vdev state, owned by the platform. */
struct platform_vdev {
    struct mfis_channel          mfis;
    uint8_t                      has_mfis;
    uint8_t                      mfis_inited;
    uint8_t                      rsc_cached;    /* notify ids valid */
    volatile uint8_t             ready;

    /* Cached from the rsc table. */
    uint32_t                     vdev_notifyid;
    uint32_t                     vring_notifyid[PLATFORM_VRING_MAX];
    unsigned int                 num_vrings;

    metal_phys_addr_t            vring_pa;
    metal_phys_addr_t            shared_buf_pa;

    struct rpmsg_virtio_device   rpvdev;
    struct rpmsg_virtio_shm_pool shpool;        /* FE only */
};

struct remoteproc_priv {
    virtio_type_t        type;
    platform_link_t      link;
    metal_phys_addr_t    shm_base;      /* = rsc table of the link */
    struct platform_vdev vdev[PLATFORM_VDEV_MAX];
};

/* remoteproc ops, remoteproc_rcar_rpmsg.c */
extern const struct remoteproc_ops rcar_rpmsg_proc_ops;

/*
 * Create the remoteproc of one side of @link (once; later calls return it).
 * BE writes the rsc table, FE reads it. Caller serializes calls per @link.
 * Returns -ENODEV if the app does not link the rsc table of @link.
 */
int rcar_platform_init(platform_link_t link, virtio_type_t type, struct remoteproc **platform);

/*
 * Bring up vdev @ch with MFIS channel @mfis_ch (or PLATFORM_NO_MFIS).
 * Same @mfis_ch on both cores. BE blocks until FE sets DRIVER_OK.
 * @notify_cb runs in the MFIS ISR. Caller serializes calls.
 * Returns -ENODEV if the rsc table has no entry @ch.
 */
int platform_add_vdev(struct remoteproc *platform, unsigned int ch,
                      unsigned int mfis_ch,
                      platform_notify_cb_t notify_cb, void *cb_arg,
                      struct rpmsg_device **rdev);

/*
 * Release vdev @ch. Do not add it again. The peer is not told.
 * No platform_drain_vdev() may run on any vdev of @platform meanwhile.
 */
int platform_remove_vdev(struct remoteproc *platform, unsigned int ch);

/*
 * Process all pending messages of vdev @ch; endpoint callbacks run in the
 * caller. Serialize per @ch. Returns -EAGAIN if the vdev is not ready.
 */
int platform_drain_vdev(struct remoteproc *platform, unsigned int ch);

#if defined __cplusplus
}
#endif

#endif /* PLATFORM_RCAR_V2_H */
