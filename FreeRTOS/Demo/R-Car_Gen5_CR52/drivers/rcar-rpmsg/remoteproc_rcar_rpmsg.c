/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <metal/io.h>
#include <openamp/remoteproc.h>
#include "platform_rcar_rpmsg.h"

/* Logging Function include. */
#include "logging_stack.h"

/* prototype */
void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
                               size_t size, unsigned int flags);

/* Find the vdev owning a notify id (ids cached by platform_add_vdev()). */
static struct platform_vdev *notifyid_to_vdev(struct remoteproc_priv *priv, uint32_t id)
{
    for (unsigned int i = 0; i < PLATFORM_VDEV_MAX; i++)
    {
        struct platform_vdev *slot = &priv->vdev[i];

        if (slot->rsc_cached == 0U)
        {
            continue;
        }
        if (slot->vdev_notifyid == id)
        {
            return slot;
        }
        for (unsigned int v = 0; v < slot->num_vrings; v++)
        {
            if (slot->vring_notifyid[v] == id)
            {
                return slot;
            }
        }
    }
    return NULL;
}

/* VA == PA. Weak: drivers/virtio/remoteproc_rcar.c has the same one. */
__attribute__((weak)) void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
                                                     size_t size, unsigned int flags)
{
    metal_unused(size);
    metal_unused(flags);

    va = (void *)pa;

    return va;
}

/* MFIS is set up per vdev in platform_add_vdev(). */
static struct remoteproc *
x5h_proc_init(struct remoteproc *rproc,
              const struct remoteproc_ops *ops,
              void *arg)
{
    (void)ops;

    if ((rproc == NULL) || (arg == NULL))
    {
        return NULL;
    }
    return rproc;
}


/* Deinit every MFIS channel. */
static void x5h_proc_remove(struct remoteproc *rproc)
{
    struct remoteproc_priv *priv = rproc->priv;

    for (unsigned int i = 0; i < PLATFORM_VDEV_MAX; i++)
    {
        if (priv->vdev[i].mfis_inited != 0U)
        {
            int mfis_ret = mfis_deinit(&priv->vdev[i].mfis);
            if (mfis_ret != 0)
            {
                LogError(("%s: mfis_deinit ch=%u failed, ret=%d\n", __func__, i, mfis_ret));
            }
            priv->vdev[i].mfis_inited = 0U;
        }
    }
}

/* Register a memory region (and its io region) with the remoteproc. */
static void *
x5h_proc_mmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
              metal_phys_addr_t *da, size_t size,
              unsigned int attribute, struct metal_io_region **io)
{
    struct remoteproc_mem *mem = NULL;
    struct metal_io_region *tmpio = NULL;
    metal_phys_addr_t lpa;
    metal_phys_addr_t lda;
    void *ret = NULL;

    lda = *da;
    lpa = *pa;

    if ((lpa != METAL_BAD_PHYS) || (lda != METAL_BAD_PHYS))
    {
        if (lpa == METAL_BAD_PHYS)
        {
            lpa = lda;
        }

        if (lda == METAL_BAD_PHYS)
        {
            lda = lpa;
        }

        if (attribute == 0U)
        {
            attribute = ATTR_NORM_SHARED_NCACHE_PRIV_RW_USER_RW;
        }

        mem = metal_allocate_memory(sizeof(*mem));
        if (mem != NULL)
        {
            mem->pa = lpa;

            tmpio = metal_allocate_memory(sizeof(*tmpio));
            if (tmpio != NULL)
            {
                /* MISRA-C:2012 Rule 11.x deviation
                 * Casting physical address to pointer is platform specific.
                 * On this platform, VA == PA mapping is guaranteed by design.
                 */
                metal_io_init(tmpio,
                              (void *)lpa,
                              &mem->pa,
                              size,
                              (sizeof(metal_phys_addr_t) << 3),
                              attribute,
                              NULL);

                remoteproc_init_mem(mem, NULL, lpa, lda, size, tmpio);
                remoteproc_add_mem(rproc, mem);

                if (io != NULL)
                {
                    *io = tmpio;
                }

                *pa = lpa;
                *da = lda;

                ret = metal_io_phys_to_virt(tmpio, mem->pa);
            }
            else
            {
                metal_free_memory(mem);
            }
        }
    }

    return ret;
}


/* Raise the vdev's MFIS interrupt on the peer. */
static int x5h_proc_notify(struct remoteproc *rproc, uint32_t id)
{
    struct remoteproc_priv *priv = rproc->priv;
    uint16_t irq_id = (uint16_t)id;
    irq_id &= (uint16_t)0x7FFF;

    struct platform_vdev *slot = notifyid_to_vdev(priv, id);

    if (slot == NULL)
    {
        LogDebug(("x5h_proc_notify: no vdev owns notifyid %u\n", id));
        return -1;
    }
    if (slot->mfis_inited == 0U)
    {
        /* PLATFORM_NO_MFIS: peer polls. */
        return 0;
    }

    int ret = mfis_trigger_interrupt(&slot->mfis, irq_id);
    LogDebug(("x5h_proc_notify: id=%u ch=%u irq_id=%u ret=%d\n", id, slot->mfis.ch, irq_id, ret));
    return ret;
}

/* remoteproc ops */
const struct remoteproc_ops rcar_rpmsg_proc_ops = {
    .init   = &x5h_proc_init,
    .remove = &x5h_proc_remove,
    .mmap   = &x5h_proc_mmap,
    .notify = &x5h_proc_notify,
    // Do not implement since CR can boot independently
    .start  = NULL,
    .stop   = NULL,
    .shutdown = NULL,
};