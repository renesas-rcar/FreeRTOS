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
#include <openamp/remoteproc.h>
#include <openamp/rpmsg_virtio.h>

#include "FreeRTOS.h"
#include "virtio/r_virtio.h"
#include "platform_rcar.h"
#include "rsc_table.h"
#include "virtio-iommu-frontend/r_virtio_iommu_frontend.h"
#include "r_virtio_iommu.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define RPMSG_SERV_NAME "virtio-iommu"

#define RSC_MEM_PA_OFFSET           0x0000UL
#define RSC_MEM_SIZE                0x1000UL
#define VRING_MEM_PA_OFFSET         0x1000UL
#define VRING_MEM_OFFSET            0x4000UL
#define SHARED_BUF_PA_OFFSET        0x9000UL
#define SHARED_BUF_SIZE             0x40000UL

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept);

/***********************************************************************************************************************
 * ISR prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
struct virtio_iommu_frontend_instance_ctrl
{
    st_virtio_instance_ctrl_t *p_virtio_inst;
    st_virtio_endpoint_t *p_ept;
};

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
virtio_iommu_frontend_instance_ctrl_t * R_VIRTIO_IOMMU_Init(e_mfis_channel_t ch)
{
    virtio_iommu_frontend_instance_ctrl_t *result = ( virtio_iommu_frontend_instance_ctrl_t * ) pvPortMalloc( sizeof( virtio_iommu_frontend_instance_ctrl_t ) );
    uintptr_t rsc_table_address;
    int rsc_size = 0;

    rsc_table_address = (uintptr_t)get_resource_table(ch, &rsc_size);

    st_rsc_table_info_t rsc_table = {
        .rsc_mem_pa         = rsc_table_address + RSC_MEM_PA_OFFSET,
        .rsc_mem_size       = RSC_MEM_SIZE,
        .vring_mem_pa       = rsc_table_address + VRING_MEM_PA_OFFSET,
        .vring_mem_offset   = VRING_MEM_OFFSET,
        .shared_buf_pa      = rsc_table_address + SHARED_BUF_PA_OFFSET,
        .shared_buf_size    = SHARED_BUF_SIZE,
    };

    st_virtio_instance_ctrl_t *virtio_inst = NULL;
    
    virtio_inst = R_VIRTIO_FE_Create(ch, &rsc_table);
    if(virtio_inst != NULL)
    {
        st_virtio_context_t *p_comtext = ( st_virtio_context_t * ) pvPortMalloc( sizeof( st_virtio_context_t ) );
        p_comtext->sem = xSemaphoreCreateBinary();
        if (p_comtext->sem != NULL)
        {
            p_comtext->s_seq_counter = 0;
            result->p_ept = ( struct rpmsg_endpoint * ) pvPortMalloc( sizeof( struct rpmsg_endpoint ) );
            int ret = R_VIRTIO_CreateEP(virtio_inst, result->p_ept, RPMSG_SERV_NAME, R_VIRTIO_ResponseCb, rpmsg_service_unbind, (void*)p_comtext);
            if( ret == 0)
            {
                result->p_virtio_inst = virtio_inst;
            }
            else
            {
                vPortFree(result->p_ept);
                vPortFree(result);
            }
        }
        else
        {
            vPortFree(p_comtext);
        }
    }
    else
    {
        vPortFree(result);
    }

    return result;
}

int R_VIRTIO_IOMMU_DeInit(void)
{
    return 0;
}

int R_VIRTIO_IOMMU_Attach(st_smmu_streamid_instance_ctrl_t *p_ctrl)
{
    st_virtio_msg_t req  = {0};
    st_virtio_msg_t resp = {0};
    virtio_iommu_frontend_instance_ctrl_t *p_virtio_fe = (virtio_iommu_frontend_instance_ctrl_t *)p_ctrl->p_context;
    req.hdr.driver_id = (uint8_t)VIRTIO_SMMU_ID;

    st_virtio_smmu_payload_req_t *smmu = (st_virtio_smmu_payload_req_t *)req.payload;
    smmu->type = VIRTIO_IOMMU_T_ATTACH;
    /* Use memcpy to copy p_ctrl because some members may be const-qualified */
    (void)memcpy(&smmu->p_ctrl, p_ctrl, sizeof(st_smmu_streamid_instance_ctrl_t));
    
    int ret = R_VIRTIO_SendDataSync(p_virtio_fe->p_ept, &req, &resp, 0);
    return (ret == 0) ? (int)resp.hdr.status : ret;
}

int R_VIRTIO_IOMMU_Map(st_smmu_streamid_instance_ctrl_t *p_ctrl,
                        uint64_t va, uint64_t pa, uint64_t size, uint64_t attr)
{
    st_virtio_msg_t req  = {0};
    st_virtio_msg_t resp = {0};
    virtio_iommu_frontend_instance_ctrl_t *p_virtio_fe = (virtio_iommu_frontend_instance_ctrl_t *)p_ctrl->p_context;

    req.hdr.driver_id = (uint8_t)VIRTIO_SMMU_ID;

    st_virtio_smmu_payload_req_t *smmu = (st_virtio_smmu_payload_req_t *)req.payload;
    smmu->type = VIRTIO_IOMMU_T_MAP;
    /* Use memcpy to copy p_ctrl because some members may be const-qualified */
    (void)memcpy(&smmu->p_ctrl, p_ctrl, sizeof(st_smmu_streamid_instance_ctrl_t));
    smmu->va   = va;
    smmu->pa   = pa;
    smmu->size = size;
    smmu->attr = attr;
    
    int ret = R_VIRTIO_SendDataSync(p_virtio_fe->p_ept, &req, &resp, 0);
    return (ret == 0) ? (int)resp.hdr.status : ret;
}

int R_VIRTIO_IOMMU_UnMap(st_smmu_streamid_instance_ctrl_t *p_ctrl,
                          uint64_t va, uint64_t pa, uint64_t size)
{
    st_virtio_msg_t req  = {0};
    st_virtio_msg_t resp = {0};
    virtio_iommu_frontend_instance_ctrl_t *p_virtio_fe = (virtio_iommu_frontend_instance_ctrl_t *)p_ctrl->p_context;

    req.hdr.driver_id = (uint8_t)VIRTIO_SMMU_ID;

    st_virtio_smmu_payload_req_t *smmu = (st_virtio_smmu_payload_req_t *)req.payload;
    smmu->type = VIRTIO_IOMMU_T_UNMAP;
    /* Use memcpy to copy p_ctrl because some members may be const-qualified */
    (void)memcpy(&smmu->p_ctrl, p_ctrl, sizeof(st_smmu_streamid_instance_ctrl_t));
    smmu->va   = va;
    smmu->pa   = pa;
    smmu->size = size;

    int ret = R_VIRTIO_SendDataSync(p_virtio_fe->p_ept, &req, &resp, 0);
    return (ret == 0) ? (int)resp.hdr.status : ret;
}

int R_VIRTIO_IOMMU_Detach(st_smmu_streamid_instance_ctrl_t *p_ctrl)
{
    st_virtio_msg_t req  = {0};
    st_virtio_msg_t resp = {0};
    virtio_iommu_frontend_instance_ctrl_t *p_virtio_fe = (virtio_iommu_frontend_instance_ctrl_t *)p_ctrl->p_context;

    req.hdr.driver_id = (uint8_t)VIRTIO_SMMU_ID;

    st_virtio_smmu_payload_req_t *smmu = (st_virtio_smmu_payload_req_t *)req.payload;
    smmu->type = VIRTIO_IOMMU_T_DETACH;
    /* Use memcpy to copy p_ctrl because some members may be const-qualified */
    (void)memcpy(&smmu->p_ctrl, p_ctrl, sizeof(st_smmu_streamid_instance_ctrl_t));

    int ret = R_VIRTIO_SendDataSync(p_virtio_fe->p_ept, &req, &resp, 0);
    return (ret == 0) ? (int)resp.hdr.status : ret;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
}


