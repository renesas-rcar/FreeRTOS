/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_taud_api.h"
#include "r_taud_reg.h"
#include <stdio.h>
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TAUD_CLK_SRC_NUMBER     4
#define TAUD_PRESCALER_MAX      15
#define TAUD_INNER_DIV_MAX      65536
#define TAUD_COUNTER_MAX_TICKS  65535U
 /***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
static uint32_t scpbusd8_scp_main;
static tau_clock_t tau_clock[4]= {
    {.clk_src = TAUD_CLK_CK0, .clk_value = 0, .is_using = 0},
    {.clk_src = TAUD_CLK_CK1, .clk_value = 0, .is_using = 0},
    {.clk_src = TAUD_CLK_CK2, .clk_value = 0, .is_using = 0},
    {.clk_src = TAUD_CLK_CK3, .clk_value = 0, .is_using = 0},
};
/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static uint8_t taud_pwm_set_prescaler(uint8_t unit, taud_clk_src_t clk_src, uint8_t prescaler);

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
uint32_t taud_set_src_clk_main(uint32_t clock)
{
    scpbusd8_scp_main = clock;
    return scpbusd8_scp_main;
}

tau_clock_t * taud_pwm_set_clk(uint8_t unit, uint32_t freq_hz)
{
    tau_clock_t * clk_set = NULL;
    for (int ck_index = 0; ck_index < TAUD_CLK_SRC_NUMBER; ck_index++)
    {
        if(tau_clock[ck_index].is_using == 0)
        {
            clk_set = &tau_clock[ck_index];
            for (uint8_t presc = 0; presc <= TAUD_PRESCALER_MAX; presc++)
            {
                clk_set->clk_value = scpbusd8_scp_main / (uint32_t)(1U << presc); 
                if(clk_set->clk_value / freq_hz < TAUD_COUNTER_MAX_TICKS)
                {
                    clk_set->clk_src = ck_index;
                    clk_set->is_using = 1;
                    taud_pwm_set_prescaler(unit, clk_set->clk_src, presc);
                    return clk_set;
                }
            }

            if(clk_set->clk_src == TAUD_CLK_CK3)
            {
                clk_set->clk_value = scpbusd8_scp_main / (uint32_t)(1U << TAUD_PRESCALER_MAX);
                taud_pwm_set_prescaler(unit, clk_set->clk_src, TAUD_PRESCALER_MAX);

                uint32_t ck3_presc = clk_set->clk_value / TAUD_COUNTER_MAX_TICKS;

                if(ck3_presc > 0xff)
                {
                    return NULL;
                }

                clk_set->clk_value = clk_set->clk_value / ck3_presc;
                clk_set->clk_src = ck_index;
                clk_set->is_using = 1;
                reg8_BRS_t BRS;
                BRS.INT = (uint8_t)ck3_presc;
                R_TAUD_RegWrite8(DRV_REG_ADDR_TAUD_BRS(unit), BRS.INT);
                return clk_set;
            }
        }
    }

    return NULL;
}

uint32_t taud_pwm_get_clk(taud_clk_src_t clk_src)
{
    return tau_clock[clk_src].clk_value;
}

uint8_t taud_pwm_channel_setup(taud_channel_config_t * p_ch_cfg)
{
    reg16_CMOR_t CMOR;
    reg8_CMUR_t CMUR;
    reg16_CDR_t CDR;

    // Set count for the master channel
    CDR.INT = p_ch_cfg->count;
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_CDR(p_ch_cfg->unit, p_ch_cfg->channel), CDR.INT);
    // setup TAUDnCMORm for the Master Channel
    CMOR.INT = R_TAUD_RegRead16(DRV_REG_ADDR_TAUD_CMOR(p_ch_cfg->unit, p_ch_cfg->channel));
    CMOR.BIT.CKS = p_ch_cfg->clk_src;
    CMOR.BIT.CCS = 0B00;
    CMOR.BIT.MAS = p_ch_cfg->is_master;
    CMOR.BIT.COS = 0B00;
    CMOR.BIT.STS = (p_ch_cfg->is_master) ? TAUD_CMOR_STS_TRIGGER_COUNTER_USING_SOFTWARE : TAUD_CMOR_STS_TRIGGER_COUNTER_USING_MASTERCHANNEL;
    CMOR.BIT.MD  = p_ch_cfg->mode;
    CMOR.BIT.MD0 = 0x1;
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_CMOR(p_ch_cfg->unit, p_ch_cfg->channel), CMOR.INT);
    // setup TAUDnCMURm for the Master Channel
    CMUR.INT = R_TAUD_RegRead8(DRV_REG_ADDR_TAUD_CMUR(p_ch_cfg->unit, p_ch_cfg->channel));
    CMUR.BIT.TIS = 0B00;
    R_TAUD_RegWrite8(DRV_REG_ADDR_TAUD_CMUR(p_ch_cfg->unit, p_ch_cfg->channel), CMUR.INT);

    if(p_ch_cfg->output_en)
    {
        // Channel Output Mode for Slave Channels
        R_TAUD_CH_Set(DRV_REG_ADDR_TAUD_TOE(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Set(DRV_REG_ADDR_TAUD_TOM(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Set(DRV_REG_ADDR_TAUD_TOL(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TOC(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TDE(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TDM(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TDL(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TRE(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TRO(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TRC(p_ch_cfg->unit), p_ch_cfg->channel);
        R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_TME(p_ch_cfg->unit), p_ch_cfg->channel);
    }
    

    // Simultaneous Rewrite for the Master Channel
    R_TAUD_CH_Set(DRV_REG_ADDR_TAUD_RDE(p_ch_cfg->unit), p_ch_cfg->channel);
    R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_RDS(p_ch_cfg->unit), p_ch_cfg->channel);
    R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_RDM(p_ch_cfg->unit), p_ch_cfg->channel);
    R_TAUD_CH_Clear(DRV_REG_ADDR_TAUD_RDC(p_ch_cfg->unit), p_ch_cfg->channel);

    return 0;
}

uint8_t taud_pwm_start(uint8_t unit, uint16_t ch_mask)
{
    reg16_TS_t TS;
    TS.INT = R_TAUD_RegRead16(DRV_REG_ADDR_TAUD_TS(unit));
    TS.INT |= ch_mask;
    
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_TS(unit), TS.INT);

    return 0;
}

uint8_t taud_pwm_stop(uint8_t unit, uint16_t ch_mask)
{
    reg16_TS_t TT;
    TT.INT = R_TAUD_RegRead16(DRV_REG_ADDR_TAUD_TS(unit));
    TT.INT |= ch_mask;
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_TS(unit), TT.INT);

    return 0;
}

uint8_t taud_pwm_set_count(uint8_t unit, uint8_t ch, uint16_t count)
{
    reg16_CDR_t CDR;
    CDR.INT = count;
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_CDR(unit, ch), CDR.INT);
    
    return 0;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static uint8_t taud_pwm_set_prescaler(uint8_t unit, taud_clk_src_t clk_src, uint8_t prescaler)
{
    reg16_TPS_t TPS;
    TPS.INT = R_TAUD_RegRead16(DRV_REG_ADDR_TAUD_TPS(unit));
    switch (clk_src)
    {
    case TAUD_CLK_CK0:
        TPS.BIT.PRS0 = prescaler;
        break;

    case TAUD_CLK_CK1:
        TPS.BIT.PRS1 = prescaler;
        break;

    case TAUD_CLK_CK2:
        TPS.BIT.PRS2 = prescaler;
        break;

    case TAUD_CLK_CK3:
        TPS.BIT.PRS3 = prescaler;
        break;
    
    default:
        return 1;
        break;
    }
    R_TAUD_RegWrite16(DRV_REG_ADDR_TAUD_TPS(unit), TPS.INT);

    return 0;
}
