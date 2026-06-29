/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_RCAR_H
#define PLATFORM_RCAR_H

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>
#include "mfis/mfis.h"

/* Cortex R52 memory attributes */
#define DEVICE_SHARED       ((uint32_t)0x00000001U) /* device, shareable */
#define DEVICE_NONSHARED    ((uint32_t)0x00000010U) /* device, non shareable */
#define NORM_NSHARED_NCACHE ((uint32_t)0x00000008U) /* Non cacheable  non shareable */
#define NORM_SHARED_NCACHE  ((uint32_t)0x0000000CU) /* Non cacheable shareable */
#define PRIV_RW_USER_RW     ((uint32_t)0x00000003U<<8U) /* Full Access */
#define ATTR_NORM_SHARED_NCACHE_PRIV_RW_USER_RW		(NORM_SHARED_NCACHE | PRIV_RW_USER_RW)
#if defined __cplusplus
extern "C" {
#endif

typedef enum virtio_type
{
	VIRTIO_FRONT_END = VIRTIO_DEV_DRIVER,
	VIRTIO_BACK_END = VIRTIO_DEV_DEVICE,
} virtio_type_t;

struct remoteproc_priv {
	metal_phys_addr_t rsc_mem_pa; /**< rsc table physical address */
	size_t rsc_mem_size; /**< Size of the rsc table */
	metal_phys_addr_t vring_mem_pa; /**< vring physical address */
	size_t vring_mem_offset; /**< Offset of each vring */
	metal_phys_addr_t shared_buf_pa; /**< Shared buffer physical address */
	size_t shared_buf_size; /**< Size of the shared buffer */

	struct metal_io_region *shm_io; /**< pointer to sh mem i/o region */

	struct remoteproc_mem shm_mem; /**< shared memory */

	struct mfis_channel *p_mfis_ch;

	virtio_type_t type;
};

/**
 * platform_init - initialize the platform
 *
 * Initialize the platform.
 *
 * @argc: number of arguments
 * @argv: array of the input arguments
 * @platform: pointer to store the platform data pointer
 *
 * return 0 for success or negative value for failure
 */
int platform_init(struct remoteproc_priv *rproc_priv, struct remoteproc **platform);

/**
 * platform_create_rpmsg_vdev - create rpmsg vdev
 *
 * Create rpmsg virtio device, and return the rpmsg virtio
 * device pointer.
 *
 * @platform: pointer to the private data
 * @vdev_index: index of the virtio device, there can more than one vdev
 *              on the platform.
 * @role: virtio driver or virtio device of the vdev
 * @rst_cb: virtio device reset callback
 * @ns_bind_cb: rpmsg name service bind callback
 *
 * return pointer to the rpmsg virtio device
 */
struct rpmsg_device *
platform_create_rpmsg_vdev(struct remoteproc *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb);

/**
 * platform_poll - platform poll function
 *
 * @platform: pointer to the platform
 *
 * return negative value for errors, otherwise 0.
 */
int platform_poll(struct remoteproc *platform);

/**
 * platform_release_rpmsg_vdev - release rpmsg virtio device
 *
 * @rpdev: pointer to the rpmsg device
 */
void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, struct remoteproc *platform);

/**
 * platform_cleanup - clean up the platform resource
 *
 * @platform: pointer to the platform
 */
void platform_cleanup(struct remoteproc *platform);


#if defined __cplusplus
}
#endif

#endif /* PLATFORM_RCAR_H */
