/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <audio/rcar_audio.h>
#include "rcar_audio_api.h"

int R_Audio_Init(st_audio_instance_ctrl_t * const p_instance_ctrl, st_audio_cfg_t const * const p_cfg)
{
	int ret;
	p_instance_ctrl->p_cfg = p_cfg;

	ret = r_audio_clock_on();
	if(ret < 0)
	{
		printf("Can't turn on clock for Audio driver!\n");
		return ret;
	}

	ret = r_ak4619_init(p_cfg);
	if(ret < 0)
	{
		printf("Can't set up AK4619 for Audio driver!\n");
		return ret;
	}
	
	ret = r_adg_init(p_cfg);
	if(ret < 0)
	{
		printf("Can't init ADG module!\n");
		return ret;
	}
	
	if(p_cfg->mode == PLAYBACK_MODE)
	{
		ret = r_ssi_init(p_cfg);
	}
	else
	{
		printf("This mode is currently not supported!\n");
		ret = -1;
	}
	
	if(ret < 0)
	{
		printf("Can't init SSI module!\n");
		return ret;
	}
	
	return ret;
}

int R_Audio_Start(st_audio_instance_ctrl_t * const p_instance_ctrl, const uint8_t * p_src, uint32_t const bytes)
{
	int ret;
	int i;
	if(p_instance_ctrl->p_cfg == NULL)
	{
		printf("p_instance_ctrl->p_cfg = NULL!\n");
		return -1;
	}
	ret = r_ssi_start((st_audio_cfg_t*)(p_instance_ctrl->p_cfg));
	if(ret < 0)
	{
		printf("Start audio failed!\n");
		return ret;
	}

	if(((st_audio_cfg_t*)(p_instance_ctrl->p_cfg))->mode == PLAYBACK_MODE)
	{
		uint32_t data = 0;
		uint8_t word_size = (uint8_t)(((st_audio_cfg_t*)(p_instance_ctrl->p_cfg))->bit_depth);
		uint8_t format = (uint8_t)(((st_audio_cfg_t*)(p_instance_ctrl->p_cfg))->format);

		if(word_size == TWO_BYTES_PER_SAMPLE)
		{
			if(format == STEREO_FORMAT)
			{
				for (int i = 0; i < bytes; i += 4) {
					uint16_t l = (p_src[i+1] << 8) | p_src[i];       // Little-endian: LSB first 
					uint16_t r = (p_src[i+3] << 8) | p_src[i+2];

					data = ((uint32_t)r << 16) | l;
					ret = r_ssi_trans(data);
				}
			}
			else if(format == MONAURAL_FORMAT)
			{
				for (int i = 0; i < bytes; i += 2) {
					uint16_t l = (p_src[i+1] << 8) | p_src[i];       // Little-endian: LSB first
					uint16_t r = (p_src[i+1] << 8) | p_src[i];

					data = ((uint32_t)r << 16) | l;
					ret = r_ssi_trans(data);
				}
			}
		}
		else if(word_size == FOUR_BYTES_PER_SAMPLE)
		{
			if(format == STEREO_FORMAT)
			{
				for (int i = 0; i < bytes; i += 4) {
					data = (p_src[i+3] << 24) | (p_src[i+2] << 16) | (p_src[i+1] << 8) | p_src[i];
					ret = r_ssi_trans(data);
				}
			}
			else if(format == MONAURAL_FORMAT)
			{
				for (int i = 0; i < bytes; i += 4) {
					data = (p_src[i+3] << 24) | (p_src[i+2] << 16) | (p_src[i+1] << 8) | p_src[i];
					ret = r_ssi_trans(data);
					ret = r_ssi_trans(data);
				}
			}
		}
	}
	else if(((st_audio_cfg_t*)(p_instance_ctrl->p_cfg))->mode == CAPTURE_MODE)
	{
		printf("This mode is currently not supported!\n");
		ret = -1;
	}
	else
	{
		printf("Unknow mode!\n");
		ret = -1;
	}
	
	return ret;
}

int R_Audio_Stop(st_audio_instance_ctrl_t * const p_instance_ctrl)
{
	int ret;
	if(p_instance_ctrl->p_cfg == NULL)
	{
		printf("p_instance_ctrl->p_cfg = NULL!\n");
		return -1;
	}
	ret = r_ssi_stop((st_audio_cfg_t*)(p_instance_ctrl->p_cfg));
	return ret;
}

int R_Audio_Config(st_audio_instance_ctrl_t * const p_instance_ctrl)
{
	int ret = 0;
	
	if(((st_audio_cfg_t*)(p_instance_ctrl->p_cfg))->mode == PLAYBACK_MODE)
	{
		ret = r_ssi_config((st_audio_cfg_t*)(p_instance_ctrl->p_cfg));
	}
	else
	{
		printf("This mode is currently not supported!\n");
		ret = -1;
	}
	
	
	if(ret < 0)
	{
		printf("Can't config SSI module!\n");
		return ret;
	}
	return ret;
}

int R_Audio_Deinit(st_audio_instance_ctrl_t * const p_instance_ctrl)
{
	int ret;
	
	p_instance_ctrl->p_cfg = NULL;
	
	ret = ak4619_module_deinit();
	ret = r_deinit();
	
	return ret;
}