/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef AK4619_MODULE_H
#define AK4619_MODULE_H

#include <stdint.h>
#include <stdbool.h>

/***********************************************************************************************************************
 * Define
 **********************************************************************************************************************/
#define REG_POWER_MANAGEMENT_ADDRESS  (0x00)
#define REG_AUDIO_IF_FORMAT_ADDRESS_1 (0x01)
#define REG_AUDIO_IF_FORMAT_ADDRESS_2 (0x02)
#define REG_SYSTEM_CLK_ADDRESS        (0x03)
#define REG_DAC_INPUT_SELECT_ADDRESS  (0x12)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef enum
{
    STEREO_I2S_COMPATIBLE = 0, // Default
    STEREO_MSB_JUSTIFIED,
} e_ak4619_interface_format_t; // Have more, 11 mode, but now just use mode 0

typedef enum
{
    CLOCK_256FS_8K_48K  = 0x00, // Default
    CLOCK_256FS_96K     = 0x01,
    CLOCK_384FS_8K_48K  = 0x02,
    CLOCK_512FS_8K_48K  = 0x03,
    CLOCK_128FS_192K    = 0x04,
} e_ak4619_clock_set_t; // System clock setting register

typedef enum 
{
    MCLK_256_FS = 256,
    MCLK_384_FS = 384,
    MCLK_512_FS = 512,
    MCLK_128_FS = 128,
} e_ak4619_mclk_multiplier_t; // Use to choose mode clock

typedef enum 
{
    FS_48K = 48000,
    FS_96K = 96000,
    FS_192K = 192000,
} e_ak4619_fs_select_t;

typedef enum 
{
    WORD_24BIT = 0x0,
    WORD_20BIT = 0x01,
    WORD_16BIT = 0x02,
    WORD_32BIT = 0x03, // SDOUT (DODL[1:0] bits) Not available for 32 bits.
} e_ak4619_data_bit_length_t; // Set word length for SDIN & SDOUT

typedef enum
{
    DAC_INPUT_FROM_SDIN1 = 0x00,
} e_ak4619_dac_source_t;

typedef struct st_ak4619_instance_set
{
    e_ak4619_interface_format_t format;
    e_ak4619_mclk_multiplier_t mclk_multiplier;
    e_ak4619_fs_select_t fs;
    e_ak4619_data_bit_length_t didl_set;
    e_ak4619_dac_source_t dac_1;
} ak4619_instance_set_t;

/**
 * @brief This function use to config clock mode of AK4619 module
 * 
 * @param mclk_multiplier fs * mclk_multiplier equal MCLK, it should be 128, 256, 384, 512. If not, function will return -1
 * @param max_channel Max channel using
 * @param bit_per_channel How many bits per channel
 * @param fs Sample rate, Hz
 * @return 0 if success
 */
int ak4619_configure_clock(e_ak4619_mclk_multiplier_t mclk_multiplier, uint8_t max_channel, uint8_t bit_per_channel, e_ak4619_fs_select_t fs);

/**
 * @brief This function use to configure audio interface format. Now it only support set mode 0 (Stereo mode I2S compatible)
 * 
 * @param format STEREO_I2S_COMPATIBLE
 * @return 0 if success
 */
int ak4619_configure_audio_format(e_ak4619_interface_format_t format);

/**
 * @brief This function use to configure word length
 * 
 * @param didl_set SDIN1/2 Word Length Setting
 * @param dodl_set SDOUT1/2 Word Length Setting
 * @return 0 if success
 */
int ak4619_configure_word_length(e_ak4619_data_bit_length_t didl_set, e_ak4619_data_bit_length_t dodl_set);

/**
 * @brief This function use to choose input for DAC. Currently, board just use SDIN1 for DAC1, so this function must follow it
 * 
 * @param dac_1 DAC_INPUT_FROM_SDIN1
 * @return 0 if success 
 */
int ak4619_configure_input_dac(e_ak4619_dac_source_t dac_1);

/**
 * @brief This function use to enable/disable DAC. It only support DAC1
 * 
 * @param DAC True/false to enable/disable DAC
 * @return 0 if success
 */
int ak4619_set_dac(bool DAC);

/**
 * @brief This function use to set 1/0 RSTN bit
 * 
 * @param rstn_bit_set True/false to set 1/0 RSTN bit
 * @return 0 if success
 */
int ak4619_set_reset_bit(bool rstn_bit_set); 

/**
 * @brief This function use to init GPIO, and set High for PDN pin to power up AK1619
 * 
 * @return 0 if success
 */
int ak4619_power_on(void);

/**
 * @brief This function use to Init module AK4619 ready to work
 * 
 * @param instance_set Variables config
 * @param max_channel Max channel will be used
 * @param bit_per_channel How many bits per channel
 * @return 0 if success
 */
int ak4619_module_init(ak4619_instance_set_t *instance_set, uint8_t max_channel, uint8_t bit_per_channel);

/**
 * @brief This function use to deinit GPIO, I2C, and set Low for PDN pin to power down AK4619
 * 
 * @return 0 if success
 */
int ak4619_module_deinit(void);

#endif
