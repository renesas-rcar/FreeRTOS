/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
 
#include "adg.h"
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_state_manager.h"

// Clock_input
static const ClockInput clock_inputs[] = {
	{AUDIO_CLKA_FREQ, BRGCKR_BRGA_CLKA, BRGCKR_BRGB_CLKA},
	{AUDIO_CLKB_FREQ, BRGCKR_BRGA_CLKB, BRGCKR_BRGB_CLKB},
	{AUDIO_CLKC_FREQ, BRGCKR_BRGA_CLKC, BRGCKR_BRGB_CLKC},
	{S0D4PHI_FREQ, BRGCKR_BRGA_S0D4, BRGCKR_BRGB_S0D4},
};

static const uint32_t ssi_div_value[] = {2, 4, 6, 8, 12, 16};

bool adg_set_clock(adg_conf_t *adg, bool use_brgb){

	uint32_t best_input_freq = 0;
	uint32_t best_brgckr_val = 0;
	uint32_t best_cks = 0;
	uint32_t best_brr = 0;
	uint32_t min_error = UINT32_MAX;
	uint32_t best_div_ssi = 0;
	uint32_t best_sample_rate = 0;
	
	for(int j = 0; j < 7; j++) {
		uint32_t div_ssi = ssi_div_value[j];
		uint32_t req_clk = adg->word_size * 2 * adg->sample_rate * div_ssi; //adg->channel is disabled now
	// CKS (0: /1; 1: /4; 2: /16; 3: /64)
		for(int i = 0; i < 4; i++) {
			uint32_t input_freq = clock_input[i].freq;
			uint32_t brgckr_val = use_brgb ? clock_input[i].brgckr_brgb : clock_input[i].brgckr_brga; 
			
			for(uint32_t cks = 0; cks <= 3; cks++){
			uint32_t divider_base = ((uint32_t)1 << (cks * 2U)); //2^(CKS*2)
			float N = input_freq / req_clk;
			float brr_float = (N / (2*divider_base) ) - 1;
			uint32_t brr = (uint32_t)(brr_float + 0.5);
			
			if(brr > 255) {
				continue;
			}
			
			uint32_t actual_div = divider_base *2*(brr + 1);
			uint32_t actual_clk = input_freq / actual_div;
			uint32_t actual_sample_rate = actual_clk / (adg->word_size * 2 * div_ssi); //adg->channel is disabled now
			uint32_t cur_error = (actual_sample_rate >  adg->sample_rate) ? 
								 (actual_sample_rate - adg->sample_rate)  : 
								 (adg->sample_rate - actual_sample_rate)  ;
			
			if(cur_error < min_error){
				min_error = cur_error;
				best_input_freq = input_freq;
				best_brgckr_val = brgckr_val;
				best_cks = cks;
				best_brr = brr;
				best_div_ssi = div_ssi;
				best_sample_rate = actual_sample_rate;
				}
			}
		}
		
		if(min_error == UINT32_MAX) {
			return false;
		}
		
		adg->div_ssi = best_div_ssi;
		
		if (use_brgb){
			BRRB0 = (best_cks << 8) | best_brr;
			BRGCKR0 = best_brgckr_val ;
			
			BRRA0 = ((uint32_t)0 << 8U) | 0U;
			BRGCKR0 |=  BRGCKR_BRGA_CLKA | BRGCKR_OUT_BRGA ;
			
			AUDIO_CLK_SEL0_(channel_ssi) = 0x00000020;
		}
		else {
			BRRA0 = (best_cks << 8) | best_brr;
			BRGCKR0 = best_brgckr_val ;
			
			BRRB0 = ((uint32_t)0 << 8U) | 0U;
			BRGCKR0 |=  BRGCKR_BRGB_CLKA | BRGCKR_OUT_BRGB;
			
			AUDIO_CLK_SEL0_(channel_ssi) = 0x00000010;
		}
	}

	return true;
}

int adg_init(adg_conf_t *p_conf)
{
    // int ret = R_StateManager_PowerOn(X5H_CLOCK_ID_MDLC_ADG0);
    // printf("R_StateManager_PowerOn pass %d \n",ret);
    // int ret = R_StateManager_ClockOn(X5H_CLOCK_ID_MDLC_ADG0);
    // printf("R_StateManager_ClockOn pass %d \n",ret);   

	adg_set_clock(p_conf,u_brgb);
	
	return 0;
}
void adg_reset_reg(adg_conf_t *p_conf)
{
	if(p_conf == NULL)
	{
		return;
	}
	
	BRRB0 |= 0xFFu;
	
	BRRA0 |= 0xFFu;
	
	BRGCKR0 &= (0xFFu << 23);
	
	AUDIO_CLK_SEL0_(channel_ssi) = 0x00u;
}

