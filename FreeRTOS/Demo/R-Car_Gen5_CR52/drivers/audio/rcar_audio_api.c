/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rcar_audio_api.h"
#include "board.h"

audio_ssi_conf_t *audio_ssi_conf;
adg_conf_t *audio_adg_conf;

int r_ssi_init(st_audio_cfg_t const * const p_cfg)
{
	uint32_t reg_val = 0;

	if((audio_ssi_conf != NULL))
	{
		printf("SSI has already been initialized!\n");
		return -1;
	}

	if(p_cfg->mode <= CAPTURE_MODE)
	{
		audio_ssi_conf = (audio_ssi_conf == NULL) ? (audio_ssi_conf_t *)malloc(sizeof(ssi_conf_t)) : audio_ssi_conf;
		
		if(audio_ssi_conf == NULL)
		{
			printf("Can't allocate audio_ssi_conf!\n");
			return -1;
		}
		ssi_conf_t *ssi_conf = (ssi_conf_t *)malloc(sizeof(ssi_conf_t));
		if(ssi_conf == NULL)
		{
			printf("Can't allocate ssi_conf!\n");
			free(audio_ssi_conf);
			return -1;
		}
		memset(ssi_conf, 0, sizeof(ssi_conf_t));

		/* Master mode channel 5*/
		ssi_conf->cr_role &= ~(SCKD | SWSD);
		ssi_conf->cr_role |= (SCKD | SWSD);
		ssi_conf->chan = SSI_CHANNEL; // channel of ssi
		
		ssi_conf->cr_own = p_cfg->mode == PLAYBACK_MODE ? ssi_conf->cr_own | (TRMD) : ssi_conf->cr_own & ~(TRMD); // transmit / receive mode
		
		if(p_cfg->num_channel > 0 && p_cfg->num_channel < 3)
		{
			// ssi_conf->cr_own = (p_cfg->format == STEREO_FORMAT) ? ssi_conf->cr_own | ((p_cfg->num_channel-1) << 22) : ssi_conf->cr_own & ~(3 << 22);
			ssi_conf->cr_own = ssi_conf->cr_own & ~(3UL << 22);
		}
		else
		{
			printf("Invalid number of channels!\n");
			free(ssi_conf);
			free(audio_ssi_conf);
			return -1;
		}
		
		ssi_conf->wsr &= ~(WS_MODE);
		ssi_conf->wsr &= ~(MONO);
		ssi_conf->wsr &= ~((uint32_t)31 << 16);
		ssi_conf->cr_own &= ~(SWSP);

		if(p_cfg->bit_depth == TWO_BYTES_PER_SAMPLE)
		{
			ssi_conf->cr_own &= ~(63u << 16);
			ssi_conf->cr_own |= DWL_16;
			reg_val = SWL_16;
			ssi_conf->cr_own = (p_cfg->format == STEREO_FORMAT) ? ssi_conf->cr_own | reg_val : ssi_conf->cr_own | (1u << 16);
		}
		else
		{
			ssi_conf->cr_own &= ~((uint32_t)63 << 16);
			ssi_conf->cr_own |= DWL_32;
			reg_val = SWL_32;
			ssi_conf->cr_own |= reg_val;
		}
		
		if(p_cfg->mode == PLAYBACK_MODE)
		{
			audio_ssi_conf->playback = ssi_conf;
			audio_ssi_conf->capture = NULL;			
		}
		else
		{
			audio_ssi_conf->playback = NULL;
			audio_ssi_conf->capture = ssi_conf;
		}
		
		ssi_init(ssi_conf);
	}
	else
	{
		printf("Unsupported mode!\n");
	}

	return 0;
}

int r_ssi_config(st_audio_cfg_t const * const p_cfg)
{
	uint32_t reg_val = 0;
	if(p_cfg->mode <= CAPTURE_MODE)
	{
		ssi_conf_t *ssi_conf = audio_ssi_conf->playback != NULL ? audio_ssi_conf->playback : audio_ssi_conf->capture;
		if(ssi_conf == NULL)
		{
			printf("Can't find playback / capture config!\n");
			return -1;
		}

		/* Master mode channel 5*/
		ssi_conf->cr_role &= ~(SCKD | SWSD);
		ssi_conf->cr_role |= (SCKD | SWSD);
		ssi_conf->chan = SSI_CHANNEL; // channel of ssi
		
		ssi_conf->cr_own = p_cfg->mode == PLAYBACK_MODE ? ssi_conf->cr_own | (TRMD) : ssi_conf->cr_own & ~(TRMD); // transmit / receive mode
		
		if(p_cfg->num_channel > 0 && p_cfg->num_channel < 3)
		{
			// ssi_conf->cr_own = (p_cfg->format == STEREO_FORMAT) ? ssi_conf->cr_own | ((p_cfg->num_channel-1) << 22) : ssi_conf->cr_own & ~(3 << 22);
			ssi_conf->cr_own = ssi_conf->cr_own & ~((uint32_t)3 << 22);
		}
		else
		{
			printf("Invalid number of channels!\n");
			return -1;
		}

		ssi_conf->wsr &= ~(WS_MODE);
		ssi_conf->wsr &= ~(MONO);
		ssi_conf->wsr &= ~((uint32_t)31 << 16);
		ssi_conf->cr_own &= ~(SWSP);
		
		if(p_cfg->bit_depth == TWO_BYTES_PER_SAMPLE)
		{
			ssi_conf->cr_own &= ~(63u << 16);
			ssi_conf->cr_own |= DWL_16;
			reg_val = SWL_16;
			ssi_conf->cr_own = (p_cfg->format == STEREO_FORMAT) ? ssi_conf->cr_own | reg_val : ssi_conf->cr_own | (1u << 16);
		}
		else
		{
			ssi_conf->cr_own &= ~(63u << 16);
			ssi_conf->cr_own |= DWL_32;
			reg_val = SWL_32;
			ssi_conf->cr_own |= reg_val;
		}
		
		ssi_init(ssi_conf);
	}
	else
	{
		printf("Unsupported mode!\n");
	}

	return 0;
}

int r_adg_init(st_audio_cfg_t const * const p_cfg)
{
	if((audio_adg_conf != NULL))
	{
		printf("ADG has already been initialized!\n");
		return -1;
	}
	audio_adg_conf = (adg_conf_t *)malloc(sizeof(adg_conf_t));
	if(audio_adg_conf == NULL)
	{
		printf("Can't allocate audio_adg_conf!\n");
		return -1;
	}
	audio_adg_conf->word_size = p_cfg->bit_depth;
	audio_adg_conf->channel = p_cfg->num_channel;
	
	if(p_cfg->sample_rate == CLK_48000kHz)
	{
		audio_adg_conf->sample_rate = 48000;
	}	
	else
	{
		printf("This sample rate is currently not supported!\n");
		return -1;
	}
	
	adg_init(audio_adg_conf);
	return 0;
}

int r_ssi_start(st_audio_cfg_t const * const p_cfg)
{
	if(audio_ssi_conf == NULL)
	{
		printf("SSI module hasn't been initialized!\n");
		return -1;
	}
	if(p_cfg->mode <= CAPTURE_MODE)
	{
		ssi_conf_t *ssi_conf = p_cfg->mode == PLAYBACK_MODE ? audio_ssi_conf->playback : audio_ssi_conf->capture;
		ssi_start(ssi_conf);
	}
	else
	{
		printf("Unknow format!\n");
		return -1;
	}
	return 0;
}

int r_ssi_trans(uint32_t data)
{
	if(audio_ssi_conf == NULL)
	{
		printf("SSI module hasn't been initialized!\n");
		return -1;
	}
	ssi_conf_t *ssi_conf = audio_ssi_conf->playback;
	ssi_trans(ssi_conf, data);
	return 0;
}

int r_ssi_stop(st_audio_cfg_t const * const p_cfg)
{
	if(audio_ssi_conf == NULL)
	{
		printf("SSI module hasn't been initialized!\n");
		return -1;
	}
	if(p_cfg->mode <= CAPTURE_MODE)
	{
		ssi_conf_t *ssi_conf = p_cfg->mode == PLAYBACK_MODE ? audio_ssi_conf->playback : audio_ssi_conf->capture;
		ssi_stop(ssi_conf);
	}
	else
	{
		printf("Unknow format!\n");
		return -1;
	}
	return 0;
}

int r_ak4619_init(st_audio_cfg_t const * const p_cfg)
{
	ak4619_instance_set_t instance_set ={
		.format = STEREO_I2S_COMPATIBLE,
		.dac_1 = DAC_INPUT_FROM_SDIN1,
	};

	if( p_cfg->sample_rate == CLK_48000kHz)
	{
		instance_set.fs = FS_48K;
		instance_set.mclk_multiplier = MCLK_256_FS;
	}
	else
	{
		printf("Unsupported this sample rate\n");
		return -1;
	}

	int res;
	res = ak4619_power_on();
    if (res != 0)
    {
        printf("Can not setup AK4619\n");
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // delay 10ms
    // After this, PMDA1 bit = RSTN bit = 0, at reset state -> let config register after this state
	switch (p_cfg->bit_depth)
	{
	case 16:
		instance_set.didl_set = WORD_16BIT;
		break;
	case 32:
		instance_set.didl_set = WORD_32BIT;
	default:
		break;
	}
    res = ak4619_module_init(&instance_set, 2, 16);
    if (res != 0)
    {
        printf("Can not setup AK4619 module\n");
    }
 
	return res;
}

int r_audio_clock_on(void)
{
	int res;

#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	int clock_id = X5H_CLOCK_ID_MDLC_ADG0;
	res = R_StateManager_ClockOn(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
	clock_id = X5H_CLOCK_ID_MDLC_SSI0;
	res = R_StateManager_ClockOn(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }

	clock_id = X5H_CLOCK_ID_MDLC_SSI05;
	res = R_StateManager_ClockOn(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
#endif
  
	return res;
}

int r_audio_clock_off(void)
{
	int res;
#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	int clock_id = X5H_CLOCK_ID_MDLC_ADG0;
	res = R_StateManager_ClockOff(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
    else
    {
        printf("Set clock id %d OFF OK!\r\n", clock_id);
    }

	clock_id = X5H_CLOCK_ID_MDLC_SSI0;
	res = R_StateManager_ClockOff(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
    else
    {
        printf("Set clock id %d OFF OK!\r\n", clock_id);
    }
	clock_id = X5H_CLOCK_ID_MDLC_SSI05;
	res = R_StateManager_ClockOff(clock_id);
    if (res != 0)
    {
        printf("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
    else
    {
        printf("Set clock id %d OFF OK!\r\n", clock_id);
    }
#endif

	return res;
}

int r_deinit(void)
{
	ssi_reset_reg(audio_ssi_conf->playback);
	ssi_reset_reg(audio_ssi_conf->capture);
	
	adg_reset_reg(audio_adg_conf);
	
	if(audio_ssi_conf->playback != NULL)
	{
		free(audio_ssi_conf->playback);
	}
	if(audio_ssi_conf->capture != NULL)
	{
		free(audio_ssi_conf->capture);
	}
	if(audio_ssi_conf != NULL)
	{
		free(audio_ssi_conf);
	}
	if(audio_adg_conf != NULL)
	{
		free(audio_adg_conf);
	}

	ak4619_module_deinit();

	r_audio_clock_off();

	return 0;
}

