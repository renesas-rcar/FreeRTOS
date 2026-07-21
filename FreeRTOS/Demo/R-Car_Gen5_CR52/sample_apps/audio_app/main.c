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
#include "stdio.h"
#include <audio/rcar_audio.h>
#include "dummy_wav_data.h"
#include "pfc/r_pfc_api.h"
#include "device_tree.h"

#define main_Audio_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvAudioTestTask( void *pvParameters );

extern char _RAM_START;

st_audio_cfg_t audio_config = 
{
	.mode = PLAYBACK_MODE,
	.bit_depth = TWO_BYTES_PER_SAMPLE,
	.num_channel = 2,
	.format = STEREO_FORMAT,
	.sample_rate = CLK_48000kHz
};

st_audio_instance_ctrl_t g_audio_device_ctrl;

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware ready to run the demo. */
	prvSetupHardware();

    xTaskCreate( prvAudioTestTask, "Testing Audio", 4096, NULL, main_Audio_TASK_PRIORITY, NULL);
    
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

static void prvAudioTestTask( void *pvParameters )
{

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;
	int res;
	int init_flag = 1;
	unsigned int reg_val;

	printf("* Test case 1: Initializes Audio driver. *\r\n");
    res = R_Audio_Init(&g_audio_device_ctrl, &audio_config);

	if(res == 0)
	{
        printf("Result: Passed!\n");
	}
	else
	{
		printf("Result: Failed!\n");
	}
	printf("***********************************************\r\n");
	
	audio_config.num_channel = 2;
	printf("* Test case 2: Config Audio configuration. *\r\n");
    res = R_Audio_Config(&g_audio_device_ctrl);
	if(res == 0)
	{
		printf("Result: Passed!\n");
	}
	else
	{
		printf("Result: Failed!\n");
	}
	printf("***********************************************\r\n");
	
	printf("* Test case 3: Start Audio driver. *\r\n");
    res = R_Audio_Start(&g_audio_device_ctrl, file_audio_create(), file_audio_create_len());
	if(res == 0)
	{
		printf("Result: Passed!\n");
	}
	else
	{
		printf("Result: Failed!\n");
	}
	printf("***********************************************\r\n");
	
	printf("* Test case 4: Verify Audio sound output. *\r\n");
	printf("Result: Passed if the recorded file on your PC matches the input sound \n");
	printf("***********************************************\r\n");
	
	printf("* Test case 5: Stop Audio driver. *\r\n");
    res = R_Audio_Stop(&g_audio_device_ctrl);
	if(res == 0)
	{
		printf("Result: Passed!\n");
	}
	else
	{
		printf("Result: Failed!\n");
	}
	printf("***********************************************\r\n");
	
	
	printf("* Test case 6: Deinitialize Audio driver. *\r\n");
    res = R_Audio_Deinit(&g_audio_device_ctrl);
	if(res == 0)
	{
		printf("Result: Passed!\n");
	}
	else
	{
		printf("Result: Failed!\n");
	}
	printf("***********************************************\r\n");
	printf("<APP_END>\n");
	
    for( ;; )
    {
		
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
