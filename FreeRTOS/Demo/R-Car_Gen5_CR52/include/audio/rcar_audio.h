/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef RCAR_AUDIO_H
#define RCAR_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup Audio_Module Audio Module
 * @{
 * @brief This module provides functions to configure and use audio device driver.
 *
 * This file defines data structures and function prototypes for controlling audio operations,
 * including configure sample rate, bit depth, format, and number of channels.
 */

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Typedef definitions
 *************************************************************a*********************************************************/

/**
 * @brief Audio operation mode
 */
typedef enum e_audio_master_mode
{
	PLAYBACK_MODE = 0,
	CAPTURE_MODE,
} e_audio_master_mode_t;

/**
* @brief Frame layout – stereo or mono.
*/
typedef enum e_audio_master_format
{
	STEREO_FORMAT = 0,
	MONAURAL_FORMAT 		
} e_audio_master_format_t;

/**
* @brief Sample-rate selection for on-chip clock generator.
*/
typedef enum e_audio_master_sample_rate
{
	CLK_48000kHz = 0,
	CLK_96000kHz,						/* This sample rate is currently not supported! */
	CLK_192000kHz						/* This sample rate is currently not supported! */
} e_audio_master_sample_rate_t;

/**
* @brief Number of bits per audio sample.
*/
typedef enum e_audio_master_bit_depth_t
{
	TWO_BYTES_PER_SAMPLE = 16,
	FOUR_BYTES_PER_SAMPLE = 32 
} e_audio_master_bit_depth_t;

/**
* @brief Audio configuration for initialization.
*/
typedef struct st_audio_cfg
{
	e_audio_master_mode_t mode;        				///< mode: Audio operating.
	e_audio_master_bit_depth_t bit_depth;   	 	///< bit_depth: Word length (in bits) of each audio sample.
	uint8_t num_channel;							///< num_channel: Number of audio channels (e.g., 1 for mono, 2 for stereo). (Currently only support 2 chan due to DAC config
	e_audio_master_format_t format;					///< format: Audio format (STEREO_FORMAT or MONAURAL_FORMAT).
	e_audio_master_sample_rate_t sample_rate;		///< clock: Sample rate (CLK_44100kHz or CLK_48000kHz).
} st_audio_cfg_t;

/**
* @brief Driver instance – allocated by user, contents managed by driver.
*/
typedef struct st_audio_instance_ctrl
{
	st_audio_cfg_t const * p_cfg;
	
	void const * p_context;
} st_audio_instance_ctrl_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/

/**
 * @brief Initialize the audio peripheral with a given configuration.
 * 
 * @param[in] p_instance_ctrl 	- Pointer to the Audio control structure.
 * @param[in] p_cfg  			- Pointer to the configuration structure containing settings for the Audio.
 * @return 0 (SUCCESS)	Start successful.
 * @return Non-zero (FAIL) Faild to start audio.
 */
int R_Audio_Init(st_audio_instance_ctrl_t * const p_instance_ctrl, st_audio_cfg_t const * const p_cfg);

/**
 * @brief Start audio processing or playback.
 *
 * This function begins audio output or streaming using the configuration
 * provided during initialization.
 *
 * 
 * @param[in] p_instance_ctrl 	- Pointer to the audio control structure.
 * @param[in] p_src     		- Pointer to buffer that containing data of wav files.
 * @param[in] bytes     		- Number of bytes to read from buffer.
 * 
 * @return 0 (SUCCESS)	Start successful.
 * @return Non-zero (FAIL) Faild to start audio.
 */
int R_Audio_Start(st_audio_instance_ctrl_t * const p_instance_ctrl, const uint8_t * p_src, uint32_t const bytes);

/**
 * @brief Stop audio processing or playback.
 *
 * This function stops the current audio operation.
 *
 * @param[in] p_instance_ctrl Pointer to the audio control structure.
 * 
 * @return 0 (SUCCESS)	Stop successful.
 * @return Non-zero (FAIL) Faild to stop audio.
 */
int R_Audio_Stop(st_audio_instance_ctrl_t * const p_instance_ctrl);

/**
 * @brief Config audio module with the specified configuration.
 *
 * @param[in] p_instance_ctrl - Pointer to the audio control structure.
 * 
 * @return 0 (SUCCESS)	Initialization successful.
 * @return Non-zero (FAIL) Initialization failed.
 */
int R_Audio_Config(st_audio_instance_ctrl_t * const p_instance_ctrl);

/**
 * @brief Deinitialize the audio module.
 *
 * @param[in] p_instance_ctrl Pointer to the audio control structure.
 * 
 * @return 0 (SUCCESS)	Initialization successful.
 * @return Non-zero (FAIL) Initialization failed.
 */
int R_Audio_Deinit(st_audio_instance_ctrl_t * const p_instance_ctrl);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
