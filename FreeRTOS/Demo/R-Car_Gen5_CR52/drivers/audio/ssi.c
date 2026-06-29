/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "ssi.h"

int ssi_init(ssi_conf_t *p_conf)
{
	uint32_t cr_own = p_conf->cr_own;
	uint32_t cr_clk = p_conf->cr_clk;
	uint32_t cr_mode = p_conf->cr_mode;
	uint32_t cr_role = p_conf->cr_role;
	uint32_t wsr = p_conf->wsr;
	uint32_t reg_val;
	
	cr_own |= (uint32_t)FORCE;
	cr_own &= ~DEL; // delay between SSI_WS and SSI_SDATA
	// Disable DMA
	cr_own &= ~(DMEN);
	
	// Data interrupt
	cr_mode &= ~(UIEN | OIEN | IIEN | DIEN);
	cr_mode |= (uint32_t)DIEN;
	
	// Serial bit clock freq = oversampling clock freq / 2
	cr_clk &= ~(7 << 4);
	cr_clk |= (uint32_t)(1 << 4);
	
	// WS Continue function is enabled
	wsr |= (uint32_t)CONT; // this bit can only be set in master mode
	
	reg_val = cr_role;
	*((volatile uint32_t *)(SSICR0(p_conf->chan))) = reg_val;
	reg_val = wsr;
	*((volatile uint32_t *)(SSIWSR0(p_conf->chan))) = reg_val;
	reg_val = cr_own | cr_mode | cr_clk;
	*((volatile uint32_t *)(SSICR0(p_conf->chan))) |= reg_val;

	
	p_conf->cr_own = cr_own;
	p_conf->cr_mode = cr_mode;
	p_conf->cr_clk = cr_clk;
	p_conf->cr_role = cr_role;
	p_conf->wsr = wsr;
	
	return 0;
}


void ssi_start(ssi_conf_t *p_conf)
{
	uint32_t cr_own = p_conf->cr_own;
	cr_own |= (uint32_t)(EN);
	*((volatile uint32_t *)(SSICR0(p_conf->chan))) |= (EN);
	p_conf->cr_own = cr_own;
}

void ssi_stop(ssi_conf_t *p_conf)
{
	uint32_t cr_own = p_conf->cr_own;
	cr_own &= ~(EN);
	while(((*((volatile uint32_t *)(SSI0SR(p_conf->chan)))) & DIRQ) == 0u) {}
	*((volatile uint32_t *)(SSICR0(p_conf->chan))) &= ~(EN);
	while(((*((volatile uint32_t *)(SSI0SR(p_conf->chan)))) & IDST) == 0u) {}
	p_conf->cr_own = cr_own;
}

void ssi_trans(ssi_conf_t *p_conf, uint32_t data)
{
	while(((*((volatile uint32_t *)(SSI0SR(p_conf->chan)))) & DIRQ) == 0u) {}
	*((volatile uint32_t *)(SSI0TDR(p_conf->chan))) = data;
}

int ssi_receive(ssi_conf_t *p_conf)
{
	uint32_t data;
    while(((*((volatile uint32_t *)(SSI0SR(p_conf->chan)))) & DIRQ) == 0u) {}
    while(((*((volatile uint32_t *)(SSI0SR(p_conf->chan)))) & DIRQ) == 1u) {
			data = *((volatile uint32_t *)(SSI0RDR(p_conf->chan)));
    }
	return data;
}

void ssi_reset_reg(ssi_conf_t *p_conf)
{
	if(p_conf == NULL)
	{ 
		return;
	}
	*((volatile uint32_t *)(SSIWSR0(p_conf->chan))) = 0x00u;
	*((volatile uint32_t *)(SSICR0(p_conf->chan))) = 0x00u;
	*((volatile uint32_t *)(SSI0TDR(p_conf->chan))) = 0x00u;
	*((volatile uint32_t *)(SSI0RDR(p_conf->chan))) = 0x00u;
	*((volatile uint32_t *)(SSICRE0(p_conf->chan))) = 0x00u;
}
