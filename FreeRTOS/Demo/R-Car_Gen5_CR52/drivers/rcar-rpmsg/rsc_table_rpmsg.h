/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Shared resource table: one vdev entry per rcar_rpmsg vdev. */

#ifndef RSC_TABLE_V2_H
#define RSC_TABLE_V2_H

#include <stddef.h>
#include <stdint.h>
#include <openamp/open_amp.h>

#if defined __cplusplus
extern "C" {
#endif

#define NO_RESOURCE_ENTRIES         8

/* vdev entries (1..4). rsc_table_rpmsg.c scales to it. */
#define RSC_TABLE_VDEV_NUM          4U
struct fw_rsc_vdev_hdr
{
    uint32_t type;
    uint32_t id;
    uint32_t notifyid;
    uint32_t dfeatures;
    uint32_t gfeatures;
    uint32_t config_len;
    uint8_t  status;
    uint8_t  num_of_vrings;
    uint8_t  reserved[2];
}__attribute__((packed));

_Static_assert(sizeof(struct fw_rsc_vdev_hdr) == sizeof(struct fw_rsc_vdev),
               "fw_rsc_vdev_hdr layout diverges from struct fw_rsc_vdev");

/* One vdev with its two vrings. */
struct fw_rsc_vdev_entry
{
    struct fw_rsc_vdev_hdr   vdev;
    struct fw_rsc_vdev_vring vring0;
    struct fw_rsc_vdev_vring vring1;
};

/* Resource table */
struct remote_resource_table {
    unsigned int version;
    unsigned int num;
    unsigned int reserved[2];
    unsigned int offset[NO_RESOURCE_ENTRIES];
    struct fw_rsc_vdev_entry rpmsg_vdev[RSC_TABLE_VDEV_NUM];
}__attribute__((packed, aligned(0x100)));

/* No padding: entry 0 must match rpmsg-echo.c's rsc table layout. */
_Static_assert(sizeof(struct fw_rsc_vdev_entry) ==
               sizeof(struct fw_rsc_vdev) + 2U * sizeof(struct fw_rsc_vdev_vring),
               "fw_rsc_vdev_entry has hidden padding - layout diverges from rsc_table.h");

/* Table index = platform_link_t. Each table sits at the start of the shared
 * memory of its link, placed by its linker script. */
#define RSC_TABLE_LOCAL             0    /* lscript_rsc_table_rcar_rpmsg.ld */
#define RSC_TABLE_UCIE0             1    /* lscript_rsc_table_rcar_rpmsg_ucie0.ld */
#define RSC_TABLE_NUM               2

/* BE: copy the template into table @src_index (once). No-op if the app does
 * not link that table. */
void rcar_rsc_table_init(uint8_t src_index);
/* Table @rsc_id and its size, NULL if the app does not link it. */
void *rcar_rsc_table_get (int rsc_id, int *len);

#if defined __cplusplus
}
#endif

#endif /* RSC_TABLE_V2_H */