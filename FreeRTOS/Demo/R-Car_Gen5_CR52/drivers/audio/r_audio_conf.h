/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT 
 *
 */

#ifndef R_AUDIO_CONF_H
#define R_AUDIO_CONF_H

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
 
/**
* @brief Set channel of SSI0 (0 - 9)
*
*/ 
#define SSI_CHANNEL (5) // According to real board connect

/**
 * @brief Set output clock for SSI and clkout0
 * Set to 1 BRGB for ssi and BRGA for clkout0
 * Set to 0 BRGA for ssi and BRGB for clkout0
 */ 
#define u_brgb 0

/**
 *@brief Set Frequency for input clock (Hz)
 *
 */
#define AUDIO_CLKA_FREQ          24576000 
#define AUDIO_CLKB_FREQ          0 
#define AUDIO_CLKC_FREQ          0 
#define S0D4PHI_FREQ             0 

/**
 *@brief Set SSI channel output from ADG
 *
 */
#define channel_ssi		5
#endif
