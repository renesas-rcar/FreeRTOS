/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Shared resource tables, one per link, same template: one vdev entry per
 * rcar_rpmsg vdev. */

#include <string.h>
#include <openamp/open_amp.h>
#include "rsc_table_rpmsg.h"

/* Logging Function include. */
#include "logging_stack.h"

/* Defined by the rsc table linker script of each link the app links
 * (weak: an app may use one link only). */
extern char __resource_table_start       __attribute__((weak));   /* local */
extern char __resource_table_end         __attribute__((weak));
extern char __resource_table_ucie0_start __attribute__((weak));   /* UCIe CH0 */
extern char __resource_table_ucie0_end   __attribute__((weak));

#define RPMSG_VDEV_DFEATURES (1 << VIRTIO_RPMSG_F_NS)

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_ 7

#define NUM_VRINGS 0x02
#define VRING_ALIGN 0x1000
#ifndef RING_TX
#define RING_TX FW_RSC_U32_ADDR_ANY
#endif /* !RING_TX */
#ifndef RING_RX
#define RING_RX FW_RSC_U32_ADDR_ANY
#endif /* RING_RX */
#define VRING_SIZE 256

/* vdev entries in the table. */
#define NUM_TABLE_ENTRIES RSC_TABLE_VDEV_NUM

/* One vdev entry: notify ids notify_base (vdev), +1, +2 (vrings).
 * Ranges must not overlap between entries. */
#define VDEV_ENTRY(notify_base)                                             \
    {                                                                       \
        .vdev = {RSC_VDEV, VIRTIO_ID_RPMSG_, (notify_base),                 \
                 RPMSG_VDEV_DFEATURES, 0, 0, 0, NUM_VRINGS, {0, 0}},         \
        .vring0 = {RING_TX, VRING_ALIGN, VRING_SIZE, (notify_base) + 1, 0}, \
        .vring1 = {RING_RX, VRING_ALIGN, VRING_SIZE, (notify_base) + 2, 0}, \
    }

#if RSC_TABLE_VDEV_NUM > 4
#error "RSC_TABLE_VDEV_NUM > 4: add RSC_ENTRY_5 etc. below, and check PLATFORM_VDEV_MAX in platform_rcar_rpmsg.h matches."

#endif

#if RSC_TABLE_VDEV_NUM >= 2
#define RSC_ENTRY_2(x) , x
#define RSC_PAD_2
#else
#define RSC_ENTRY_2(x)
#define RSC_PAD_2 , 0U
#endif
#if RSC_TABLE_VDEV_NUM >= 3
#define RSC_ENTRY_3(x) , x
#define RSC_PAD_3
#else
#define RSC_ENTRY_3(x)
#define RSC_PAD_3 , 0U
#endif
#if RSC_TABLE_VDEV_NUM >= 4
#define RSC_ENTRY_4(x) , x
#define RSC_PAD_4
#else
#define RSC_ENTRY_4(x)
#define RSC_PAD_4 , 0U
#endif

#define RSC_TABLE_OFFSETS                                                        \
    offsetof(struct remote_resource_table, rpmsg_vdev[0].vdev)                   \
    RSC_ENTRY_2(offsetof(struct remote_resource_table, rpmsg_vdev[1].vdev))      \
    RSC_ENTRY_3(offsetof(struct remote_resource_table, rpmsg_vdev[2].vdev))      \
    RSC_ENTRY_4(offsetof(struct remote_resource_table, rpmsg_vdev[3].vdev))      \
    RSC_PAD_2 RSC_PAD_3 RSC_PAD_4, 0U, 0U, 0U, 0U

#define RSC_TABLE_VDEVS               \
    VDEV_ENTRY(1)                     \
    RSC_ENTRY_2(VDEV_ENTRY(5))        \
    RSC_ENTRY_3(VDEV_ENTRY(9))        \
    RSC_ENTRY_4(VDEV_ENTRY(13))

/* Workaround (FlashWriter/IPL cannot load SDRAM): `resources` sits in the
 * rsc table section, `resources_data` is the template BE copies at run
 * time. */

static struct remote_resource_table __attribute__((section(".resource_table"))) resources = {
    /* Version */
    1,

    /* Number of table entries */
    NUM_TABLE_ENTRIES,
    /* reserved fields */
    {
        0,
        0,
    },

    /* Offsets of rsc entries, one per vdev */
    {
        RSC_TABLE_OFFSETS,
    },

    /* vdev entries */
    {
        RSC_TABLE_VDEVS,
    },
};

static const struct remote_resource_table resources_data = {
    /* Version */
    1,

    /* Number of table entries */
    NUM_TABLE_ENTRIES,
    /* reserved fields */
    {
        0,
        0,
    },

    /* Offsets of rsc entries, one per vdev */
    {
        RSC_TABLE_OFFSETS,
    },

    /* vdev entries */
    {
        RSC_TABLE_VDEVS,
    },
};


/* UCIe CH0 table, in the UCIe shared memory.  */
static struct remote_resource_table __attribute__((section(".resource_table_ucie0"), used)) resources_ucie0 = {
    /* Version */
    1,

    /* Number of table entries */
    NUM_TABLE_ENTRIES,
    /* reserved fields */
    {
        0,
        0,
    },

    /* Offsets of rsc entries, one per vdev */
    {
        RSC_TABLE_OFFSETS,
    },

    /* vdev entries */
    {
        RSC_TABLE_VDEVS,
    },
};

static uint8_t s_rsc_table_inited[RSC_TABLE_NUM];

/* Linker range of table @rsc_id; start NULL if the app does not link it. */
static char *table_range(int rsc_id, size_t *len)
{
    char *start = NULL;
    char *end = NULL;

    if (rsc_id == RSC_TABLE_LOCAL)
    {
        start = &__resource_table_start;
        end   = &__resource_table_end;
    }
    else if (rsc_id == RSC_TABLE_UCIE0)
    {
        start = &__resource_table_ucie0_start;
        end   = &__resource_table_ucie0_end;
    }
    else
    {
        LogError(("[%s] This resource ID is not supported\n", __func__));
    }

    if ((start != NULL) && (end > start))
    {
        ptrdiff_t diff = end - start;
        *len = (size_t)diff;
    }
    else
    {
        *len = 0U;
    }
    return (*len != 0U) ? start : NULL;
}

void rcar_rsc_table_init(uint8_t src_index)
{
    size_t len;
    char *start = table_range((int)src_index, &len);

    if ((start == NULL) || (s_rsc_table_inited[src_index] != 0U))
    {
        return;
    }

    (void)memcpy((void *)start, &resources_data,
                 (len < sizeof(resources_data)) ? len : sizeof(resources_data));
    s_rsc_table_inited[src_index] = 1U;
}

void *rcar_rsc_table_get(int rsc_id, int *len)
{
    size_t n;
    char *start = table_range(rsc_id, &n);

    *len = (int)n;
    return (void *)start;
}