/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://github.com/FreeRTOS
 *
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "interrupts.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "smmu/smmu.h"
#include "pfc/r_pfc_api.h"

#include "device_tree.h"
#include "board.h"
#include "rcar_utils.h"

#define main_SMMU_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define SMMU_COREID_MAX 2
#define NUMBER_OF_STREAMID 2

#if (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
#define RCTBUBYPSEN_ADDRESS     0x18B41010  /* Realtime Core TBU bypass enable register address */
#define MASK 0x00000003
#else
#define RCTBUBYPSEN_ADDRESS     0x18B47800  /* Realtime Core TBU bypass enable register address */
#define MASK 0x00000FFF
#endif

/*-----------------------------------------------------------*/
/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvSMMU_RT_Task( void *pvParameters );

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();
    
    
    xTaskCreate( prvSMMU_RT_Task, "SMMU_RT_Task", configMINIMAL_STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 1, NULL );
    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
	/* Don't expect to reach here. */
	return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Ensure no interrupts execute while the scheduler is in an inconsistent
	state.  Interrupts are automatically enabled when the scheduler is
	started. */
	portDISABLE_INTERRUPTS();

	Irq_Setup();
	(void)pfcInitModules(getModuleConfigs());
}

static void prvSMMU_RT_Task( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    #if (BOARD == MDP_AIACC_HIL || BOARD == MDP_X5H_HIL)
    vTaskDelay(1500);
    #endif
    ( void ) pvParameters;
    int ret;

    bool is_secure = true;

    uint32_t coreid = R_UTILS_GetCpuID();
    
    if(coreid >= SMMU_COREID_MAX)
    {
        printf("Error: invalid core_id=%u (max supported=%u)\n", coreid, SMMU_COREID_MAX - 1);
        return;
    }

    #if (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
    uint32_t streamId[SMMU_COREID_MAX][NUMBER_OF_STREAMID] = {
        { 0x00800, 0x00900},    // core0
        { 0x00B00, 0x00A00}     // core1
    };

    #else
    uint32_t streamId[SMMU_COREID_MAX][NUMBER_OF_STREAMID] = {
        { 0x00000, 0x00C00},    // core0
        { 0x10C01, 0x10100}     // core1
    };
    #endif

    st_smmu_streamid_instance_ctrl_t smmu_ctrl = {
        .smmu_domain = SMMU_RT,
	    .is_secure = is_secure,
    };

    printf("**********************************************\r\n");

    printf("* SMMU-RT Cortex-R52 coreid: %d *\r\n", coreid);

    R_SMMU_Init(SMMU_RT, is_secure);
    R_SMMU_InvalidateTLB(SMMU_RT, is_secure);

    for (uint8_t i = 0; i < sizeof(streamId[coreid])/sizeof(uint32_t); i ++) {
        smmu_ctrl.stream_id = streamId[coreid][i];

        ret = R_SMMU_Attach(&smmu_ctrl);
        if (ret == 0) {
            printf("Attach stream id 0x%x result: Passed\r\n", streamId[coreid][i]);
        } else {
            printf("Attach stream id 0x%x result: Failed\r\n", streamId[coreid][i]);
        }

        R_SMMU_Map(&smmu_ctrl, 0x00, 0x00, 0x60000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0xC0000000, 0xC0000000, 0x40000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0x80000000, 0x1840000000, 0x1000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0x70000000, 0x90000000, 0x1000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0x90000000, 0x90000000, 0x1000000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
        R_SMMU_Map(&smmu_ctrl, 0xA0000000, 0xA0000000, 0x1000000, ATTR_DEVICE_NGNRNE_EL1_RO_EL0_RO);
    }

    printf("**********************************************\r\n");

    printf("* Test case 5: Disable SMMU bypass mode and Enable SMMU *\r\n");

    volatile uint32_t *RCTBUBYPSEN = (volatile uint32_t *)RCTBUBYPSEN_ADDRESS;
    
    uint32_t smmu_bypass = ~(1U<<coreid) & MASK ;
    uint32_t old = *RCTBUBYPSEN;
    uint32_t new = (old & ~MASK) | (smmu_bypass & MASK);
    *RCTBUBYPSEN = new;
    /* Wait for TBU bypass-disable to settle before next access */
    for (int i = 0; i < 10000; i++)
    {
        __asm__ volatile("nop");
    }

    if (R_SMMU_Enable(SMMU_RT, is_secure) != 0)
    {
        printf("TC5 result: Failed\n");
    }
    else
    {
        printf("TC5 result: Passed\n");  
    }
    printf("**********************************************\r\n");

    printf("* Test case 6: Verify data *\r\n");

    *(uint32_t *)0x90000000 = 0x7012;
    *(uint32_t *)0x70000000 = 0x123;

    vTaskDelay(10);
    printf("Value at VA 0x70000000 - PA 0x90000000:   0x%x\n", *(uint32_t *)0x70000000);
    printf("Value at VA 0x90000000 - PA 0x90000000:   0x%x\n", *(uint32_t *)0x90000000);

    if (*(uint32_t *)0x70000000 == *(uint32_t *)0x90000000) {
        printf("Result: Passed\r\n");
    }
    else {
        printf("Result: Failed\r\n");
    }

    printf("* Test case 7: SMMU-UnMap *\r\n");

    for (uint8_t i = 0; i < sizeof(streamId[coreid])/sizeof(uint32_t); i ++) {
        smmu_ctrl.stream_id = streamId[coreid][i];

        R_SMMU_Unmap(&smmu_ctrl, 0x70000000, 0x90000000, 0x1000000);
    }
    printf("Unmap 0x70000000 and remap to a new PA. Expect it no longer maps to 0x90000000\n");
    for (uint8_t i = 0; i < sizeof(streamId[coreid])/sizeof(uint32_t); i ++) {
        smmu_ctrl.stream_id = streamId[coreid][i];
        R_SMMU_Map(&smmu_ctrl, 0x70000000, 0x8E200000, 0x00100000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    }
    *(uint32_t *)0x90000000 = 0x1111;
    *(uint32_t *)0x70000000 = 0x2222;
    vTaskDelay(10);
    printf("Value at VA 0x70000000 - new PA 0x8E200000: 0x%x\n", *(uint32_t *)0x70000000);
    printf("Value at VA 0x90000000 - PA 0x90000000:     0x%x\n", *(uint32_t *)0x90000000);

    if (*(uint32_t *)0x70000000 != *(uint32_t *)0x90000000) {
        printf("Result: Passed\r\n");
    }
    else {
        printf("Result: Failed\r\n");
    }

    printf("**********************************************\r\n");

    printf("* Test case 8: Test Read only permission *\r\n");
    printf("TC8: Pass if no further logs after <APP_END>\r\n<APP_END>\n");
    *(uint32_t *)0xA0000000 = 0xBEFFBEFF;
    printf("TC8 result: FAIL\n");
    printf("**********************************************\r\n");

    for(;;);
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

