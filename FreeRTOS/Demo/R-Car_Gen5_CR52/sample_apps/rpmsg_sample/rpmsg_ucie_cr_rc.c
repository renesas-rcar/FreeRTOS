/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @brief This sample is for demonstrate rpmsg communicate via ucie
 *        between two CR core of chiplet. And this is for core run
 *        with UCIe RC mode.
 */

#include <stdio.h>
#include <errno.h>
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "platform_info.h"
#include "platform_info_common.h"
#include "rsc_table.h"
#include "pfc/r_pfc_api.h"
#include "smmu/smmu.h"
#include "ucie/r_ucie.h"

#define RPMSG_SERVICE_NAME         "rpmsg-client-sample"
#define SHUTDOWN_MSG    0xEF56A55A

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;

#define hello_msg "Hello world from RC CR52/Free-RTOS!"
#define goodbye_msg "Good bye!"

#if (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
#define RCTBUBYPSEN_ADDRESS     0x18B41010  /* Realtime Core TBU bypass enable register address */
#define MASK 0x00000003
#else
#define RCTBUBYPSEN_ADDRESS     0x18B47800  /* Realtime Core TBU bypass enable register address */
#define MASK 0x00000FFF
#endif

#ifndef UCIE_CH
#define UCIE_CH     UCIE_CH0
#endif

#define UCIE_D2D_MEM    (0x20000000000ULL + UCIE_CH*0x4000000000ULL)

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                 uint32_t src, void *priv)
{
    char payload[RPMSG_BUFFER_SIZE];
    (void)priv;
    (void)src;

    memset(payload, 0, RPMSG_BUFFER_SIZE);
    memcpy(payload, data, len);

    printf("Incoming msg: %s\r\n", payload);

    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    printf("unexpected Remote endpoint destroy\r\n");
    shutdown_req = 1;
}


/*----------------------------------------------------------------------------*/
static void prvSetupHardware( void )
{
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
    state.  Interrupts are automatically enabled when the scheduler is
    started. */
    portDISABLE_INTERRUPTS();

    Irq_Setup();

    (void)pfcInitModules(getModuleConfigs());

    R_UCIE_Config(UCIE_CH, UCIE_MODE_RC, LINKSPEED_4GTPS, true);
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
void prvGetNotifyTask( void *pvParameters )
{
    while(1) {
        platform_poll(pvParameters);
        vTaskDelay(1);
    }
}

void echoTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    int ret;
    void *platform;
    struct rpmsg_device *rpdev;

    void *rsc_table;
    int len;

    /* Root-Complex */
    /* Check ucie linkup */
    while (R_UCIE_Get_Linkup_Status(UCIE_CH) != LINKUP_SUCCESS){
        __asm__ volatile("nop");
    }
    vTaskDelay(500);

    rsc_table = get_resource_table(0, &len);

    uint32_t address_check = (uint32_t)rsc_table + 0x200;

    /* Setup outbound ATU */
    st_ucie_iatu_cfg_t cfg = {
        .ucie_ch = UCIE_CH,
        .rgn = IATU_RGN0,
        .type = IATU_OUTBOUND,
#if (BOARD == MDP_AIACC_RFS2 || BOARD == X5H_RFS2)
        .mSrcAddr = (uint64_t)(uintptr_t)rsc_table,
#else
        .mSrcAddr = UCIE_D2D_MEM,
#endif
        .mDestAddr = (uint64_t)(uintptr_t)rsc_table,
        .size = 0x100000
    };

    R_UCIE_IATU_SetRegion(&cfg);

#if (BOARD != MDP_AIACC_RFS2 && BOARD != X5H_RFS2)
    /* Setup SMMU */
    bool is_secure = true;

#if (BOARD == MDP_AIACC_HIL)
    uint32_t streamId[] = {
        0x00800,
        0x00900,
    };
#else // BOARD == MDP_AIACC_HIL
    uint32_t streamId[] = {
        0x000,
        0xC00,
    };
#endif

    st_smmu_streamid_instance_ctrl_t smmu_ctrl = {
        .smmu_domain = SMMU_RT,
        .is_secure  = is_secure,
    };
    
    R_SMMU_Init(SMMU_RT, is_secure);

    for (uint8_t i = 0; i < sizeof(streamId)/sizeof(uint32_t); i ++) {
        smmu_ctrl.stream_id = streamId[i];

        R_SMMU_Attach(&smmu_ctrl);

        R_SMMU_Map(&smmu_ctrl, 0x00, 0x00, 0x60000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0xC0000000, 0xC0000000, 0x40000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, (uint64_t)(uintptr_t)rsc_table, UCIE_D2D_MEM, 0x100000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    }

    volatile uint32_t *RCTBUBYPSEN = (volatile uint32_t *)RCTBUBYPSEN_ADDRESS;
    uint32_t smmu_bypass = ~(1U << 0U) & MASK;
    uint32_t old = *RCTBUBYPSEN;
    uint32_t new = (old & ~MASK) | (smmu_bypass & MASK);
    *RCTBUBYPSEN = new;
    R_SMMU_Enable(SMMU_RT, is_secure);

#endif // BOARD != MDP_AIACC_RFS2 && BOARD != X5H_RFS2

    printf("openamp lib version: %s (", openamp_version());
    printf("Major: %d, ", openamp_version_major());
    printf("Minor: %d, ", openamp_version_minor());
    printf("Patch: %d)\r\n", openamp_version_patch());

    printf("libmetal lib version: %s (", metal_ver());
    printf("Major: %d, ", metal_ver_major());
    printf("Minor: %d, ", metal_ver_minor());
    printf("Patch: %d)\r\n", metal_ver_patch());

    /* Trigger EP start setup rproc */
    *(volatile uint32_t*)address_check = 0x1234;
    vTaskDelay(500);

    /* Initialize platform */
    ret = platform_init(NO_USING_MFIS, &platform);
    if (ret) {
        printf("Failed to initialize platform.\r\n");
        goto err1;
    } else {
        rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_DRIVER, NULL, NULL);
        if (!rpdev) {
            printf("Failed to create rpmsg virtio device.\r\n");
            goto err2;
        }
    }

    /* Initialize RPMSG framework */
    printf("Try to create rpmsg endpoint.\r\n");

    ret = rpmsg_create_ept(&lept, rpdev, RPMSG_SERVICE_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_endpoint_cb,
                   rpmsg_service_unbind);
    if (ret) {
        printf("Failed to create endpoint.\r\n");
        goto err3;
    }

    printf("Successfully created rpmsg endpoint.\r\n");

    printf("RPMsg device TX buffer size: %#x\r\n", rpmsg_get_tx_buffer_size(&lept));
    printf("RPMsg device RX buffer size: %#x\r\n", rpmsg_get_rx_buffer_size(&lept));
    
    TaskHandle_t xHandle = NULL;
    xTaskCreate(prvGetNotifyTask, "GetNotifyTask", configMINIMAL_STACK_SIZE*10,
                              platform, (configMAX_PRIORITIES - 1), &xHandle);

    for (int i = 0; i < 100; i++) {
        rpmsg_send(&lept, hello_msg, strlen(hello_msg));
        vTaskDelay(100);
    }
    
    vTaskDelay(500);
    vTaskDelete(xHandle);

    rpmsg_destroy_ept(&lept);
err3:
    platform_release_rpmsg_vdev(rpdev, platform);
err2:
    platform_cleanup(platform);
err1:
    printf("Stopping application...\r\n");
    while(1)
    {
        printf("Task loop\n");
        vTaskDelay(1000);
    }
}



/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int main(void)
{

    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate( echoTask, "echoTask", configMINIMAL_STACK_SIZE*100, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
    /* Don't expect to reach here. */

    return 0;
}

/*-----------------------------------------------------------*/

int printf_raw(const char *format, ...);

void vMainAssertCalled( const char *pcFileName, uint32_t ulLineNumber )
{
    /* Don't use printf as it uses FreeRTOS resources */
    printf_raw("ASSERT!  Line %d of file %s\n", ulLineNumber, pcFileName);
    taskENTER_CRITICAL();
    for( ;; );
}

void vDeleteCallingTask( void )
{
     vTaskDelete( NULL );
}
