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
#include "semphr.h"

#include "interrupts.h"
#include "wcrc/r_wcrc_common.h"
#include "wcrc/r_wcrc.h"
#include "dmac/dmac_common.h"
#include "dmac/rtdmac_ctrl.h"
#include <stdio.h>
#include "rcar_utils.h"

#define main_CRC_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

#include "pfc/r_pfc_api.h"
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvCRCTask( void *pvParameters );

void crcUserCallback(void *data);
void kcrcUserCallback(void *data);
/*-----------------------------------------------------------*/

/*
 * Declare some structs used for CRC API.
 */
static uint32_t crc_input[4]    = {0x12345678, 0x12345678, 0x12345678, 0x12345678};
static uint32_t kcrc_input[4]   = {0x12345678, 0x12345678, 0x12345678, 0x12345678};
static uint32_t crc_output = 0xefbda81a;
static uint32_t kcrc_output = 0xefbda81a;
static uint32_t crc_input2[16];
static uint32_t kcrc_input2[16];
static uint32_t crc_output2 = 0xaf6d87d2;
static uint32_t kcrc_output2 = 0xaf6d87d2;
static uint32_t crc_output3[4] = {0x24884ce6, 0xf897a2b3, 0xf9dd2419, 0x25c2ca4c};
static uint32_t kcrc_output3[4] = {0xc9d32fbd, 0x7e657dd7, 0x2f212a60, 0x9897780a};

static uint32_t rtdma_inst[4]  = {RTDMA2_CH4, RTDMA2_CH2, RTDMA0_CH4, RTDMA1_CH6};
static uint32_t rtdma_inst2[4] = {RTDMA0_CH0, RTDMA1_CH4, RTDMA1_CH8, RTDMA2_CH6};

/**** Config CRC Independent mode ****/
wcrc_cfg_t  g_wcrc_cfg0 =
{
    .unit       = WCRC_00,
    .mode       = INDEPENDENT_CRC_MODE,
    .sub_module = CRC_KCRC_SUB_MODULE,

    .crc_cfg    =
    {
        .input_cfg      =
        {
            .p_input_buffer = crc_input,
            .num_data       = sizeof(crc_input)/sizeof(crc_input[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .is_out_exor    = false,
        .is_out_bitswap = false,
        .out_byteswap   = BYTE_SWAP_00,
        .is_in_exor     = false,
        .is_in_bitswap  = false,
        .in_byteswap    = BYTE_SWAP_00
    },

    .kcrc_cfg   =
    {
        .input_cfg      =
        {
            .p_input_buffer = kcrc_input,
            .num_data       = sizeof(kcrc_input)/sizeof(kcrc_input[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .poly_size      = POLY_SIZE_32_BIT,
        .is_out_reflect = true,
        .is_in_reflect  = true,
        .shift_mode     = MSB_SHIFT, 
        .xor_mask_out   = 0xFFFFFFFF
    }
};

wcrc_instance_ctrl_t g_wcrc_inst_ctrl_indepe;

/**** Config E2E CRC mode ****/
wcrc_cfg_t  g_wcrc_cfg1 =
{
    .unit       		= WCRC_09,
    .mode       		= E2E_CRC_MODE,
    .conv_size[CRC_SUB_MODULE]  = 4,
    .conv_size[KCRC_SUB_MODULE] = 4,
    .sub_module 		= CRC_KCRC_SUB_MODULE,

    .crc_cfg    =
    {
        .input_cfg      =
        {
            .p_input_buffer = crc_input,
            .num_data       = sizeof(crc_input)/sizeof(crc_input[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .is_out_exor    = false,
        .is_out_bitswap = false,
        .out_byteswap   = BYTE_SWAP_00,
        .is_in_exor     = false,
        .is_in_bitswap  = false,
        .in_byteswap    = BYTE_SWAP_00,
        .p_rtdma_inst   = &rtdma_inst[0],
        .num_rtdma_inst = 2
    },

    .kcrc_cfg   =
    {
        .input_cfg      =
        {
            .p_input_buffer = kcrc_input,
            .num_data       = sizeof(kcrc_input)/sizeof(kcrc_input[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .poly_size      = POLY_SIZE_32_BIT,
        .is_out_reflect = true,
        .is_in_reflect  = true,
        .shift_mode     = MSB_SHIFT, 
        .xor_mask_out   = 0xFFFFFFFF,
        .p_rtdma_inst   = &rtdma_inst[2],
        .num_rtdma_inst = 2
    }
};

wcrc_instance_ctrl_t g_wcrc_inst_ctrl_e2e;

/**** Config E2E CRC mode ****/
wcrc_cfg_t  g_wcrc_cfg3 =
{
    .unit       		= WCRC_04,
    .mode       		= E2E_CRC_MODE,
    .conv_size[CRC_SUB_MODULE]  = 16,
    .conv_size[KCRC_SUB_MODULE] = 16,
    .sub_module 		= CRC_KCRC_SUB_MODULE,

    .crc_cfg    =
    {
        .input_cfg      =
        {
            .p_input_buffer = crc_input2,
            .num_data       = sizeof(crc_input2)/sizeof(crc_input2[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .is_out_exor    = false,
        .is_out_bitswap = false,
        .out_byteswap   = BYTE_SWAP_00,
        .is_in_exor     = false,
        .is_in_bitswap  = false,
        .in_byteswap    = BYTE_SWAP_00,
        .p_rtdma_inst   = &rtdma_inst2[0],
        .num_rtdma_inst = 2
    },

    .kcrc_cfg   =
    {
        .input_cfg      =
        {
            .p_input_buffer = kcrc_input2,
            .num_data       = sizeof(kcrc_input2)/sizeof(kcrc_input2[0]),
            .crc_seed       = 0xFFFFFFFF,
            .bit_width      = WIDTH_32_BIT
        },

        .poly           = POLY_32_ETHERNET,
        .poly_size      = POLY_SIZE_32_BIT,
        .is_out_reflect = false,
        .is_in_reflect  = false,
        .shift_mode     = MSB_SHIFT, 
        .xor_mask_out   = 0xFFFFFFFF,
        .p_rtdma_inst   = &rtdma_inst2[2],
        .num_rtdma_inst = 2
    }
};

wcrc_instance_ctrl_t g_wcrc_inst_ctrl_e2e_3;
/*-----------------------------------------------------------*/
static void init_data_input()
{
    int i = 0;
    for(i = 0; i < sizeof(crc_input2)/sizeof(crc_input2[0]); i++)
    {
        crc_input2[i]   = 0x12345678 + i;
        kcrc_input2[i]  = 0x12345678 + i;
    }
}

int main( void )
{
    /* Configure the hardware ready to run the demo. */
    prvSetupHardware();

    xTaskCreate( prvCRCTask, "CRC", configMINIMAL_STACK_SIZE * 2, NULL, main_CRC_TASK_PRIORITY, NULL );
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

static void prvCRCTask( void *pvParameters )
{
    uint8_t ret;
    uint32_t num_data;
    bool * is_done[2];
    st_memory_t region;
    uint32_t crc_buf, kcrc_buf, crc_size;
    wcrc_instance_ctrl_t * p_instance_ctrl;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    ret = R_RTDMAC_RcarDmacCtrlInit(RT_DMAC0,
                                    DRV_RTDMAC_PRIO_FIX);
    //ret = R_RTDMAC_RcarDmacCtrlInit(RT_DMAC1,
    //                                DRV_RTDMAC_PRIO_FIX);
    ret = R_RTDMAC_RcarDmacCtrlInit(RT_DMAC2,
                                    DRV_RTDMAC_PRIO_FIX);
    ret = R_RTDMAC_RcarDmacCtrlInit(RT_DMAC3,
                                    DRV_RTDMAC_PRIO_FIX);

    init_data_input();

    printf("\n********** TC1: CRC Independent Mode **********\n");
    ret = R_CRC_Open(&g_wcrc_inst_ctrl_indepe, &g_wcrc_cfg0);
    printf("R_CRC_Open: ret = %d\n", ret);

    /* CRC buffer address */
    region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    crc_buf = region.base_address;

    /* KCRC buffer address */
    ret = R_CRC_Get_BufferSize(CRC_SUB_MODULE, &g_wcrc_inst_ctrl_indepe, &crc_size);
    kcrc_buf = crc_buf + crc_size;

    /* Set KCRC/CRC buffer address */
    ret = R_CRC_Set_BufferAddress(&g_wcrc_inst_ctrl_indepe, crc_buf, kcrc_buf);

    ret = R_CRC_Calculate(&g_wcrc_inst_ctrl_indepe);
    printf("\nR_CRC_Calculate: ret = %d\n\n", ret);

    /* Wait WCRC operation in 3ms */
    ret = R_CRC_Wait_Operation(&g_wcrc_inst_ctrl_indepe, 3);
    printf("\nR_CRC_Wait_Operation: ret = %d\n\n", ret);

    ret = R_CRC_Get_Input_Data(&g_wcrc_inst_ctrl_indepe);
    printf("\nR_CRC_Get_Input_Data: ret = %d\n\n", ret);

    ret = R_CRC_Get_Generated_Value(&g_wcrc_inst_ctrl_indepe);
    printf("\nR_CRC_Get_Generated_Value: ret = %d\n", ret);

    uint32_t *p_data_crc;
    p_data_crc = (uint32_t *)g_wcrc_inst_ctrl_indepe.crc_data[CRC_SUB_MODULE].p_output_buffer;

    uint32_t *p_data_kcrc;
    p_data_kcrc = (uint32_t *)g_wcrc_inst_ctrl_indepe.crc_data[KCRC_SUB_MODULE].p_output_buffer;

    if(*p_data_crc == crc_output && *p_data_kcrc == kcrc_output)
    {
        printf("TC1 Result: Passed");
    }
    else
    {
        printf("TC1 Result: Failed");
    }

    printf("\n********** TC2: E2E CRC Mode **********\n");
    ret = R_CRC_Open(&g_wcrc_inst_ctrl_e2e, &g_wcrc_cfg1);
    printf("R_CRC_Open: ret = %d\n", ret);

    /* CRC buffer address */
    region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    crc_buf = region.base_address + 0x200;

    /* KCRC buffer address */
    ret = R_CRC_Get_BufferSize(CRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e, &crc_size);
    kcrc_buf = crc_buf + crc_size;

    /* Set KCRC/CRC buffer address */
    ret = R_CRC_Set_BufferAddress(&g_wcrc_inst_ctrl_e2e, crc_buf, kcrc_buf);

    is_done[CRC_SUB_MODULE] = &g_wcrc_inst_ctrl_e2e.crc_data[CRC_SUB_MODULE].is_done;
    ret  = R_CRC_Set_Callback(CRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e,
                             crcUserCallback, is_done[CRC_SUB_MODULE]);

    is_done[KCRC_SUB_MODULE] = &g_wcrc_inst_ctrl_e2e.crc_data[KCRC_SUB_MODULE].is_done;
    ret |= R_CRC_Set_Callback(KCRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e,
                             kcrcUserCallback, is_done[KCRC_SUB_MODULE]);
    printf("R_CRC_Set_Callback: ret = %d\n", ret);

    ret = R_CRC_Calculate(&g_wcrc_inst_ctrl_e2e);
    printf("\nR_CRC_Calculate: ret = %d\n\n", ret);

    /* Wait WCRC operation in 3ms */
    ret = R_CRC_Wait_Operation(&g_wcrc_inst_ctrl_e2e, 3);
    printf("\nR_CRC_Wait_Operation: ret = %d\n\n", ret);

    ret = R_CRC_Get_Input_Data(&g_wcrc_inst_ctrl_e2e);
    printf("\nR_CRC_Get_Input_Data: ret = %d\n\n", ret);

    ret = R_CRC_Get_Generated_Value(&g_wcrc_inst_ctrl_e2e);
    printf("\nR_CRC_Get_Generated_Value: ret = %d\n", ret);

    p_data_crc = (uint32_t *)g_wcrc_inst_ctrl_e2e.crc_data[CRC_SUB_MODULE].p_output_buffer;
    p_data_kcrc = (uint32_t *)g_wcrc_inst_ctrl_e2e.crc_data[KCRC_SUB_MODULE].p_output_buffer;

    if(*p_data_crc == crc_output2 && *p_data_kcrc == kcrc_output2)
    {
        printf("TC2 Result: Passed");
    }
    else
    {
        printf("TC2 Result: Failed");
    }

    printf("\n********** TC3: E2E CRC Mode **********\n");
    ret = R_CRC_Open(&g_wcrc_inst_ctrl_e2e_3, &g_wcrc_cfg3);
    printf("R_CRC_Open: ret = %d\n", ret);

    region = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
    crc_buf = region.base_address + 0x1000;

    /* KCRC buffer address */
    ret = R_CRC_Get_BufferSize(CRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e_3, &crc_size);
    kcrc_buf = crc_buf + crc_size;

    /* Set KCRC/CRC buffer address */
    ret = R_CRC_Set_BufferAddress(&g_wcrc_inst_ctrl_e2e_3, crc_buf, kcrc_buf);

    is_done[CRC_SUB_MODULE] = &g_wcrc_inst_ctrl_e2e_3.crc_data[CRC_SUB_MODULE].is_done;
    ret  = R_CRC_Set_Callback(CRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e_3,
                             crcUserCallback, is_done[CRC_SUB_MODULE]);

    is_done[KCRC_SUB_MODULE] = &g_wcrc_inst_ctrl_e2e_3.crc_data[KCRC_SUB_MODULE].is_done;
    ret |= R_CRC_Set_Callback(KCRC_SUB_MODULE, &g_wcrc_inst_ctrl_e2e_3,
                             kcrcUserCallback, is_done[KCRC_SUB_MODULE]);
    printf("R_CRC_Set_Callback: ret = %d\n", ret);

    ret = R_CRC_Calculate(&g_wcrc_inst_ctrl_e2e_3);
    printf("\nR_CRC_Calculate: ret = %d\n\n", ret);

    /* Wait WCRC operation in 3ms */
    ret = R_CRC_Wait_Operation(&g_wcrc_inst_ctrl_e2e_3, 3);
    printf("\nR_CRC_Wait_Operation: ret = %d\n\n", ret);

    ret = R_CRC_Get_Input_Data(&g_wcrc_inst_ctrl_e2e_3);
    printf("\nR_CRC_Get_Input_Data: ret = %d\n\n", ret);

    ret = R_CRC_Get_Generated_Value(&g_wcrc_inst_ctrl_e2e_3);
    printf("\nR_CRC_Get_Generated_Value: ret = %d\n", ret);

    ret = R_CRC_Close(&g_wcrc_inst_ctrl_indepe);
    printf("\nR_CRC_Close: ret = %d\n", ret);

    ret = R_CRC_Close(&g_wcrc_inst_ctrl_e2e);
    printf("\nR_CRC_Close: ret = %d\n", ret);

    ret = R_CRC_Close(&g_wcrc_inst_ctrl_e2e_3);
    printf("\nR_CRC_Close: ret = %d\n", ret);

    p_data_crc = (uint32_t *)g_wcrc_inst_ctrl_e2e_3.crc_data[CRC_SUB_MODULE].p_output_buffer;
    p_data_kcrc = (uint32_t *)g_wcrc_inst_ctrl_e2e_3.crc_data[KCRC_SUB_MODULE].p_output_buffer;
    bool pass = true;

    for (int i = 0; i < g_wcrc_inst_ctrl_e2e_3.crc_data[CRC_SUB_MODULE].num_data; i++)
    {
        if (p_data_crc[i] != crc_output3[i])
        {
            pass = false;
            break;
        }
    }

    for (int i = 0; i < g_wcrc_inst_ctrl_e2e_3.crc_data[KCRC_SUB_MODULE].num_data; i++)
    {
        if (p_data_kcrc[i] != kcrc_output3[i])
        {
            pass = false;
            break;
        }
    }

    printf("TC3 Result: %s\n", pass ? "Passed" : "Failed");
    printf("<APP_END>\n");

    for( ;; )
    {
    }
}

/*-----------------------------------------------------------*/

void crcUserCallback(void *data) {
    bool * is_done = (bool *)data;
    * is_done = true;
    printf("CRC: cb\n");
}

void kcrcUserCallback(void *data) {
    bool * is_done = (bool *)data;
    * is_done = true;
    printf("KCRC: cb\n");
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
