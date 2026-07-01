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
#include "stdio.h"
#include "i2c/r_i2c.h"
#define main_I2C_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#include "pfc/r_pfc_api.h"

#include "device_tree.h"

#include "dmac/dmac_common.h"
#include "rcar_utils.h"

#define TCAL9539	0x77
#define PTN3222 	0x43
#define MAX96726A_1	0x31
#define MAX96726A_2	0x33
#define RC21214		0x09
#define RC21214_REG_ADDR 0xFC
#define TCAL9539_REG_ADDR   0x03
#define PTN3222_REG_ADDR 	0x02

#define I2C_APP_SIZE (configMINIMAL_STACK_SIZE * 2)
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvI2CTask( void *pvParameters );
static void i2cUserCallback(void *data);

static bool transfer_success = false;
static SemaphoreHandle_t xI2C_Semaphore;

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();


	xTaskCreate( prvI2CTask, "prvI2CTask", I2C_APP_SIZE, NULL, main_I2C_TASK_PRIORITY, NULL);
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

#if (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2 || BOARD == X5H_VDK || BOARD == MDP_X5H_HIL)

static void prvI2CTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	/* For PIO mode */
	uint8_t send_data[] = { 0x03, 0x02};
	uint8_t result[1] = {0};
	uint8_t default_value[1] = {0}, restore_data[2];

	/* For DMA mode */
	uint8_t send_data_dma[] = { RC21214_REG_ADDR, 0x00, 0x01, 0x00 }; /* Register address - Data1 - Data2 - Data3 */
	uint8_t result_dma[4] = {0};
	uint8_t default_value_dma[4] = {0}, restore_data_dma[5];

	xI2C_Semaphore = xSemaphoreCreateBinary();
    if (xI2C_Semaphore == NULL) 
	{
        printf("ERROR: Cannot create semaphore for I2C\n");
        for( ;; )
		{
			vTaskDelay(1000);
		};
    }

	/* Device driver part for ch1 */
	i2c_instance_ctrl_t g_i2c_device_ctrl_1;
	i2c_master_cfg_t        g_i2c_device_cfg_1 =
	{
	    .channel       = 1,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = TCAL9539,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_1,
	};
	printf("------------- Start I2C Test -------------\r\n");
	
	printf("------------- TC1: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);
	R_I2C_Open(&g_i2c_device_ctrl_1, &g_i2c_device_cfg_1);

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	R_I2C_Write(&g_i2c_device_ctrl_1, send_data, sizeof(send_data), 0);
	printf("WRITE DONE\r\n");

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, (uint8_t *)&result, sizeof(result));
	printf("READ DATA: ");
	for(uint8_t i = 0;i < sizeof(result); i++)
		printf("0x%x\t", result[i]);
	printf("\r\n");

	if (result[0] == send_data[1]) {
	    printf(">>> TEST OK <<<\r\n");
	} else {
	    printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore original value */
	restore_data[0] = 0x03;
	restore_data[1] = default_value[0];

	R_I2C_Write(&g_i2c_device_ctrl_1, restore_data, sizeof(restore_data), 0);
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");
	R_I2C_Close(&g_i2c_device_ctrl_1);
	printf("------------- TC1: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);



	/* Device driver part for channel 2*/
	i2c_instance_ctrl_t g_i2c_device_ctrl_2;
	i2c_master_cfg_t        g_i2c_device_cfg_2 =
	{
	    .channel       = 2,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = TCAL9539,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_2,
	};

	printf("------------- TC2: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_2.channel);
	R_I2C_Open(&g_i2c_device_ctrl_2, &g_i2c_device_cfg_2);

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_2, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	R_I2C_Write(&g_i2c_device_ctrl_2, send_data, sizeof(send_data), 0);
	printf("WRITE DONE\r\n");

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_2, 0x03, (uint8_t *)&result, sizeof(result));
	printf("READ DATA: ");
	for(uint8_t i = 0;i < sizeof(result); i++)
		printf("0x%x\t", result[i]);
	printf("\r\n");

	if (result[0] == send_data[1]) {
	    printf(">>> TEST OK <<<\r\n");
	} else {
	    printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore original value */
	restore_data[0] = 0x03;
	restore_data[1] = default_value[0];

	R_I2C_Write(&g_i2c_device_ctrl_2, restore_data, sizeof(restore_data), 0);
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_2, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");
	R_I2C_Close(&g_i2c_device_ctrl_2);
	printf("------------- TC2: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_2.channel);

	/* Device driver part for channel 3*/
	i2c_instance_ctrl_t g_i2c_device_ctrl_3;
	i2c_master_cfg_t        g_i2c_device_cfg_3 =
	{
	    .channel       = 3,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = MAX96726A_1,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_3,
	};

	printf("------------- TC3: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_3.channel);
	R_I2C_Open(&g_i2c_device_ctrl_3, &g_i2c_device_cfg_3);

	uint16_t Addr = 0x73;
	int8_t   no_bytes = 2;	// number of bytes to send for the register address (16-bit → 2 bytes)
	uint8_t  send_byte[no_bytes];
	uint8_t write_data[3];

	#if (BOARD == MDP_X5H_HIL)
	printf("TC3 result: NA. Cannot test channel3 on MDP X5H board currently\n");
	R_I2C_Close(&g_i2c_device_ctrl_3);
	printf("------------- TC3: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_3.channel);
	#else
	send_byte[0] = Addr >> 8;
	send_byte[1] = Addr & 0xff;
	R_I2C_Write(&g_i2c_device_ctrl_3, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_3, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	write_data[0] = send_byte[0];   // MSB of address
	write_data[1] = send_byte[1];   // LSB of address
	write_data[2] = 0x2;            // Data to write
	R_I2C_Write(&g_i2c_device_ctrl_3, write_data, sizeof(write_data), 0);
	printf("WRITE DONE\r\n");

	R_I2C_Write(&g_i2c_device_ctrl_3, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_3, result, sizeof(result), false);
	printf("READ DATA AFTER WRITE: 0x%x\t", result[0]);
	printf("\r\n");

	if (result[0] == write_data[2]) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore */
	write_data[2] = default_value[0];
	R_I2C_Write(&g_i2c_device_ctrl_3, write_data, sizeof(write_data), 0);


	R_I2C_Write(&g_i2c_device_ctrl_3, send_byte, no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_3, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: 0x%x\t", default_value[0]);
	printf("\r\n");

	R_I2C_Close(&g_i2c_device_ctrl_3);
	printf("------------- TC3: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_3.channel);
	#endif

	/* Coding for channel 4*/
	i2c_instance_ctrl_t g_i2c_device_ctrl_4;
	i2c_master_cfg_t        g_i2c_device_cfg_4 =
	{
	    .channel       = 4,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = MAX96726A_2,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_4,
	};

	printf("------------- TC4: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_4.channel);
	R_I2C_Open(&g_i2c_device_ctrl_4, &g_i2c_device_cfg_4);

	Addr = 0x31;
	no_bytes = 2;	// number of bytes to send for the register address (16-bit → 2 bytes)
	send_byte[no_bytes];
	write_data[3];
	//uint8_t result[1];

	send_byte[0] = Addr >> 8;
	send_byte[1] = Addr & 0xff;
	R_I2C_Write(&g_i2c_device_ctrl_4, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_4, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	write_data[0] = send_byte[0];   // MSB of address
	write_data[1] = send_byte[1];   // LSB of address
	write_data[2] = 0xa2;            // Data to write
	R_I2C_Write(&g_i2c_device_ctrl_4, write_data, sizeof(write_data), 0);
	printf("WRITE DONE\r\n");

	/* Read back to verify */
	R_I2C_Write(&g_i2c_device_ctrl_4, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_4, result, sizeof(result), false);
	printf("READ DATA AFTER WRITE: 0x%x\t", result[0]);
	printf("\r\n");

	if (result[0] == write_data[2]) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}


	write_data[2] = default_value[0];
	R_I2C_Write(&g_i2c_device_ctrl_4, write_data, sizeof(write_data), 0);

	/* Read again to confirm restore */
	R_I2C_Write(&g_i2c_device_ctrl_4, send_byte, no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_4, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: 0x%x\t", default_value[0]);
	printf("\r\n");

	R_I2C_Close(&g_i2c_device_ctrl_4);
	printf("------------- TC4: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_4.channel);

	/* Device driver part for channel 5 */
	i2c_instance_ctrl_t g_i2c_device_ctrl_5;
	i2c_master_cfg_t        g_i2c_device_cfg_5 =
	{
	    .channel       = 5,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = PTN3222,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_5,
	};
	send_data[0] = 0x02;
	send_data[1] = 0x40;

	printf("------------- TC5: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_5.channel);
	R_I2C_Open(&g_i2c_device_ctrl_5, &g_i2c_device_cfg_5);

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_5, 0x02, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	R_I2C_Write(&g_i2c_device_ctrl_5, send_data, sizeof(send_data), 0);
	printf("WRITE DONE\r\n");

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_5, 0x02, (uint8_t *)&result, sizeof(result));
	printf("READ DATA: ");
	for (uint8_t i = 0; i < sizeof(result); i++)
		printf("0x%x\t", result[i]);
	printf("\r\n");

	if (result[0] == send_data[1]) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}

	restore_data[0] = 0x02;
	restore_data[1] = default_value[0];
	R_I2C_Write(&g_i2c_device_ctrl_5, restore_data, sizeof(restore_data), 0);
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_5, 0x02, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	R_I2C_Close(&g_i2c_device_ctrl_5);
	printf("------------- TC5: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_5.channel);

	#if (BOARD == MDP_X5H_HIL)
	send_data_dma[0] = PTN3222_REG_ADDR;
	/* Device driver part for channel 5 DMA */
    i2c_instance_ctrl_t g_i2c_device_ctrl_6;
	i2c_master_cfg_t g_i2c_device_cfg_6 =
	{
		.channel       = 5,
		.rate          = I2C_MASTER_RATE_FAST,
		.slave         = PTN3222,
		.addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
		.dma_single    = true,
		.p_context     = &g_i2c_device_ctrl_6,
		.dmac_unit      = SYS_DMAC2,
		.dmac_channel   = DMAC_CH4,
		.dmac_irq_id    = INTID_SYSDMA2_CH4
	};

    printf("------------- TC6: START TEST I2C CHANNEL %d DMA MODE (MDP) -------------\r\n", g_i2c_device_cfg_6.channel);
	
	/* Open I2C */
	R_I2C_Open(&g_i2c_device_ctrl_6, &g_i2c_device_cfg_6);

	/* Set interrupt I2C */
	R_I2C_CallbackSet(&g_i2c_device_ctrl_6, (void *)i2cUserCallback, &g_i2c_device_ctrl_6, NULL);

	/* Read default value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("DEFAULT VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	send_data_dma[1] = 0x40;
	send_data_dma[2] = default_value_dma[1];
	send_data_dma[3] = default_value_dma[2];
	/* Write data */
	printf("WRITE DATA\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_6, send_data_dma, sizeof(send_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE DONE\r\n");
	}

	/* Read again to check write OK or not */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], (uint8_t *)&result_dma, sizeof(result_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(result_dma, sizeof(result_dma));
			printf("READ DATA: ");
			for (uint8_t i = 0; i < sizeof(result_dma); i++)
				printf("0x%02x\t", result_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	bool test_passed = true;
	for (uint8_t i = 0; i < 3; i++) { 
		if (result_dma[i] != send_data_dma[i+1]) { // result_dma[0,1,2] compare send_data_dma[1,2,3]
			test_passed = false;
			break;
		}
	}
	if (test_passed) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore default value */
	restore_data_dma[0] = PTN3222_REG_ADDR; // Register address
	restore_data_dma[1] = default_value_dma[0];
	restore_data_dma[2] = default_value_dma[1];
	restore_data_dma[3] = default_value_dma[2];
	restore_data_dma[4] = default_value_dma[3];
	printf("RESTORING ORIGINAL VALUES...\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_6, restore_data_dma, sizeof(restore_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE RESTORE VALUE DONE\n");
	}

	/* Print restore value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("RESTORED VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	/* Close instance */
	R_I2C_Close(&g_i2c_device_ctrl_6);
    printf("------------- TC6: END TEST I2C CHANNEL %d DMA MODE -------------\r\n", g_i2c_device_cfg_6.channel);
	#else
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Device driver part for channel 6*/
    i2c_instance_ctrl_t g_i2c_device_ctrl_6;
	i2c_master_cfg_t g_i2c_device_cfg_6 =
	{
		.channel       = 6,
		.rate          = I2C_MASTER_RATE_FAST,
		.slave         = RC21214,
		.addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
		.dma_single    = true,
		.p_context     = &g_i2c_device_ctrl_6,
		.dmac_unit     = SYS_DMAC2,
		.dmac_channel  = DMAC_CH4,
		.dmac_irq_id   = INTID_SYSDMA2_CH4
	};

    printf("------------- TC6: START TEST I2C CHANNEL %d DMA MODE -------------\r\n", g_i2c_device_cfg_6.channel);
	
	/* Open I2C */
	R_I2C_Open(&g_i2c_device_ctrl_6, &g_i2c_device_cfg_6);

	/* Set interrupt I2C */
	R_I2C_CallbackSet(&g_i2c_device_ctrl_6, (void *)i2cUserCallback, &g_i2c_device_ctrl_6, NULL);

	/* Read default value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("DEFAULT VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	/* Write data */
	printf("WRITE DATA\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_6, send_data_dma, sizeof(send_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE DONE\r\n");
	}

	/* Read again to check write OK or not */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], (uint8_t *)&result_dma, sizeof(result_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(result_dma, sizeof(result_dma));
			printf("READ DATA: ");
			for (uint8_t i = 0; i < sizeof(result_dma); i++)
				printf("0x%02x\t", result_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	bool test_passed = true;
	for (uint8_t i = 0; i < 3; i++) { 
		if (result_dma[i] != send_data_dma[i+1]) { // result_dma[0,1,2] compare send_data_dma[1,2,3]
			test_passed = false;
			break;
		}
	}
	if (test_passed) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore default value */
	restore_data_dma[0] = RC21214_REG_ADDR; // Register address
	restore_data_dma[1] = default_value_dma[0];
	restore_data_dma[2] = default_value_dma[1];
	restore_data_dma[3] = default_value_dma[2];
	restore_data_dma[4] = default_value_dma[3];
	printf("RESTORING ORIGINAL VALUES...\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_6, restore_data_dma, sizeof(restore_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE RESTORE VALUE DONE\n");
	}

	/* Print restore value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_6, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("RESTORED VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	/* Close instance */
	R_I2C_Close(&g_i2c_device_ctrl_6);
    printf("------------- TC6: END TEST I2C CHANNEL %d DMA MODE -------------\r\n", g_i2c_device_cfg_6.channel);
	#endif
	/* Delete semaphore */
	vSemaphoreDelete(xI2C_Semaphore);
	printf("------------- End -------------\r\n");
	for( ;; )
	{
	};
}

#else   // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
static void prvI2CTask( void *pvParameters )
{
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	/* For PIO mode */
	uint8_t send_data[] = { 0x03, 0x02};
	uint8_t result[1] = {0};
	uint8_t default_value[1] = {0}, restore_data[2];

	/* For DMA mode */
	uint8_t send_data_dma[] = { TCAL9539_REG_ADDR, 0x00, 0x01, 0x00 }; /* Register address - Data1 - Data2 - Data3 */
	uint8_t result_dma[4] = {0};
	uint8_t default_value_dma[4] = {0}, restore_data_dma[5];

	xI2C_Semaphore = xSemaphoreCreateBinary();
    if (xI2C_Semaphore == NULL) 
	{
        printf("ERROR: Cannot create semaphore for I2C\n");
        for( ;; )
		{
			vTaskDelay(1000);
		};
    }

	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;

	/* Device driver part for ch1 */
	i2c_instance_ctrl_t g_i2c_device_ctrl_1;
	i2c_master_cfg_t        g_i2c_device_cfg_1 =
	{
	    .channel       = 1,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = TCAL9539,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_1,
	};
	printf("------------- Start I2C Test -------------\r\n");
	
	printf("------------- TC1: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);
	R_I2C_Open(&g_i2c_device_ctrl_1, &g_i2c_device_cfg_1);

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	R_I2C_Write(&g_i2c_device_ctrl_1, send_data, sizeof(send_data), 0);
	printf("WRITE DONE\r\n");

	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, (uint8_t *)&result, sizeof(result));
	printf("READ DATA: ");
	for(uint8_t i = 0;i < sizeof(result); i++)
		printf("0x%x\t", result[i]);
	printf("\r\n");

	if (result[0] == send_data[1]) {
	    printf(">>> TEST OK <<<\r\n");
	} else {
	    printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore original value */
	restore_data[0] = 0x03;
	restore_data[1] = default_value[0];

	R_I2C_Write(&g_i2c_device_ctrl_1, restore_data, sizeof(restore_data), 0);
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, 0x03, default_value, sizeof(default_value));
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");
	R_I2C_Close(&g_i2c_device_ctrl_1);
	printf("------------- TC1: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);



	/* Coding for channel 0*/
	i2c_instance_ctrl_t g_i2c_device_ctrl_0;
	i2c_master_cfg_t        g_i2c_device_cfg_0 =
	{
	    .channel       = 0,
	    .rate          = I2C_MASTER_RATE_FAST,
	    .slave         = MAX96726A_2,
	    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
	    .p_callback    = NULL,     // Callback
	    .p_context     = &g_i2c_device_ctrl_0,
	};

	printf("------------- TC2: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_0.channel);
	R_I2C_Open(&g_i2c_device_ctrl_0, &g_i2c_device_cfg_0);

	uint16_t Addr = 0x73;
	int8_t   no_bytes = 2;	// number of bytes to send for the register address (16-bit → 2 bytes)
	uint8_t  send_byte[no_bytes];
	uint8_t write_data[3];

	Addr = 0x31;
	no_bytes = 2;	// number of bytes to send for the register address (16-bit → 2 bytes)
	send_byte[no_bytes];
	write_data[3];
	//uint8_t result[1];

	send_byte[0] = Addr >> 8;
	send_byte[1] = Addr & 0xff;
	R_I2C_Write(&g_i2c_device_ctrl_0, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_0, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: ");
	for (uint8_t i = 0; i < sizeof(default_value); i++)
		printf("0x%x\t", default_value[i]);
	printf("\r\n");

	printf("WRITE DATA \r\n");
	write_data[0] = send_byte[0];   // MSB of address
	write_data[1] = send_byte[1];   // LSB of address
	write_data[2] = 0xa2;            // Data to write
	R_I2C_Write(&g_i2c_device_ctrl_0, write_data, sizeof(write_data), 0);
	printf("WRITE DONE\r\n");

	/* Read back to verify */
	R_I2C_Write(&g_i2c_device_ctrl_0, &send_byte[0], no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_0, result, sizeof(result), false);
	printf("READ DATA AFTER WRITE: 0x%x\t", result[0]);
	printf("\r\n");

	if (result[0] == write_data[2]) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}


	write_data[2] = default_value[0];
	R_I2C_Write(&g_i2c_device_ctrl_0, write_data, sizeof(write_data), 0);

	/* Read again to confirm restore */
	R_I2C_Write(&g_i2c_device_ctrl_0, send_byte, no_bytes, false);
	R_I2C_Read(&g_i2c_device_ctrl_0, default_value, sizeof(default_value), false);
	printf("DEFAULT VALUE: 0x%x\t", default_value[0]);
	printf("\r\n");

	R_I2C_Close(&g_i2c_device_ctrl_0);
	printf("------------- TC2: END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_0.channel);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Device driver part for channel 1*/
	g_i2c_device_cfg_1 = (i2c_master_cfg_t)
	{
		.channel        = 1,
		.rate           = I2C_MASTER_RATE_FAST,
		.slave          = TCAL9539,
		.addr_mode      = I2C_MASTER_ADDR_MODE_7BIT,
		.dma_single     = true,
		.p_context      = &g_i2c_device_ctrl_1,
		.dmac_unit      = RT_DMAC0,
		.dmac_channel   = DMAC_CH3,
		.dmac_irq_id    = INTID_RTDMA0_CH3
	};

     send_data_dma[0] = TCAL9539_REG_ADDR;
     send_data_dma[1] = 0x03;
     send_data_dma[2] = 0x02;
     send_data_dma[3] = 0x03;

    printf("------------- TC3: START TEST I2C CHANNEL %d DMA MODE -------------\r\n", g_i2c_device_cfg_1.channel);
	
	/* Open I2C */
	R_I2C_Open(&g_i2c_device_ctrl_1, &g_i2c_device_cfg_1);

	/* Set interrupt I2C */
	R_I2C_CallbackSet(&g_i2c_device_ctrl_1, (void *)i2cUserCallback, &g_i2c_device_ctrl_1, NULL);

	/* Read default value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("DEFAULT VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	/* Write data */
	printf("WRITE DATA\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_1, send_data_dma, sizeof(send_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE DONE\r\n");
	}

	/* Read again to check write OK or not */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, send_data_dma[0], (uint8_t *)&result_dma, sizeof(result_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(result_dma, sizeof(result_dma));
			printf("READ DATA: ");
			for (uint8_t i = 0; i < sizeof(result_dma); i++)
				printf("0x%02x\t", result_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	bool test_passed = true;
	for (uint8_t i = 0; i < 3; i++) { 
		if (result_dma[i] != send_data_dma[i+1]) { // result_dma[0,1,2] compare send_data_dma[1,2,3]
			test_passed = false;
			break;
		}
	}
	if (test_passed) {
		printf(">>> TEST OK <<<\r\n");
	} else {
		printf(">>> TEST FAILED <<<\r\n");
	}

	/* Restore default value */
	restore_data_dma[0] = TCAL9539_REG_ADDR; // Register address
	restore_data_dma[1] = default_value_dma[0];
	restore_data_dma[2] = default_value_dma[1];
	restore_data_dma[3] = default_value_dma[2];
	restore_data_dma[4] = default_value_dma[3];
	printf("RESTORING ORIGINAL VALUES...\r\n");
	R_I2C_Write(&g_i2c_device_ctrl_1, restore_data_dma, sizeof(restore_data_dma), 0);
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		printf("WRITE RESTORE VALUE DONE\n");
	}

	/* Print restore value */
	R_I2C_ReadRegMap(&g_i2c_device_ctrl_1, send_data_dma[0], default_value_dma, sizeof(default_value_dma));
	if (xSemaphoreTake(xI2C_Semaphore, portMAX_DELAY) == pdTRUE)
	{
		if (transfer_success == true)
		{
			R_UTILS_ReadMemForDMA(default_value_dma, sizeof(default_value_dma));
			printf("RESTORED VALUE: ");
			for (uint8_t i = 0; i < sizeof(default_value_dma); i++)
				printf("0x%02x\t", default_value_dma[i]);
			printf("\r\n");
			transfer_success = false;
		}
		else 
		{
			printf("ERROR: I2C transfer failed\n");
			for (;;) {};
		}
	}

	/* Close instance */
	R_I2C_Close(&g_i2c_device_ctrl_1);
    printf("------------- TC3: END TEST I2C CHANNEL %d DMA MODE -------------\r\n", g_i2c_device_cfg_1.channel);
	
	/* Delete semaphore */
	vSemaphoreDelete(xI2C_Semaphore);
    printf("<APP_END>\n");
	for( ;; )
	{
	};
}

#endif  // (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2 || BOARD == X5H_VDK)

/*-----------------------------------------------------------*/

void i2cUserCallback(void *data) 
{
	i2c_master_callback_args_t *p_args = (i2c_master_callback_args_t *) data; 
	i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_args->p_context; /* Saved for later use by users */

	if (p_args->event == I2C_MASTER_EVENT_RX_COMPLETE || p_args->event == I2C_MASTER_EVENT_TX_COMPLETE)
	{
		transfer_success = true;
	}
	else 
	{
		transfer_success = false;
	}

	/* Give semaphore to unblock waiting task */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2C_Semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

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
