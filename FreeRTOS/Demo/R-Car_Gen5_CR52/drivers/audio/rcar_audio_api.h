/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef RCAR_AUDIO_API_H
#define RCAR_AUDIO_API_H

#include "FreeRTOS.h"
#include "task.h"

#include <audio/rcar_audio.h>
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_state_manager.h"
#include "ssi.h"
#include "adg.h"
#include "ak4619_module.h"

int r_ssi_init(st_audio_cfg_t const * const p_cfg);
int r_ssi_config(st_audio_cfg_t const * const p_cfg);
int r_adg_init(st_audio_cfg_t const * const p_cfg);
int r_ak4619_init(st_audio_cfg_t const * const p_cfg);

int r_ssi_start(st_audio_cfg_t const * const p_cfg);
int r_ssi_trans(uint32_t data);
int r_ssi_stop(st_audio_cfg_t const * const p_cfg);

int r_audio_clock_on(void);
int r_audio_clock_off(void);

int r_deinit(void);

#endif
