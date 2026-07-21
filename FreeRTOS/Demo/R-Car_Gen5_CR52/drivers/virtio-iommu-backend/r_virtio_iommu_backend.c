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

#include "FreeRTOS.h"
#include "virtio/r_virtio.h"
#include "virtio-iommu-backend/r_virtio_iommu_backend.h"
#include "r_virtio_iommu.h"
#include "platform_rcar.h"
#include "rsc_table.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define VIRTIO_SERVICE_NAME         "virtio-iommu"

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv);
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept);
uint32_t VirtIO_SMMU_Handler(st_virtio_msg_t *msg);
/***********************************************************************************************************************
 * ISR prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
virtio_iommu_instance_ctrl_t * R_VIRTIO_IOMMU_Backend_Init(e_mfis_channel_t ch)
{
    virtio_iommu_instance_ctrl_t *result = NULL;
    st_virtio_instance_ctrl_t *virtio_inst = NULL;
    st_virtio_endpoint_t *lept = ( struct rpmsg_endpoint * ) pvPortMalloc( sizeof( struct rpmsg_endpoint ) );
    virtio_inst = R_VIRTIO_BE_Create(ch);
    if(virtio_inst != NULL)
    {
        int ret = R_VIRTIO_CreateEP(virtio_inst, lept, VIRTIO_SERVICE_NAME, rpmsg_endpoint_cb, rpmsg_service_unbind, NULL);
        if( ret == 0)
        {
            result = (virtio_iommu_instance_ctrl_t *)virtio_inst;
        }
    }

    return result;
}

uint8_t R_VIRTIO_IOMMU_Backend_DeInit(virtio_iommu_instance_ctrl_t * p_inst)
{
    (void)p_inst;
    return 0;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv)
{
    (void)priv;
    (void)src;
    (void)len;

    st_virtio_msg_t *req = (st_virtio_msg_t *)data;

    /* Dispatch to the appropriate driver handler; result stored in req->hdr.status */
    VirtIO_driver_handler(req);

    /* Build response in-place: flip msg_type, keep seq_id unchanged for FE matching */
    req->hdr.msg_type = (uint8_t)VIRTIO_MSG_RESPONSE;

    R_VIRTIO_SendData(ept, req, sizeof(st_virtio_msg_t));

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
}

/**----------------------------- req paser -----------------------------*/

static int virtio_smmu_attach(st_virtio_smmu_payload_req_t* data);
static int virtio_smmu_detach(st_virtio_smmu_payload_req_t* data);
static int virtio_smmu_map(st_virtio_smmu_payload_req_t* data);
static int virtio_smmu_unmap(st_virtio_smmu_payload_req_t* data);
static int virtio_smmu_probe(st_virtio_smmu_payload_req_t* data);

typedef int (*virtio_smmu_hdl)(st_virtio_smmu_payload_req_t*);

static const virtio_smmu_hdl virtio_smmu_handler_tbl[] = {
    [VIRTIO_IOMMU_T_ATTACH] = virtio_smmu_attach,
    [VIRTIO_IOMMU_T_DETACH] = virtio_smmu_detach,
    [VIRTIO_IOMMU_T_MAP]    = virtio_smmu_map,
    [VIRTIO_IOMMU_T_UNMAP]  = virtio_smmu_unmap,
    [VIRTIO_IOMMU_T_PROBE]  = virtio_smmu_probe,
};

uint32_t VirtIO_SMMU_Handler(st_virtio_msg_t *msg) {
    uint32_t ret;
    st_virtio_smmu_payload_req_t *smmu_payload = (st_virtio_smmu_payload_req_t*)(msg->payload);

    if (msg->hdr.driver_id != VIRTIO_SMMU_ID || smmu_payload->type > VIRTIO_IOMMU_T_PROBE) {
        ret = 1;
    }
    else {
        ret = virtio_smmu_handler_tbl[smmu_payload->type](smmu_payload);
    }

    /* Write result into the message header for the generic response path */
    msg->hdr.status = ret;

    return ret;
}

static int virtio_smmu_attach(st_virtio_smmu_payload_req_t* data)
{
    int ret = 0;
    /*----- Init and enable SMMU -----*/
    R_SMMU_Init(SMMU_PERW, false);
    R_SMMU_InvalidateTLB(SMMU_PERW, false);
    R_SMMU_Enable(SMMU_PERW, false);
    st_smmu_streamid_instance_ctrl_t *p_ctrl = &(data->p_ctrl);
    ret = R_SMMU_Attach(&(data->p_ctrl));
    return ret;
}

static int virtio_smmu_detach(st_virtio_smmu_payload_req_t* data)
{
    int ret = 0;
    R_SMMU_Detach(&(data->p_ctrl));
    return ret;
}

static int virtio_smmu_map(st_virtio_smmu_payload_req_t* data)
{
    int ret = 0;
    st_smmu_cmd_t cmd_inv_tlb = {0};

    ret = R_SMMU_Map(&(data->p_ctrl), data->va, data->pa, data->size, data->attr);
    
    if (ret == 0) {
        cmd_inv_tlb.opcode = CMDQ_OP_TLBI_NSNH_ALL;
        ret = R_SMMU_IssueCommand(data->p_ctrl.smmu_domain, data->p_ctrl.is_secure, &cmd_inv_tlb, 1);
    }

    return ret;
}

static int virtio_smmu_unmap(st_virtio_smmu_payload_req_t* data)
{
    int ret = 0;
    st_smmu_cmd_t cmd_inv_tlb = {0};

    R_SMMU_Unmap(&(data->p_ctrl), data->va, data->pa, data->size);

    cmd_inv_tlb.opcode = CMDQ_OP_TLBI_NSNH_ALL;
    ret = R_SMMU_IssueCommand(data->p_ctrl.smmu_domain, data->p_ctrl.is_secure, &cmd_inv_tlb, 1);

    return ret;
}

static int virtio_smmu_probe(st_virtio_smmu_payload_req_t* data)
{
    (void)data;
    int ret = 0;
    return ret;
}
