/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "platform_rcar_rpmsg.h"
#include "rsc_table_rpmsg.h"
#include "transport_ucie.h"

#include <metal/utilities.h>
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>

/* Logging Function include. */
#include "logging_stack.h"

/* Shared memory layout of one link, same on both cores:
 *   shm_base               = rsc table of the link (its linker script):
 *                            local 0x96600000, UCIe 0x9E600000
 *                            (RFS2: 0x96600000, lscript_rsc_table_rcar_rpmsg_ucie0_rfs.ld)
 *   +-- RSC_TABLE_RESERVE  resource table
 *   +-- vdev0 block        vring0 | vring1 | shared buffers
 *   +-- vdev1 block ...
 */
#define SHARED_CH_RAM_SIZE   (0x122000UL)

#define RSC_TABLE_RESERVE    (0x1000UL)
#define PER_CH_VRING_AREA    (0x8000UL)      /* vring0 + vring1 */
#define VRING_MEM_OFFSET     (0x4000UL)      /* vring0 -> vring1 */
#define SHARED_BUF_SIZE      (0x40000UL)
#define PER_CH_BLOCK         (PER_CH_VRING_AREA + SHARED_BUF_SIZE)

/* The last page stays free: UCIe BE sync page (transport_ucie.c). */
_Static_assert(((uint32_t)RSC_TABLE_RESERVE + (PLATFORM_VDEV_MAX * PER_CH_BLOCK) + 0x1000U) <= (uint32_t)SHARED_CH_RAM_SIZE,
               "SHARED_CH_RAM_SIZE too small for PLATFORM_VDEV_MAX channels + UCIe sync page");
_Static_assert(((int)PLATFORM_LINK_LOCAL == RSC_TABLE_LOCAL) && ((int)PLATFORM_LINK_UCIE0 == RSC_TABLE_UCIE0) &&
               ((int)PLATFORM_LINK_MAX == RSC_TABLE_NUM), "link = rsc table index");

#define ATTR_NORM_NSHARED   NORM_NSHARED_NCACHE
#define ATTR_PRIV_RW        PRIV_RW_USER_RW
#define ATTR_NORM_NSHARED_PRIV_RW       (ATTR_NORM_NSHARED | ATTR_PRIV_RW)

extern size_t find_rsc(void *rsc_table, unsigned int rsc_type, unsigned int index);

/* [link][0] = FE, [link][1] = BE */
static struct remoteproc      s_rproc[PLATFORM_LINK_MAX][2];
static struct remoteproc_priv s_priv[PLATFORM_LINK_MAX][2];
static uint8_t                s_rproc_inited[PLATFORM_LINK_MAX][2];

static unsigned int side_index(virtio_type_t type)
{
    return (type == VIRTIO_BACK_END) ? 1U : 0U;
}

static metal_phys_addr_t vring_base_pa(const struct remoteproc_priv *priv, unsigned int ch)
{
    return priv->shm_base + RSC_TABLE_RESERVE + ((metal_phys_addr_t)ch * PER_CH_BLOCK);
}

/* Init remoteproc, map rsc table + shared memory, parse rsc table. */
static int platform_create_proc(struct remoteproc *rproc, struct remoteproc_priv *rproc_priv)
{
    void *rsc_table;
    int rsc_size = 0;
    metal_phys_addr_t pa;
    uint8_t link = (uint8_t)rproc_priv->link;

    LogDebug(("platform_create_proc: link=%u, type=%s\n", (unsigned int)link,
         (rproc_priv->type == VIRTIO_BACK_END) ? "BE" : "FE"));

    /* The rsc table opens the shared memory of the link. */
    rsc_table = rcar_rsc_table_get((int)link, &rsc_size);
    if ((rsc_table == NULL) || (rsc_size <= 0))
    {
        return -ENODEV;   /* rsc table linker script of the link not linked */
    }
    rproc_priv->shm_base = (metal_phys_addr_t)(uintptr_t)(uint8_t *)rsc_table;
    LogDebug(("platform_create_proc: rsc_table=%p, rsc_size=%d\n", rsc_table, rsc_size));

    if (rproc_priv->type == VIRTIO_BACK_END)
    {
        /* BE writes the rsc table, FE only reads it. */
        rcar_rsc_table_init(link);
    }

    if (rproc_priv->link == PLATFORM_LINK_UCIE0)
    {
        /* BE has written the rsc table, FE has not read it yet. */
        int ucie_ret = platform_ucie_prepare(rproc_priv->type, (uintptr_t)rproc_priv->shm_base,
                                             SHARED_CH_RAM_SIZE);
        LogDebug(("platform_create_proc: platform_ucie_prepare ret=%d\n", ucie_ret));
        if (ucie_ret != 0)
        {
            return ucie_ret;
        }
    }

    if (remoteproc_init(rproc, &rcar_rpmsg_proc_ops, rproc_priv) == NULL)
    {
        return -EINVAL;
    }

    /* mmap resource table */
    pa = (metal_phys_addr_t)(uintptr_t)(uint8_t *)rsc_table;
    if (remoteproc_mmap(rproc, &pa, NULL, (size_t)rsc_size,
                        ATTR_NORM_NSHARED_PRIV_RW, &rproc->rsc_io) == NULL)
    {
        remoteproc_remove(rproc);
        return -ENOMEM;
    }

    /* mmap shared memory */
    pa = rproc_priv->shm_base;
    if (remoteproc_mmap(rproc, &pa, NULL, SHARED_CH_RAM_SIZE,
                        ATTR_NORM_NSHARED_PRIV_RW, NULL) == NULL)
    {
        remoteproc_remove(rproc);
        return -ENOMEM;
    }

    /* parse resource table */
    int ret = remoteproc_set_rsc_table(rproc, rproc->rsc_io->virt, (size_t)rsc_size);
    LogDebug(("platform_create_proc: remoteproc_set_rsc_table ret=%d\n", ret));
    if (ret != 0)
    {
        remoteproc_remove(rproc);
    }
    return ret;
}

int rcar_platform_init(platform_link_t link, virtio_type_t type, struct remoteproc **platform)
{
    if ((platform == NULL) || ((unsigned int)link >= (unsigned int)PLATFORM_LINK_MAX))
    {
        return -EINVAL;
    }

    unsigned int s = side_index(type);
    if (s_rproc_inited[link][s] == 0U)
    {
        (void)memset(&s_priv[link][s], 0, sizeof(s_priv[link][s]));
        (void)memset(&s_rproc[link][s], 0, sizeof(s_rproc[link][s]));
        s_priv[link][s].type = type;
        s_priv[link][s].link = link;

        int ret = platform_create_proc(&s_rproc[link][s], &s_priv[link][s]);
        if (ret != 0)
        {
            return ret;
        }
        s_rproc_inited[link][s] = 1U;
    }

    *platform = &s_rproc[link][s];
    return 0;
}

/* Cache notify ids; FE also sets unset vring addresses. */
static int platform_setup_vdev_rsc(struct remoteproc *rproc, unsigned int ch,
                                   struct platform_vdev *slot, int role)
{
    char *rsc_table = rproc->rsc_table;
    int ret = 0;

    metal_mutex_acquire(&rproc->lock);

    size_t off = find_rsc(rsc_table, RSC_VDEV, ch);
    if (off == 0U)
    {
        ret = -ENODEV;
    }
    else
    {
        struct fw_rsc_vdev *vdev_rsc = (struct fw_rsc_vdev *)(void *)((uint8_t *)rsc_table + off);

        slot->vdev_notifyid = vdev_rsc->notifyid;
        slot->num_vrings = metal_min(vdev_rsc->num_of_vrings, PLATFORM_VRING_MAX);
        for (unsigned int i = 0; i < slot->num_vrings; i++)
        {
            struct fw_rsc_vdev_vring *vring_rsc = &vdev_rsc->vring[i];

            slot->vring_notifyid[i] = vring_rsc->notifyid;
            if ((role == VIRTIO_DEV_DRIVER) && (vring_rsc->da == FW_RSC_U32_ADDR_ANY))
            {
                vring_rsc->da = (uint32_t)(slot->vring_pa + (i * VRING_MEM_OFFSET));
            }
        }
    }

    metal_mutex_release(&rproc->lock);
    return ret;
}

int platform_add_vdev(struct remoteproc *platform, unsigned int ch,
                      unsigned int mfis_ch,
                      platform_notify_cb_t notify_cb, void *cb_arg,
                      struct rpmsg_device **rdev)
{
    if ((platform == NULL) || (rdev == NULL) || (ch >= PLATFORM_VDEV_MAX) ||
        ((mfis_ch != PLATFORM_NO_MFIS) && (mfis_ch > 63U)))
    {
        return -EINVAL;
    }

    struct remoteproc *rproc = platform;
    struct remoteproc_priv *priv = rproc->priv;
    struct platform_vdev *slot = &priv->vdev[ch];
    int role = (priv->type == VIRTIO_BACK_END) ? VIRTIO_DEV_DEVICE : VIRTIO_DEV_DRIVER;

    if (slot->ready != 0U)
    {
        *rdev = rpmsg_virtio_get_rpmsg_device(&slot->rpvdev);
        return 0;
    }

    LogDebug(("platform_add_vdev: ch=%u, type=%s\n", ch,
         (priv->type == VIRTIO_BACK_END) ? "BE" : "FE"));

    slot->vring_pa      = vring_base_pa(priv, ch);
    slot->shared_buf_pa = slot->vring_pa + PER_CH_VRING_AREA;

    int ret = platform_setup_vdev_rsc(rproc, ch, slot, role);
    if (ret != 0)
    {
        LogDebug(("platform_add_vdev: no vdev entry %u in rsc table\n", ch));
        return ret;
    }
    slot->rsc_cached = 1U;

    /* MFIS before the vdev: creating it already notifies the peer. */
    slot->has_mfis = (mfis_ch != PLATFORM_NO_MFIS) ? 1U : 0U;
    if ((slot->has_mfis != 0U) && (slot->mfis_inited == 0U))
    {
        slot->mfis.ch           = (uint8_t)mfis_ch;
        slot->mfis.type         = (priv->type == VIRTIO_BACK_END) ? MFIS_TYPE_RECEVER : MFIS_TYPE_SENDER;
        slot->mfis.cb_function  = notify_cb;
        slot->mfis.arg          = cb_arg;
        int mfis_ret = mfis_init(&slot->mfis);
        if (mfis_ret != 0)
        {
            LogError(("%s: mfis_init ch=%u failed, ret=%d\n", __func__, mfis_ch, mfis_ret));
            return mfis_ret;
        }
        slot->mfis_inited = 1U;
    }

    struct metal_io_region *shbuf_io = remoteproc_get_io_with_pa(rproc, slot->shared_buf_pa);
    if (shbuf_io == NULL)
    {
        return -ENOMEM;
    }

    struct rpmsg_virtio_shm_pool *shpool = NULL;
    if (role == VIRTIO_DEV_DRIVER)
    {
        /* Driver side owns the buffer pool. */
        void *shbuf = metal_io_phys_to_virt(shbuf_io, slot->shared_buf_pa);
        rpmsg_virtio_init_shm_pool(&slot->shpool, shbuf, SHARED_BUF_SIZE);
        shpool = &slot->shpool;
    }

    /* BE blocks here until FE sets DRIVER_OK. */
    struct virtio_device *vdev = remoteproc_create_virtio(rproc, (int)ch, (unsigned int)role, NULL);
    LogDebug(("platform_add_vdev: remoteproc_create_virtio vdev=%p\n", (void *)vdev));
    if (vdev == NULL)
    {
        return -ENODEV;
    }

    ret = rpmsg_init_vdev(&slot->rpvdev, vdev, NULL, shbuf_io, shpool);
    LogDebug(("platform_add_vdev: rpmsg_init_vdev ret=%d\n", ret));
    if (ret != 0)
    {
        remoteproc_remove_virtio(rproc, vdev);
        return ret;
    }

    /* Publish last: drain checks this flag. */
    __atomic_store_n(&slot->ready, 1U, __ATOMIC_RELEASE);
    *rdev = rpmsg_virtio_get_rpmsg_device(&slot->rpvdev);
    return 0;
}

int platform_remove_vdev(struct remoteproc *platform, unsigned int ch)
{
    if ((platform == NULL) || (ch >= PLATFORM_VDEV_MAX))
    {
        return -EINVAL;
    }

    struct remoteproc_priv *priv = platform->priv;
    struct platform_vdev *slot = &priv->vdev[ch];

    if (__atomic_load_n(&slot->ready, __ATOMIC_ACQUIRE) == 0U)
    {
        return -EAGAIN;
    }
    __atomic_store_n(&slot->ready, 0U, __ATOMIC_RELEASE);

    /* No more interrupts for this vdev. */
    if (slot->mfis_inited != 0U)
    {
        int mfis_ret = mfis_deinit(&slot->mfis);
        if (mfis_ret != 0)
        {
            LogError(("%s: mfis_deinit ch=%u failed, ret=%d\n", __func__, ch, mfis_ret));
        }
        slot->mfis_inited = 0U;
    }

    struct virtio_device *vdev = slot->rpvdev.vdev;
    rpmsg_deinit_vdev(&slot->rpvdev);

    metal_mutex_acquire(&platform->lock);
    remoteproc_remove_virtio(platform, vdev);
    metal_mutex_release(&platform->lock);

    slot->rsc_cached = 0U;
    slot->has_mfis   = 0U;
    LogDebug(("platform_remove_vdev: ch=%u removed\n", ch));
    return 0;
}

int platform_drain_vdev(struct remoteproc *platform, unsigned int ch)
{
    if ((platform == NULL) || (ch >= PLATFORM_VDEV_MAX))
    {
        return -EINVAL;
    }

    struct remoteproc_priv *priv = platform->priv;
    struct platform_vdev *slot = &priv->vdev[ch];

    if (__atomic_load_n(&slot->ready, __ATOMIC_ACQUIRE) == 0U)
    {
        return -EAGAIN;
    }

    /* Drains every pending buffer. */
    for (unsigned int v = 0; v < slot->num_vrings; v++)
    {
        (void)remoteproc_get_notification(platform, slot->vring_notifyid[v]);
    }
    return 0;
}
