/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**********************************************************************************************************************
 Includes
 *********************************************************************************************************************/
#include "ak4619_module.h"
#include "gpio/r_gpio.h"
#include "i2c/r_i2c.h"
#include <stdio.h>

/***********************************************************************************************************************
 * Define
 **********************************************************************************************************************/
#define AK4619_I2C_ADDR  0x10 // shift right because LSB bit is set by I2C driver for read/write operation
#define I2C_CHANNEL      6

/***********************************************************************************************************************
 * I2C variables
 **********************************************************************************************************************/
static i2c_instance_ctrl_t g_i2c_device_ak4619;
static const i2c_master_cfg_t g_i2c_device_cfg_ak4619 =
{
    .channel       = I2C_CHANNEL,
    .rate          = I2C_MASTER_RATE_FAST,
    .slave         = AK4619_I2C_ADDR,
    .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
    .dma_single    = false,
    .p_context     = &g_i2c_device_ak4619,
};

/***********************************************************************************************************************
 * GPIO variables
 **********************************************************************************************************************/
gpio_pin_cfg_t g_gpio_pin_cfg[] =
{
    {
        .pin_cfg = GPIO_DIRECTION_OUTPUT,
        .pin = GPIO_PORT_06_PIN_20
    },
    {
        .pin_cfg = GPIO_DIRECTION_OUTPUT,
        .pin = GPIO_PORT_06_PIN_21
    },
};


gpio_cfg_t g_gpio_cfg =
{
    .number_of_pins = sizeof(g_gpio_pin_cfg) / sizeof(gpio_pin_cfg_t),
    .p_pin_cfg_data = g_gpio_pin_cfg,
    .p_extend = NULL
};

gpio_instance_ctrl_t g_gpio_instance_ctrl;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
// static uint8_t reg_value_fm_1 = 0x0C;
// static uint8_t reg_value_fm_2 = 0x0C;
// static uint8_t reg_value_dac_input = 0x04;
// static uint8_t reg_value_pw_mnm = 0x00;
// static uint8_t reg_value_sys_clk = 0x00;

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
static void i2cUserCallback(void *data) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) data;
}

static int ak4619_i2c_init(void)
{
    return 0;
}

static int ak4619_i2c_deinit(void)
{
    return 0;
}

static int write_i2c_reg(uint8_t reg_addr, uint8_t data)
{
    (void)reg_addr;
    (void)data;
    return 0;
}

static int set_pdn_pin_high(void)
{
    return 0;
}

static int ak4619_check_bick(e_ak4619_mclk_multiplier_t mclk_multiplier, uint8_t max_channel, uint8_t bit_per_channel)
{
    (void)mclk_multiplier;
    (void)max_channel;
    (void)bit_per_channel;
    return 0;
}

int ak4619_configure_clock(e_ak4619_mclk_multiplier_t mclk_multiplier,
                            uint8_t max_channel,
                            uint8_t bit_per_channel,
                            e_ak4619_fs_select_t fs)
{
    (void)mclk_multiplier;
    (void)max_channel;
    (void)bit_per_channel;
    (void)fs;
    return 0;
}

int ak4619_configure_audio_format(e_ak4619_interface_format_t format) 
{
    (void)format;
    return 0;
}

int ak4619_configure_word_length(e_ak4619_data_bit_length_t didl_set, e_ak4619_data_bit_length_t dodl_set)
{
    (void)didl_set;
    (void)dodl_set;
    return 0;
}

/* Board has only connect to SDIN1, so input must be SDIN1 */
int ak4619_configure_input_dac(e_ak4619_dac_source_t dac_1)
{
    (void)dac_1;
    return 0;
}

/* Now it only support use DAC1 */
int ak4619_set_dac(bool DAC)
{
    (void)DAC;
    return 0;
}

int ak4619_set_reset_bit(bool rstn_bit_set)
{
    (void)rstn_bit_set;
    return 0;
}

int ak4619_power_on(void)
{
    /* After PDN pin is up (using gpio), PMAD1 bit = PMAD2 bit = PMDA1 bit =
    PMDA2 bit = RSTN bit = “0” is defined as the standby state. After setting the control register, supply the
    necessary system clock (MCLK, BICK, LRCK) and then release the standby state. Once the power
    management bit (PMADx, PMDAx, x=1, 2) of the required block has been changed from “0” to “1” and
    the reset bit (RSTN bit) is changed from “0” to “1”, the normal operational state is established. */
    return 0;
}

int ak4916_set_volume(void)
{
    return 0;
}


int ak4619_module_init(ak4619_instance_set_t *instance_set, uint8_t max_channel,
                        uint8_t bit_per_channel)
{
    (void)instance_set;
    (void)max_channel;
    (void)bit_per_channel;
    return 0;  
}

int ak4619_module_deinit(void)
{
    return 0;
}
