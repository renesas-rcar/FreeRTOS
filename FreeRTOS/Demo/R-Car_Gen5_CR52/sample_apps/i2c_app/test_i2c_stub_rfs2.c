/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
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

#define I2C_STUB0_SLAVE_ADDR            (0x3F)
#define I2C_STUB0_BASE                  (0x3C001000)
#define I2C_STUB0_RXTX_REG              (I2C_STUB0_BASE + 0x24)

#define I2C_APP_SIZE (configMINIMAL_STACK_SIZE * 2)
#define I2C_TEST_CHANNEL_COUNT 				(2)
/*-----------------------------------------------------------*/

/*
 * Configure the hardware as necessary to run this demo.
 */
static void prvSetupHardware( void );

static void prvI2CTask( void *pvParameters );
static void i2cUserCallback(void *data);
static void vPrintTestSummary(void);

static bool channel1_init_ok = false;
static bool channel1_read_pio_ok = false;
static bool	channel1_write_pio_ok = false;
static bool channel2_init_ok = false;
static bool channel2_read_pio_ok = false;
static bool	channel2_write_pio_ok = false;

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

static void prvI2CTask( void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    /* For PIO mode */
    uint8_t send_data = 0x23;
    uint8_t result = 0;
    volatile uint32_t *rx_tx_reg = (volatile uint32_t *)I2C_STUB0_RXTX_REG;

    /* Device driver part for ch1 */
    i2c_instance_ctrl_t g_i2c_device_ctrl_1;
    i2c_master_cfg_t        g_i2c_device_cfg_1 =
    {
        .channel       = 1,
        .rate          = I2C_MASTER_RATE_FAST,
        .slave         = I2C_STUB0_SLAVE_ADDR,
        .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
        .p_callback    = NULL,     // Callback
        .p_context     = &g_i2c_device_ctrl_1,
    };
    printf("------------- Start I2C Test -------------\r\n");
    
    printf("------------- PIO Test: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);
    if (R_I2C_Open(&g_i2c_device_ctrl_1, &g_i2c_device_cfg_1) == 0)
    {
        channel1_init_ok = true;
    }
    else
    {
        printf("Error: Cannot open I2C%d\n", g_i2c_device_cfg_1.channel);
        for (;;) {};
    }
    
    printf("Before read I2C stub, result value is: 0x%02x\n", result);
    if (R_I2C_Read(&g_i2c_device_ctrl_1, &result, 1, false) != 0)
    {
        printf("Read FAILED\n");
    }
    printf("After read I2C stub, result value is: 0x%02x.\n", result);
    
    if (result == 0xFF) /* RFS2 stubs I2C read always return 0xFF */
    {
        channel1_read_pio_ok = true;
    }

    printf("Before write I2C stub, ICRXD_ICTXD value is: 0x%08x\n", *rx_tx_reg);
    if (R_I2C_Write(&g_i2c_device_ctrl_1, &send_data, 1, false) != 0)
    {
        printf("Write FAILED\n");
    }
    printf("After write I2C stub, ICRXD_ICTXD value is: 0x%08x\n", *rx_tx_reg);
    
    if (send_data == (uint8_t)(*rx_tx_reg))
    {
        channel1_write_pio_ok = true;
    }

    R_I2C_Close(&g_i2c_device_ctrl_1);
    printf("------------- END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_1.channel);

    result = 0;
    send_data = 0x25;
    /* Device driver part for ch2 */
    i2c_instance_ctrl_t g_i2c_device_ctrl_2;
    i2c_master_cfg_t        g_i2c_device_cfg_2 =
    {
        .channel       = 2,
        .rate          = I2C_MASTER_RATE_FAST,
        .slave         = I2C_STUB0_SLAVE_ADDR,
        .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
        .p_callback    = NULL,     // Callback
        .p_context     = &g_i2c_device_ctrl_2,
    };
    printf("------------- Start I2C Test -------------\r\n");
    
    printf("------------- PIO Test: START TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_2.channel);
    if (R_I2C_Open(&g_i2c_device_ctrl_2, &g_i2c_device_cfg_2) == 0)
    {
        channel2_init_ok = true;
    }
    else
    {
        printf("Error: Cannot open I2C%d\n", g_i2c_device_cfg_2.channel);
        for (;;) {};
    }
    
    printf("Before read I2C stub, result value is: 0x%02x\n", result);
    if (R_I2C_Read(&g_i2c_device_ctrl_2, &result, 1, false) != 0)
    {
        printf("Read FAILED\n");
    }
    printf("After read I2C stub, result value is: 0x%02x\n", result);

    if (result == 0xFF)
    {
        channel2_read_pio_ok = true;
    }

    printf("Before write I2C stub, ICRXD_ICTXD value is: 0x%08x\n", *rx_tx_reg);
    if (R_I2C_Write(&g_i2c_device_ctrl_2, &send_data, 1, false) != 0)
    {
        printf("Write FAILED\n");
    }
    printf("After write I2C stub, ICRXD_ICTXD value is: 0x%08x\n", *rx_tx_reg);

    if (send_data == (uint8_t)(*rx_tx_reg))
    {
        channel2_write_pio_ok = true;
    }

    R_I2C_Close(&g_i2c_device_ctrl_2);
    printf("------------- END TEST I2C CHANNEL %d -------------\r\n", g_i2c_device_cfg_2.channel);

    vPrintTestSummary();
    printf("------------- End -------------\r\n");
    printf("<APP_END>\n");
    for( ;; )
    {
    };
}

static void vPrintTestSummary(void)
{
    bool tc1_init_ok = channel1_init_ok || channel2_init_ok;

    bool tc2_read_ok = channel1_read_pio_ok || channel2_read_pio_ok;

    bool tc3_write_ok = channel1_write_pio_ok || channel2_write_pio_ok;

    bool tc4_multi_channel_ok = channel1_init_ok && channel1_read_pio_ok && channel1_write_pio_ok &&  /* Validate all tested channels operate correctly. */
                                channel2_init_ok && channel2_read_pio_ok && channel2_write_pio_ok;

    printf("\r\n");
    printf("============= I2C TEST SUMMARY =============\r\n");

    printf("TC1 - Init I2C channel 	: %s\r\n", tc1_init_ok ? "PASS" : "FAILED");

    printf("TC2 - Read PIO       	: %s\r\n", tc2_read_ok ? "PASS" : "FAILED");

    printf("TC3 - Write PIO      	: %s\r\n", tc3_write_ok ? "PASS" : "FAILED");

    printf("TC4 - Multi-Channel 	: %s\r\n", tc4_multi_channel_ok ? "PASS" : "FAILED");

    if (!tc4_multi_channel_ok)
    {
        printf("  CH1: Init=%s Read=%s Write=%s\r\n",
               channel1_init_ok ? "OK" : "NG",
               channel1_read_pio_ok ? "OK" : "NG",
               channel1_write_pio_ok ? "OK" : "NG");

        printf("  CH2: Init=%s Read=%s Write=%s\r\n",
               channel2_init_ok ? "OK" : "NG",
               channel2_read_pio_ok ? "OK" : "NG",
               channel2_write_pio_ok ? "OK" : "NG");
    }

    printf("============================================\r\n");
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
