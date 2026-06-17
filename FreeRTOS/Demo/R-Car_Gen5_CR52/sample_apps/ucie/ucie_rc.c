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

#include <stdlib.h>
#include "stdio.h"
#include "string.h"

#include "pfc/r_pfc_api.h"

#include "device_tree.h"

#include "smmu/smmu.h"
#include "ucie/r_ucie.h"
#include "ucie_concept.h"
#include "rcar_utils.h"

#define main_ucie_TASK_PRIORITY        (tskIDLE_PRIORITY + 1)
#define UCIE_RC_SIZE (configMINIMAL_STACK_SIZE * 2)

/* Memory size macros */
#define SIZE_64MB           (0x4000000)
#define SIZE_32MB           (0x2000000)
#define SIZE_16MB           (0x1000000)
#define SIZE_8MB            (0x800000)
#define SIZE_1MB            (0x100000)
#define SIZE_256KB          (0x40000)
#define SIZE_1KB            (0x400)
#define DMA_SIZE_PER_CHAN	(SIZE_64MB)

#define DRAM_DBSC01_ADDR_PA     (0xB0000000)

#define DBSC01_HDMA_PA(n)       (DRAM_DBSC01_ADDR_PA + (n) * DMA_SIZE_PER_CHAN)

#define MASK 0x00000FFF

// For RC write EP read test data
st_ucie_hdma_cfg_t hdma_tbl_wrtest_dt[] = {	// UCIE1 WRCHx1
	/*	ucie_ch	    hdma_ch	    mSrcAddr			mDestAddr			size				rw */
	{	UCIE_CH1,   HDMA_CH0,	DBSC01_HDMA_PA(0), 	DBSC01_HDMA_PA(1),  DMA_SIZE_PER_CHAN,	0,  },
	{	0,		    0,		    0x0,				0x0,				0x0,		        0,  },	// End Of Table
};

const uint32_t TEST_DATA0[8U] = {0x12345678, 0xFEDCBA98, 0xA5A5A5A5, 0x5A5A5A5A,
						   0x11223344, 0x55667788, 0xAABBCCDD, 0xEEEEFFFF};

const uint32_t TEST_DATA1[8U] = {0x11111111, 0x22222222, 0x66666666, 0x88888888,
                                 0xBBBBBBBB, 0xCCCCCCCC, 0xEEEEEEEE, 0x55555555};

#define PIO_RC_WRITE_DATA   0xFFCCFFCC
#define PIO_EP_WRITE_DATA   0x68686868

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void ucie_comm_task( void *pvParameters );
extern int console_getc(unsigned char *p_char);
extern uint32_t Ucie_Setup_rc(uint8_t ch);
/*-----------------------------------------------------------*/

void create_test_data(uint64_t src, const uint32_t *pattern, uint32_t size) {
    for (uint32_t i = 0; i < size/32; i++) {
        *((volatile uint32_t *)(uintptr_t)src + 0) = pattern[0];
        *((volatile uint32_t *)(uintptr_t)src + 1) = pattern[1];
        *((volatile uint32_t *)(uintptr_t)src + 2) = pattern[2];
        *((volatile uint32_t *)(uintptr_t)src + 3) = pattern[3];
        *((volatile uint32_t *)(uintptr_t)src + 4) = pattern[4];
        *((volatile uint32_t *)(uintptr_t)src + 5) = pattern[5];
        *((volatile uint32_t *)(uintptr_t)src + 6) = pattern[6];
        *((volatile uint32_t *)(uintptr_t)src + 7) = pattern[7];

        src = src + 32;
    }
}

void print_test_data(uint64_t src, uint32_t size) {
    printf("Start data at addr 0x%llX\n\t", src);
    for (uint8_t i = 0; i < 8; i++) {
        printf("0x%X ", *((volatile uint32_t *)(uintptr_t)src + i));
    }
    printf("\n");

    if (size > 32) {
        printf("End data at addr 0x%llX\n\t", src + size - 32);
        for (uint8_t i = 0; i < 8; i++) {
            printf("0x%X ", *((volatile uint32_t *)(uintptr_t)(src + size - 32) + i));
        }
        printf("\n");
    }
}

uint32_t verify_test_data(uint64_t src, const uint32_t *pattern, uint32_t size) {
    for(uint32_t i = 0; i < size/4; i++) {
        if (*((volatile uint32_t*)(uintptr_t)src + i) != pattern[i%8]) {
            return 1;
        }
    }
    return 0;
}

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate(ucie_comm_task, "UCIe", UCIE_RC_SIZE, NULL, main_ucie_TASK_PRIORITY, NULL );

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

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

static void ucie_comm_task(void *pvParameters)
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    uint32_t ret = 0;
    uint32_t val;
    uint8_t index;
    *(st_ucie_trigger_sig_t*)UCIE_LOOPCHECK_ADDR = (st_ucie_trigger_sig_t){0};

    /* UCIe_chan = 1 - Root-Complex */
    printf("<----- [RC] TC1: UCIE LINKUP ----->\n");
    ret = R_UCIE_Setup(UCIE_CH1, UCIE_MODE_RC, LINKSPEED_16GTPS);
    if (ret == LINKUP_TIMEOUT) {
        printf("The first time linkup timeout. Retry 30 times\n");
        ret = R_UCIE_Retry_Linkup(UCIE_CH1, UCIE_MODE_RC, LINKSPEED_16GTPS, 30);
    }
    
    if (ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("<----- [RC] TC 2: TRANSFER DATA USING HDMA ----->\n");
    printf("Generate test data\n");
    create_test_data(hdma_tbl_wrtest_dt->mSrcAddr, TEST_DATA0, hdma_tbl_wrtest_dt->size); 
    print_test_data(hdma_tbl_wrtest_dt->mSrcAddr, hdma_tbl_wrtest_dt->size);

    printf("Start RC HDMA transfer\n");
    
    index = 0;
    /* Start All CH Transfer */
    while (hdma_tbl_wrtest_dt[index].size != 0) {
        R_UCIE_HDMA_Start(hdma_tbl_wrtest_dt + index);
        index++;
    }

    index = 0;
    /* Wait All CH STOP */
    while (hdma_tbl_wrtest_dt[index].size != 0) {
        if (R_UCIE_HDMA_WaitStop(hdma_tbl_wrtest_dt + index)) {
            ret = 1;
        }

        index++;
    }

    printf("End RC HDMA transfer\n");
    if (ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");

        /* Send trigger signal to EP */
        *((st_ucie_trigger_sig_t*)(uintptr_t)hdma_tbl_wrtest_dt[0].mSrcAddr) = (st_ucie_trigger_sig_t) {
            .addr = hdma_tbl_wrtest_dt[0].mDestAddr,
            .size = hdma_tbl_wrtest_dt[0].size,
            .flag = 1,
        };
        
        hdma_tbl_wrtest_dt[0].mDestAddr = UCIE_LOOPCHECK_ADDR;
        hdma_tbl_wrtest_dt[0].size = sizeof(st_ucie_trigger_sig_t);
        
        R_UCIE_HDMA_Start(hdma_tbl_wrtest_dt + 0);
    }

    printf("<----- [RC] TC 3: VERIFY TRANSFER DATA ----->\n");
    st_ucie_trigger_sig_t *signal = (st_ucie_trigger_sig_t *)UCIE_LOOPCHECK_ADDR;
    uint32_t timer_freq = R_UTILS_GetTimerFrequency();
    uint64_t start = R_UTILS_GetTimerCounter();
    ret = 1;

    while ((R_UTILS_GetTimerCounter() - start)/timer_freq < 3) {
        if (signal->flag == 1) {
            ret = 0;
            break;
        }
    }

    if (!ret) {
        printf("Data after transfer:\n");
        print_test_data(signal->addr, signal->size);
        if (verify_test_data(signal->addr, TEST_DATA1, signal->size)) {
            printf("Result: FAILED\n");
        }
        else {
            printf("Result: PASSED\n");
        }
    }
    else {
        printf("Result: FAILED\n");
    }

    printf("<----- [RC] TC 4: PIO TRANSFER ----->\n");
    uint64_t ucie1_pa = 0x24000000000;
    uint32_t ucie1_va = 0x8E600000;

    bool is_secure = true;

    uint32_t streamId[] = {
        0x000,
        0xC00,
    };

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
        R_SMMU_Map(&smmu_ctrl, ucie1_va, ucie1_pa, 0x10000, ATTR_DEVICE_NGNRNE_EL1_RW_EL0_RW);
    }

    volatile uint32_t *RCTBUBYPSEN = (volatile uint32_t *)0x18B47800;
    uint32_t smmu_bypass = 0xFFE;
    uint32_t old = *RCTBUBYPSEN;
    uint32_t new = (old & ~MASK) | (smmu_bypass & MASK);
    *RCTBUBYPSEN = new;
    R_SMMU_Enable(SMMU_RT, is_secure);

    /* RC Write EP Read */
    *((volatile uint32_t*)ucie1_va) = PIO_RC_WRITE_DATA;

    /* EP Write RC Read */
    ret = 1;
    start = R_UTILS_GetTimerCounter();
    while ((R_UTILS_GetTimerCounter() - start)/timer_freq < 3) {
        if(*((volatile uint32_t*)ucie1_va) == PIO_EP_WRITE_DATA) {
            ret = 0;
            break;
        }
    }
    
    printf("Data after transfer: 0x%X\n", *((volatile uint32_t*)ucie1_va));
    if(ret) {
        printf("Result: FAILED\n");
    }
    else {
        printf("Result: PASSED\n");
    }

    printf("<----- [RC] <APP_END> ----->\n");
    for(;;) {
        __asm__ volatile("nop");
    }
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
