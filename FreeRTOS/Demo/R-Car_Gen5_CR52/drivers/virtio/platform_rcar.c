/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdint.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "platform_rcar.h"
#include "rsc_table.h"
#include <stdio.h>

#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>

/* Define shared DRAM area for each channel. */
#define SHARED_CH_RAM_BASE (0x40000000UL)
#define SHARED_CH_RAM_SIZE (0x80000000UL)

#define ATTR_NORM_NSHARED   NORM_NSHARED_NCACHE
#define ATTR_PRIV_RW        PRIV_RW_USER_RW
#define ATTR_NORM_NSHARED_PRIV_RW       (ATTR_NORM_NSHARED | ATTR_PRIV_RW)


/* Remote processor operations from r52 to a720. It defines
 * notification operation and remote processor managementi operations. */
extern const struct remoteproc_ops x5h_r_a_proc_ops;

extern size_t find_rsc(void *rsc_table, unsigned int rsc_type, unsigned int index);
/* RPMsg virtio shared buffer pool */

static struct remoteproc * platform_create_proc(struct remoteproc_priv *rproc_priv, int rsc_index);

/*----------------------------- RPMSG Platform implementation ----------------------------*/
/* Create platform
- Init remoteproc instance
- Map shared memory and resource table to the instance
- Set resource table
*/
static struct remoteproc * platform_create_proc(struct remoteproc_priv *rproc_priv, int rsc_index)
{
    struct remoteproc *rproc_inst = NULL;
    struct remoteproc *ret_rproc = NULL;

    void *rsc_table = NULL;
    int rsc_size = 0;
    int ret;
    metal_phys_addr_t pa;

    rproc_inst = (struct remoteproc *)pvPortMalloc(sizeof(struct remoteproc));
    

    if ((rproc_inst != NULL) && (rproc_priv != NULL))
    {
        struct mfis_channel *p_mfis_ch = rproc_priv->p_mfis_ch;
        /* Initialize the resource table on shared memory */
        if(rproc_priv->type == VIRTIO_BACK_END)
        {
            init_resource_table(rsc_index);
            rsc_table = get_resource_table(rsc_index, &rsc_size);
        }
        else
        {
            pa = rproc_priv->rsc_mem_pa;
            rsc_table = (void*)pa;
            rsc_size = rproc_priv->rsc_mem_size;
        }
        
        if ((rsc_table != NULL) && (rsc_size > 0))
        {
            if (remoteproc_init(rproc_inst,
                                &x5h_r_a_proc_ops,
                                (void *)rproc_priv) != NULL)
            {
                /* mmap resource table */
                pa = (metal_phys_addr_t)rsc_table;
                rsc_table = remoteproc_mmap(rproc_inst,
                                      &pa,
                                      NULL,
                                      rsc_size,
                                      ATTR_NORM_NSHARED_PRIV_RW,
                                      &(rproc_inst->rsc_io));

                /* mmap shared memory */
                pa = SHARED_CH_RAM_BASE;
                (void *)remoteproc_mmap(rproc_inst,
                                      &pa,
                                      NULL,
                                      SHARED_CH_RAM_SIZE,
                                      ATTR_NORM_NSHARED_PRIV_RW,
                                      NULL);

                /* parse resource table */
                ret = remoteproc_set_rsc_table(rproc_inst,
                                               rproc_inst->rsc_io->virt,
                                               rsc_size);
                
                if (ret == 0)
                {
                    ret_rproc = rproc_inst;
                }
            }
        }
    }

    /* cleanup on failure */
    if (ret_rproc == NULL)
    {
        if (rproc_inst != NULL)
        {
            remoteproc_remove(rproc_inst);
            vPortFree(rproc_inst);
        }
    }

    return ret_rproc;
}


/* Init platform: This function does:
- Init related HW module if required
- Create a `struct remoteproc` and return it to `platform` pointer
*/
int platform_init(struct remoteproc_priv *rproc_priv, struct remoteproc **platform)
{
    int ret = -EINVAL;
    unsigned long rsc_id = rproc_priv->p_mfis_ch->ch;
    struct remoteproc *rproc = NULL;
    
    if (platform != NULL)
    {
        rproc = platform_create_proc(rproc_priv, rsc_id);
        if (rproc != NULL)
        {
            *platform = rproc;
            ret = 0;
        }
    }
    return ret;
}

void platform_update_vring_addr(struct remoteproc *rproc, unsigned int vdev_id, int role)
{
	char *rsc_table = rproc->rsc_table;
	struct fw_rsc_vdev *vdev_rsc;
	size_t vdev_rsc_offset;
	unsigned int num_vrings, i;

    if (role == VIRTIO_DEV_DEVICE) {
		return;
    }

	metal_assert(rproc);
	metal_mutex_acquire(&rproc->lock);

	vdev_rsc_offset = find_rsc(rsc_table, RSC_VDEV, vdev_id);
    if (!vdev_rsc_offset) {
		goto err;
    }

	vdev_rsc = (struct fw_rsc_vdev *)(rsc_table + vdev_rsc_offset);
	num_vrings = vdev_rsc->num_of_vrings;

	for (i = 0; i < num_vrings; i++) {
		struct fw_rsc_vdev_vring *vring_rsc;
		struct remoteproc_priv *priv = rproc->priv;

		vring_rsc = &vdev_rsc->vring[i];

        if (vring_rsc->da == FW_RSC_U32_ADDR_ANY) {
			vring_rsc->da = priv->vring_mem_pa + i * priv->vring_mem_offset;
        }
	}

err:
	metal_mutex_release(&rproc->lock);
}

/* Create a RPMsg VirtIO device
- Create VirtIO device of remoteproc instance
- (Driver only) Initialize the shared buffer pool
- Init rpmsg virtio device with remoteproc device
- 
*/
struct rpmsg_device *
platform_create_rpmsg_vdev(struct remoteproc *platform,
                           unsigned int vdev_index,
                           unsigned int role,
                           void (*rst_cb)(struct virtio_device *vdev),
                           rpmsg_ns_bind_cb ns_bind_cb)
{
    struct rpmsg_device *ret_rpdev = NULL;
    struct rpmsg_virtio_device *rpmsg_vdev = NULL;
    struct virtio_device *vdev = NULL;
    /* MISRA-C:2012 Rule 11.5 deviation
    * Reason:
    *  - platform is an opaque context pointer defined by OpenAMP API
    *  - Cast is required to restore the original object type
    *  - platform always points to struct remoteproc created internally
    *  - No pointer arithmetic is performed
    */
    struct remoteproc *rproc = platform;
    struct metal_io_region *shbuf_io = NULL;
    struct rpmsg_virtio_shm_pool *shpool = NULL;
    struct remoteproc_priv *rproc_priv = rproc->priv;

    if (rproc_priv->type == VIRTIO_BACK_END)
    {
        rproc_priv->shared_buf_pa = SHARED_CH_RAM_BASE;
    }
    
    void *shbuf;
    int ret = 1;

    if (rproc != NULL)
    {
        rpmsg_vdev = metal_allocate_memory(sizeof(*rpmsg_vdev));
    }

    if (rpmsg_vdev != NULL)
    {
        shbuf_io = remoteproc_get_io_with_pa(rproc, rproc_priv->shared_buf_pa);
    }

    if (shbuf_io != NULL)
    {
        shbuf = metal_io_phys_to_virt(shbuf_io, rproc_priv->shared_buf_pa);

        if (rproc_priv->type == VIRTIO_FRONT_END)
        {
            shpool = metal_allocate_memory(sizeof(*shpool));
            platform_update_vring_addr(rproc, vdev_index, role);
            /* Only RPMsg virtio driver needs to initialize the shared buffers pool */
            rpmsg_virtio_init_shm_pool(shpool, shbuf, rproc_priv->shared_buf_size);
        }

        vdev = remoteproc_create_virtio(rproc,
                                        vdev_index,
                                        role,
                                        rst_cb);
    }

    if (vdev != NULL)
    {
        ret = rpmsg_init_vdev(rpmsg_vdev,
                                vdev,
                                ns_bind_cb,
                                shbuf_io,
                                shpool);
    }

    if (ret == 0)
    {
        ret_rpdev = rpmsg_virtio_get_rpmsg_device(rpmsg_vdev);
    }

    /* cleanup on failure */
    if ((ret_rpdev == NULL) && (rpmsg_vdev != NULL))
    {
        if (vdev != NULL)
        {
            remoteproc_remove_virtio(rproc, vdev);
        }
        metal_free_memory(rpmsg_vdev);
        metal_free_memory(shpool);
    }
    return ret_rpdev;
}


/* Wait for notification from driver
Return 0 if got noti
Otherwise return negative value
*/
int platform_poll(struct remoteproc *platform)
{
    struct remoteproc *rproc = platform;
    struct remoteproc_priv *priv = rproc->priv;
    struct mfis_channel* mfis = priv->p_mfis_ch;
    int ret = -1;

    if (0 != mfis->int_source)
    {
	    remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
        mfis->int_source = 0; // Reset int source to 0
        ret = 0;
    }

    return ret;
}

/* Deinit RPMsg device, call 2 functions:
- `rpmsg_deinit_vdev`
- `remoteproc_remove_virtio`
*/
void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, struct remoteproc *platform)
{
    (void)platform;
    (void)rpdev;
}

/* Remove platform resource of remote proc
- Remove `remote_proc` device
- Free memory, deinit HW
*/
void platform_cleanup(struct remoteproc *platform)
{
    (void)platform;
}
