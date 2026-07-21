/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "virtio/r_virtio.h"
#include "platform_rcar.h"
#include "rsc_table.h"
#include "interrupts.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void Virtio_Task( void *pvParameters );
/***********************************************************************************************************************
 * ISR prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
struct st_virtio_instance_ctrl
{
    struct remoteproc             * platform;
    struct rpmsg_device           * rp_dev;
    e_mfis_channel_t                mfis_ch;
    uint8_t                         is_initialized;

    st_virtio_endpoint_t          * p_endpoit_list;
    uint16_t                        ept_num;
};

static st_virtio_instance_ctrl_t *virtio_be_inst[MFIS_CH_MAX] =
{
    [0 ... (MFIS_CH_MAX - 1)] = NULL
};

static st_virtio_instance_ctrl_t *virtio_fe_inst[MFIS_CH_MAX] =
{
    [0 ... (MFIS_CH_MAX - 1)] = NULL
};
/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
st_virtio_instance_ctrl_t * R_VIRTIO_BE_Create(e_mfis_channel_t mfis_ch)
{
    st_virtio_instance_ctrl_t *result;
    int ret;

    if(virtio_be_inst[mfis_ch] != NULL)
    {
        result = virtio_be_inst[mfis_ch];
    }
    else
    {
        result = ( st_virtio_instance_ctrl_t * ) pvPortMalloc( sizeof( st_virtio_instance_ctrl_t ) );
        struct mfis_channel *mfis_inst  = (struct mfis_channel *)pvPortMalloc(sizeof(struct mfis_channel));
        struct remoteproc_priv *rproc_priv = (struct remoteproc_priv *)pvPortMalloc(sizeof(struct remoteproc_priv));
        mfis_inst->ch = (unsigned int)mfis_ch;
        mfis_inst->type = MFIS_TYPE_RECEVER;
        mfis_inst->int_source = 0U;
        mfis_inst->recv_message = 0U;
        mfis_inst->cb_function = NULL;
        rproc_priv->p_mfis_ch = mfis_inst;
        rproc_priv->type = VIRTIO_BACK_END;
        ret = platform_init(rproc_priv, &(result->platform));
        if (ret != 0) {
            if (rproc_priv != NULL)
            {
                vPortFree(mfis_inst);
                vPortFree(rproc_priv);
            }
            vPortFree(result);
            result = NULL;
        } else {
            result->rp_dev = platform_create_rpmsg_vdev(result->platform, 0, VIRTIO_DEV_DEVICE, NULL, NULL);
            if (!(result->rp_dev))
            {
                vPortFree(result);
                result = NULL;
            }
            else
            {
                result->is_initialized = 1;
                virtio_be_inst[mfis_ch] = result;
                char task_name[16];
                snprintf(task_name, sizeof(task_name), "Virtio_BE_Task%d", mfis_ch);
                xTaskCreate( Virtio_Task, task_name, configMINIMAL_STACK_SIZE *10, result, ( configMAX_PRIORITIES - 1), NULL);
            }
        }
    }
 
    return result;
}

st_virtio_instance_ctrl_t * R_VIRTIO_FE_Create(e_mfis_channel_t mfis_ch, st_rsc_table_info_t * rsc_table_info)
{
    st_virtio_instance_ctrl_t *result;
    int ret;

    if(virtio_fe_inst[mfis_ch] != NULL)
    {
        result = virtio_fe_inst[mfis_ch];
    }
    else
    {
        result = ( st_virtio_instance_ctrl_t * ) pvPortMalloc( sizeof( st_virtio_instance_ctrl_t ) );
        struct mfis_channel *mfis_inst  = (struct mfis_channel *)pvPortMalloc(sizeof(struct mfis_channel));
        struct remoteproc_priv *rproc_priv = (struct remoteproc_priv *)pvPortMalloc(sizeof(struct remoteproc_priv));
        rproc_priv->rsc_mem_pa = rsc_table_info->rsc_mem_pa;
        rproc_priv->rsc_mem_size = rsc_table_info->rsc_mem_size;
        rproc_priv->vring_mem_pa = rsc_table_info->vring_mem_pa;
        rproc_priv->vring_mem_offset = rsc_table_info->vring_mem_offset;
        rproc_priv->shared_buf_pa = rsc_table_info->shared_buf_pa;
        rproc_priv->shared_buf_size = rsc_table_info->shared_buf_size;
        mfis_inst->ch = (unsigned int)mfis_ch;
        mfis_inst->type = MFIS_TYPE_SENDER;
        mfis_inst->int_source = 0U;
        mfis_inst->recv_message = 0U;
        mfis_inst->cb_function = NULL;
        rproc_priv->p_mfis_ch = mfis_inst;
        rproc_priv->type = VIRTIO_FRONT_END;
        
        ret = platform_init(rproc_priv, &(result->platform));
        if (ret != 0) {
            if (rproc_priv != NULL)
            {
                vPortFree(mfis_inst);
                vPortFree(rproc_priv);
            }
            vPortFree(result);
            result = NULL;
        } else {
            result->rp_dev = platform_create_rpmsg_vdev(result->platform, 0, VIRTIO_DEV_DRIVER, NULL, NULL);
            if (!(result->rp_dev))
            {
                vPortFree(result);
                result = NULL;
            }
            else
            {
                result->is_initialized = 1;
                virtio_fe_inst[mfis_ch] = result;
                char task_name[16];
                snprintf(task_name, sizeof(task_name), "Virtio_FE_Task%d", mfis_ch);
                xTaskCreate( Virtio_Task, task_name, configMINIMAL_STACK_SIZE *10, result, ( configMAX_PRIORITIES - 1), NULL);
            }
        }
    }
 
    return result;
}

uint8_t R_VIRTIO_Release(st_virtio_instance_ctrl_t * p_ctrl)
{
    platform_release_rpmsg_vdev(p_ctrl->rp_dev, p_ctrl->platform);
    platform_cleanup(p_ctrl->platform);

    return 0;
}

uint8_t R_VIRTIO_CreateEP(st_virtio_instance_ctrl_t *p_vdev_ctrl, st_virtio_endpoint_t * p_ept,
		    const char *name, virtio_ept_cb ept_cb, virtio_ns_unbind_cb unbind_cb, st_virtio_context_t *priv)
{
    int ret;
    p_ept->priv = priv;
    ret = rpmsg_create_ept(p_ept, p_vdev_ctrl->rp_dev, name,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   ept_cb,
                   unbind_cb);
    return ret;
}

uint8_t R_VIRTIO_ReleaseEP(st_virtio_endpoint_t * p_ept)
{
    rpmsg_destroy_ept(p_ept);
    return 0;
}

uint8_t R_VIRTIO_SendData(struct rpmsg_endpoint *ept, const void *data, int len)
{
    int ret = rpmsg_send(ept, data, len);
    return ret;
}

int R_VIRTIO_SendDataSync(struct rpmsg_endpoint *ept,
                          st_virtio_msg_t       *req,
                          st_virtio_msg_t       *resp,
                          uint32_t               timeout_ms)
{
    if (ept == NULL || req == NULL)
    {
        return -EINVAL;
    }
    st_virtio_context_t *p_context = ept->priv;
    /* Fill request header fields managed by the virtio layer */
    req->hdr.msg_type = (uint8_t)VIRTIO_MSG_REQUEST;
    req->hdr.status   = 0;

    
        req->hdr.seq_id       = p_context->s_seq_counter ++;
        p_context->seq_id   = req->hdr.seq_id;
        p_context->resp_buf = resp;
        /* Ensure the binary semaphore starts in the taken state */
        xSemaphoreTake(p_context->sem, 0);

    

    /* Send the request frame */
    int ret = rpmsg_send(ept, req, sizeof(st_virtio_msg_t));

    /* Block until R_VIRTIO_FE_ResponseCb gives the semaphore or timeout */
    TickType_t ticks = (timeout_ms == 0U)
        ? pdMS_TO_TICKS(VIRTIO_SEND_TIMEOUT_MS)
        : pdMS_TO_TICKS(timeout_ms);

    ret = (xSemaphoreTake(p_context->sem, ticks) == pdTRUE) ? 0 : -ETIMEDOUT;

    return ret;
}

int R_VIRTIO_ResponseCb(struct rpmsg_endpoint *ept,
                            void *data, size_t len,
                            uint32_t src, void *priv)
{
    (void)priv;
    (void)src;
    st_virtio_context_t *p_context = ept->priv;

    if (len < sizeof(st_virtio_msg_header_t))
    {
        return RPMSG_SUCCESS;
    }

    const st_virtio_msg_t *msg = (const st_virtio_msg_t *)data;
    if (msg->hdr.msg_type != (uint8_t)VIRTIO_MSG_RESPONSE)
    {
        return RPMSG_SUCCESS;
    }

    /* Find the pending slot whose seq_id matches this response */

    if (p_context->resp_buf != NULL)
        {
            size_t copy_len = (len < sizeof(st_virtio_msg_t)) ? len : sizeof(st_virtio_msg_t);
            memcpy(p_context->resp_buf, msg, copy_len);
        }
        /* Unblock the SendDataSync caller */
        xSemaphoreGive(p_context->sem);
    

    return RPMSG_SUCCESS;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static void Virtio_Task( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    st_virtio_instance_ctrl_t *p_virtio_inst =  (st_virtio_instance_ctrl_t *)pvParameters;

    for( ;; )
    {
        (void)platform_poll(p_virtio_inst->platform);

        vTaskDelay(1);
    }
}


typedef uint32_t (*VirtIO_Handler)(st_virtio_msg_t *msg);

static uint32_t virtio_default_handler (st_virtio_msg_t *msg) {
    (void)msg;
    uint32_t ret = 0;
    return ret;
}

extern uint32_t VirtIO_SMMU_Handler(st_virtio_msg_t *msg);

static const VirtIO_Handler VirtIO_Handler_tbl[] = {
    [VIRTIO_SMMU_ID]    = VirtIO_SMMU_Handler,
    [VIRTIO_GPIO_ID]    = virtio_default_handler,
    [VIRTIO_I2C_ID]     = virtio_default_handler,
};

uint32_t VirtIO_driver_handler(st_virtio_msg_t *msg) {
    if (msg->hdr.driver_id >= (uint8_t)(sizeof(VirtIO_Handler_tbl) / sizeof(VirtIO_Handler_tbl[0])))
    {
        return 1U;
    }
    return VirtIO_Handler_tbl[msg->hdr.driver_id](msg);
}
