/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>

#include "platform_info_common.h"

/* Cortex R52 memory attributes */
#define DEVICE_SHARED       0x00000001U /* device, shareable */
#define DEVICE_NONSHARED    0x00000010U /* device, non shareable */
#define NORM_NSHARED_NCACHE 0x00000008U /* Non cacheable  non shareable */
#define NORM_SHARED_NCACHE  0x0000000CU /* Non cacheable shareable */
#define PRIV_RW_USER_RW     (0x00000003U<<8U) /* Full Access */
#define NO_USING_MFIS       (0xFFU)
#define NOTIFY_FLAG_OFFSET  (0x500U)

#if defined __cplusplus
extern "C" {
#endif

struct remoteproc_priv {
    uint32_t shm_addr;                  /* Share mem address of 2 proc */
    struct mfis_channel *p_mfis;        /* MFIS trigger channel of 2 proc */
    uint32_t type;                      /* VirtIO dev type */
};

#if defined __cplusplus
}
#endif

#endif /* PLATFORM_INFO_H_ */
