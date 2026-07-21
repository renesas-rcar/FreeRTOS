/*
 * Copyright (c) 2025 Renesas Electronics Corporation
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
    if(R_I2C_Open(&g_i2c_device_ak4619, &g_i2c_device_cfg_ak4619) != 0)
    {
        printf("Can not open I2C\n");
        return -1;
    }
    R_I2C_CallbackSet(&g_i2c_device_ak4619, (void *)i2cUserCallback, &g_i2c_device_ak4619, NULL);    
    return 0;
}

static int ak4619_i2c_deinit(void)
{
    if(R_I2C_Close(&g_i2c_device_ak4619) != 0)
    {
        printf("Can not close I2C\n");
        return -1;
    }
    return 0;
}

static int write_i2c_reg(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    if(R_I2C_Write(&g_i2c_device_ak4619, write_buf, 2, false) != 0)
    {
        printf("Can not write using I2C\n");
        return -1;
    }
    return 0;
}

static int set_pdn_pin_high(void)
{
    gpio_level_t readLevel;
    R_GPIO_PinRead(&g_gpio_instance_ctrl, GPIO_PORT_06_PIN_20, &readLevel);

    if (R_GPIO_PinWrite(&g_gpio_instance_ctrl, GPIO_PORT_06_PIN_20, GPIO_LEVEL_HIGH) != 0)
    {
        printf("Can not write GPIO Port 6 Pin 20 High\n");
        return -1;
    }
    R_GPIO_PinRead(&g_gpio_instance_ctrl, GPIO_PORT_06_PIN_20, &readLevel);

    return 0;
}

static int ak4619_check_bick(e_ak4619_mclk_multiplier_t mclk_multiplier, uint8_t max_channel, uint8_t bit_per_channel)
{
    (void)mclk_multiplier;
    uint16_t total_bits = max_channel * bit_per_channel;
    if (total_bits == 32 || total_bits == 48 || total_bits == 64)
    {
        return 0;
    }
    else
    {
        printf("BICK invalid. Must use 16/24/32 bit per channel\n");
        return -1;
    }
}

int ak4619_configure_clock(e_ak4619_mclk_multiplier_t mclk_multiplier,
                            uint8_t max_channel,
                            uint8_t bit_per_channel,
                            e_ak4619_fs_select_t fs)
{
    uint8_t reg_clk_value = 0xFFU; // default Invalid
    if (ak4619_check_bick(mclk_multiplier, max_channel, bit_per_channel) != 0) 
    {
        return -1;
    }

    if ((fs != FS_48K) && (fs != FS_96K) && (fs != FS_192K))
    {
        printf("Invalid fs\n");
        return -1;
    }

    switch (mclk_multiplier) 
    {
        case MCLK_128_FS:
            if (fs == FS_192K)
            {
                reg_clk_value = CLOCK_128FS_192K;
            }
            break;

        case MCLK_256_FS:
            if (fs == FS_96K)
            {
                reg_clk_value = CLOCK_256FS_96K;  
            }
            else if (fs == FS_48K)
            {
                reg_clk_value = CLOCK_256FS_8K_48K;  
            }
            break;

        case MCLK_384_FS:
            if (fs == FS_48K)
            {
                reg_clk_value = CLOCK_384FS_8K_48K;
            }
            break;

        case MCLK_512_FS:
            if (fs == FS_48K)
            {
                reg_clk_value = CLOCK_512FS_8K_48K;
            }
            break;

        default:
            printf("Invalid MCLK multiplier\n");
            return -1;
    }

    if (reg_clk_value == 0xFF)
    {
        printf("fs not match with MCLK mode\n");
    }

    R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_SYSTEM_CLK_ADDRESS, &reg_clk_value, sizeof(reg_clk_value));

    if (write_i2c_reg(REG_SYSTEM_CLK_ADDRESS, reg_clk_value) != 0)
    {
        printf("Can not configure clock for AK4619 module using I2C\n");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_SYSTEM_CLK_ADDRESS, &reg_clk_value, sizeof(reg_clk_value));

    return 0;
}

int ak4619_configure_audio_format(e_ak4619_interface_format_t format) 
{
    uint8_t reg_value_fm_1 = 0;
    uint8_t reg_value_fm_2 = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_1, &reg_value_fm_1, sizeof(reg_value_fm_1)) != 0) 
    {
        printf("Can not read register format 1 to change audio format interface");
        return -1;
    }

    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_2, &reg_value_fm_2, sizeof(reg_value_fm_2)) != 0) 
    {
        printf("Can not read register format 2 to change audio format interface\n");
        return -1;
    } 

    if (format == STEREO_I2S_COMPATIBLE)
    {
        reg_value_fm_1 &= ~0b11110000; // Clear 4 MSB bit (TDM, DCF[2:0] bit) to set mode Stereo I2S, bit 2 3 is DSL set Slot Length default 11 (32bit)
        reg_value_fm_2 &= ~(1U << 4); // Clear bit 4 (Slot bit) to set mode Stereo I2S
        if ((write_i2c_reg(REG_AUDIO_IF_FORMAT_ADDRESS_1, reg_value_fm_1) != 0) || (write_i2c_reg(REG_AUDIO_IF_FORMAT_ADDRESS_2, reg_value_fm_2) != 0))
        {
            printf("Can not configure stereo i2s compatible mode using I2C\n");
            return -1;
        }

        R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_1, &reg_value_fm_1, sizeof(reg_value_fm_1));
        R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_2, &reg_value_fm_2, sizeof(reg_value_fm_2));
    }
    else 
    {
        printf("Just support one mode Stereo I2S Compatible\n");
        return -1;
    }
    return 0;
}

int ak4619_configure_word_length(e_ak4619_data_bit_length_t didl_set, e_ak4619_data_bit_length_t dodl_set)
{
    if (dodl_set == WORD_32BIT)
    {
        printf("SDOUT not available for 32 bit word");
        return -1;
    }
    uint8_t temp;
    uint8_t reg_value_fm_2 = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_2, &reg_value_fm_2, sizeof(reg_value_fm_2)) != 0) 
    {
        printf("Can not read register format 2 to change word length");
        return -1;
    } 
    
    reg_value_fm_2 &= ~0b00001111; // Clear 4 LSB bit, it use to change word length
    temp = ((uint8_t)didl_set << 2) | ((uint8_t)dodl_set);
    reg_value_fm_2 |= temp; // Set 4 LSB bit to change word length

    if (write_i2c_reg(REG_AUDIO_IF_FORMAT_ADDRESS_2, reg_value_fm_2) != 0)
    {
        printf("Can not configure word length");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_2, &reg_value_fm_2, sizeof(reg_value_fm_2));

    return 0;
}

/* Board has only connect to SDIN1, so input must be SDIN1 */
int ak4619_configure_input_dac(e_ak4619_dac_source_t dac_1)
{
    if (dac_1 != DAC_INPUT_FROM_SDIN1)
    {
        printf("Do not support another input, must use SDIN1");
        return -1;
    }
    uint8_t reg_value_dac_input = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_DAC_INPUT_SELECT_ADDRESS, &reg_value_dac_input, sizeof(reg_value_dac_input)) != 0) 
    {
        printf("Can not read register format 2 to change word length");
        return -1;
    } 

    reg_value_dac_input &= ~0b00000011; // Clear 2 LSB bit
    reg_value_dac_input |= dac_1; // dac_1 set 2 LSB bit
    if (write_i2c_reg(REG_DAC_INPUT_SELECT_ADDRESS, reg_value_dac_input) != 0)
    {
        printf("Can not configure SDIN1 input for DAC1");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_DAC_INPUT_SELECT_ADDRESS, &reg_value_dac_input, sizeof(reg_value_dac_input));
    return 0;
}

/* Now it only support use DAC1 */
int ak4619_set_dac(bool DAC)
{
    uint8_t reg_value_pw_mnm = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_POWER_MANAGEMENT_ADDRESS, &reg_value_pw_mnm, sizeof(reg_value_pw_mnm)) != 0) 
    {
        printf("Can not read register power management to use DAC1");
        return -1;
    } 

    if (DAC == true)
    {
        if (R_GPIO_PinWrite(&g_gpio_instance_ctrl, GPIO_PORT_06_PIN_21, GPIO_LEVEL_LOW) != 0)
        {
            printf("Can not write GPIO Port 6 Pin 21 Low to select SDIN1\n");
            return -1;
        }
        reg_value_pw_mnm |= 1U << 1; // set bit 1 of power management register to 1 -> enable DAC1
        if (write_i2c_reg(REG_POWER_MANAGEMENT_ADDRESS, reg_value_pw_mnm) != 0)
        {
            printf("Can not enable DAC1");
            return -1;
        }
        R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_POWER_MANAGEMENT_ADDRESS, &reg_value_pw_mnm, sizeof(reg_value_pw_mnm));

    }
    else if (DAC == false)
    {
        reg_value_pw_mnm &= ~(1U << 1); // set bit 1 of power management register to 0 -> disable DAC1
        if (write_i2c_reg(REG_POWER_MANAGEMENT_ADDRESS, reg_value_pw_mnm) != 0)
        {
            printf("Can not disable DAC1");
            return -1;
        }
    }
    return 0;
}

int ak4619_set_reset_bit(bool rstn_bit_set)
{
    uint8_t reg_value_pw_mnm = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_POWER_MANAGEMENT_ADDRESS, &reg_value_pw_mnm , sizeof(reg_value_pw_mnm)) != 0) 
    {
        printf("Can not read register power management to set RSTN bit");
        return -1;
    } 

    if (rstn_bit_set == true)
    {
        reg_value_pw_mnm |= 1U << 0; // set bit RSTN = 1 -> normal operation
        if (write_i2c_reg(REG_POWER_MANAGEMENT_ADDRESS, reg_value_pw_mnm) != 0)
        {
            printf("Can not set RSTN bit");
            return -1;
        }
        R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_POWER_MANAGEMENT_ADDRESS, &reg_value_pw_mnm , sizeof(reg_value_pw_mnm));

    }
    else if (rstn_bit_set == false)
    {
        reg_value_pw_mnm &= ~(1U << 0); // set bit RSTN = 0
        if (write_i2c_reg(REG_POWER_MANAGEMENT_ADDRESS, reg_value_pw_mnm) != 0)
        {
            printf("Can not clear RSTN bit");
            return -1;
        }
    }
    return 0;
}

int ak4619_power_on(void)
{
    int ret = R_GPIO_Open(&g_gpio_instance_ctrl, &g_gpio_cfg);
    if (ret != 0) 
    {
        printf("Can not open GPIO\n");
        return ret;
    }

    if (set_pdn_pin_high() != 0)
    {
        return -1;
    }
    /* After PDN pin is up (using gpio), PMAD1 bit = PMAD2 bit = PMDA1 bit =
    PMDA2 bit = RSTN bit = “0” is defined as the standby state. After setting the control register, supply the
    necessary system clock (MCLK, BICK, LRCK) and then release the standby state. Once the power
    management bit (PMADx, PMDAx, x=1, 2) of the required block has been changed from “0” to “1” and
    the reset bit (RSTN bit) is changed from “0” to “1”, the normal operational state is established. */
    return 0;
}

int ak4916_set_volume(void)
{
    uint8_t reg_v = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, 0x0E, &reg_v, sizeof(reg_v)) != 0) 
    {
        printf("Can not read Reg DAC1Left");
        return -1;
    } 

    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, 0x0F, &reg_v, sizeof(reg_v)) != 0) 
    {
        printf("Can not read Reg DAC1Right");
        return -1;
    } 

    reg_v = 0x48;
    if (write_i2c_reg(0x0E, reg_v) != 0)
    {
        printf("Can not configure Reg DAC1Left");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, 0x0E, &reg_v, sizeof(reg_v));

    if (write_i2c_reg(0x0F, reg_v) != 0)
    {
        printf("Can not configure Reg DAC1Right");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, 0x0F, &reg_v, sizeof(reg_v));

}


int ak4619_module_init(ak4619_instance_set_t *instance_set, uint8_t max_channel,
                        uint8_t bit_per_channel)
{
    ak4619_i2c_init();
    uint8_t ret = 0;
    ret = ak4619_set_reset_bit(false); // make sure to set RSTN bit = 0
    if (ret != 0) {
        return ret;
    }

    ret = ak4619_configure_word_length(instance_set->didl_set, WORD_24BIT); // DODL 24 bit hardcode because it dont use now, so use default for it
    if (ret != 0) {
        return ret;
    }
    
    ret = ak4619_configure_audio_format(instance_set->format); // only support one mode currently
    if (ret != 0) {
        return ret;
    }
    
    ret = ak4619_configure_clock(instance_set->mclk_multiplier, max_channel, bit_per_channel, instance_set->fs);
    if (ret != 0) {
        return ret;
    }

    uint8_t reg_v = 0;
    if (R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_1, &reg_v, sizeof(reg_v)) != 0) 
    {
        printf("Can not read Reg REG_AUDIO_IF_FORMAT_ADDRESS_1 slot lenght set");
        return -1;
    } 

    reg_v &= ~0b00001100;
    reg_v |= ((uint8_t)(instance_set->didl_set)<<2); // set DSL[1:0] = didl_set;
    if (write_i2c_reg(REG_AUDIO_IF_FORMAT_ADDRESS_1, reg_v) != 0)
    {
        printf("Can not configure Reg REG_AUDIO_IF_FORMAT_ADDRESS_1 slot lenght set");
        return -1;
    }
    R_I2C_ReadRegMap(&g_i2c_device_ak4619, REG_AUDIO_IF_FORMAT_ADDRESS_1, &reg_v, sizeof(reg_v));

    ak4916_set_volume();

    ret = ak4619_set_dac(true); // enable DAC1 
    if (ret != 0) { 
        return ret;
    }
    
    ret = ak4619_configure_input_dac(instance_set->dac_1);
    if (ret != 0) { 
        return ret;
    }
    
    ret = ak4619_set_reset_bit(true); // release reset state, set RSTN bit = 1 -> normal operation
    if (ret != 0) { 
        return ret;
    }
    
    return 0;  
}

int ak4619_module_deinit(void)
{
    ak4619_i2c_deinit();
    if (R_GPIO_PinWrite(&g_gpio_instance_ctrl, GPIO_PORT_06_PIN_20, GPIO_LEVEL_LOW) != 0)
    {
        printf("Can not write Port 6 Pin 20 Low\n");
        return -1;
    }

    if (R_GPIO_Close(&g_gpio_instance_ctrl) != 0)
    {
        printf("Can not close GPIO\n");
        return -1;
    }
    return 0;
}
