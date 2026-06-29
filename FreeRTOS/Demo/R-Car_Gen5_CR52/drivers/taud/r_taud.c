/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "taud/r_taud.h"
#include "r_taud_api.h"
#include "interrupts.h"
#include "state-manager/r_state_manager.h"
#include "state-manager/r_clock_domain_id.h"
#include <stdio.h>
#include "gic.h"


/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static uint32_t r_taud_get_clk_src(void);
static e_taud_err_t r_taud_pwm_output_func_open(r_taud_ctrl_t * const p_ctrl);
static e_taud_err_t r_taud_pwm_output_func_start(r_taud_ctrl_t * const p_ctrl);
static e_taud_err_t r_taud_pwm_output_func_stop(r_taud_ctrl_t * const p_ctrl);
static e_taud_err_t r_taud_pwm_hardware_initialize(e_taud_unit_t unit, e_taud_func_t func, st_taud_pwm_cfg_t * p_pwm_cfg);
static uint8_t r_taud_pwm_output_master_ch_setup(e_taud_unit_t unit, taud_clk_src_t clk_src, st_taud_pwm_cfg_t *p_pwm_cfg);
static uint8_t r_taud_pwm_output_slave_ch_setup(e_taud_unit_t unit, taud_clk_src_t clk_src, uint32_t freq_hz, st_taud_slave_ch_cfg_t * p_slave_cfg);
static uint8_t r_taud_irq_setup(e_taud_unit_t unit, e_taud_ch_t ch, st_taud_irq_Cfg_t *  p_irq_cfg);

static unsigned int taud_get_irq_id(uint8_t unit, uint8_t channel);
/***********************************************************************************************************************
 * ISR prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/
e_taud_err_t R_TAUD_PWM_Open (r_taud_ctrl_t * const p_ctrl, st_taud_cfg_t const * const p_cfg)
{
    st_taud_instance_ctrl_t *p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;

    if ((NULL == p_instance_ctrl) || (NULL == p_cfg))
    {
        return TAUD_ERR_INVALID_POINTER;
    }

    if (true == p_instance_ctrl->open)
    {
        return TAUD_ERR_ALREADY_OPEN;
    }

    if(p_cfg->unit >= TAUDMAX)
    {
        return TAUD_ERR_UNIT_OR_CHANNEL_NOT_SUPORT;
    }

    p_instance_ctrl->p_cfg      = p_cfg;
    p_instance_ctrl->func       = p_cfg->func;
    p_instance_ctrl->unit       = p_cfg->unit;
    p_instance_ctrl->func_cfg   = p_cfg->func_cfg;

    switch (p_instance_ctrl->func)
    {
    case TAUD_PWM_OUTPUT_FUNCTION:
        err = r_taud_pwm_output_func_open(p_instance_ctrl);
        break;
    
    default:
        err = TAUD_ERR_NO_FUNC_SUPORT;
        break;
    }
    

    p_instance_ctrl->open  = true;

    return err;
}

e_taud_err_t R_TAUD_PWM_Start(r_taud_ctrl_t * const p_ctrl)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;

    err = r_taud_pwm_output_func_start(p_instance_ctrl);

    return err;
}

e_taud_err_t R_TAUD_PWM_Stop(r_taud_ctrl_t * const p_ctrl)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;

    err = r_taud_pwm_output_func_stop(p_instance_ctrl);

    return err;
}

e_taud_err_t R_TAUD_PWM_UpdateDuty(r_taud_ctrl_t * const p_ctrl, e_taud_ch_t ch, uint32_t duty)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;
    st_taud_pwm_cfg_t *p_pwm_cfg = &p_instance_ctrl->func_cfg.pwm;
    

    for (uint8_t ch_index = 0; ch_index < p_pwm_cfg->slave_num; ch_index++)
    {
        if(p_pwm_cfg->p_slave[ch_index].ch == ch)
        {
            p_pwm_cfg->p_slave[ch_index].duty = duty;
            uint16_t count = (taud_pwm_get_clk(p_pwm_cfg->clk_src) * p_pwm_cfg->p_slave[ch_index].duty) / (p_instance_ctrl->func_cfg.pwm.freq_hz * 100);

            err = taud_pwm_set_count(p_instance_ctrl->unit, p_pwm_cfg->p_slave[ch_index].ch, count);
        }
        
    }
    

    return err;
}

e_taud_err_t R_TAUD_PWM_UpdateFreq(r_taud_ctrl_t * const p_ctrl, uint32_t freq_hz)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;
    st_taud_pwm_cfg_t *p_pwm_cfg = &p_instance_ctrl->func_cfg.pwm;
    p_pwm_cfg->freq_hz = freq_hz;

    uint16_t count = taud_pwm_get_clk(p_pwm_cfg->clk_src) / p_pwm_cfg->freq_hz;
    
    err = taud_pwm_set_count(p_instance_ctrl->unit, p_pwm_cfg->master_ch, count);

    return err;
}

e_taud_err_t R_TAUD_PWM_CallbackSet(r_taud_ctrl_t * const p_ctrl,
                                e_taud_ch_t ch,
                                st_taud_irq_Cfg_t irg_cfg)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;
    r_taud_irq_setup(p_instance_ctrl->unit, ch, &irg_cfg);

    return err;
}

e_taud_err_t R_TAUD_PWM_Close(r_taud_ctrl_t * const p_ctrl)
{
    st_taud_instance_ctrl_t * p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    e_taud_err_t err = TAUD_SUCCESS;

    return err;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
static uint8_t r_taud_setup_clk_src(void)
{
    static bool set_one_time = false;
    if (set_one_time)
    {
        return 0;
    }
    uint32_t rate;
    R_StateManager_ClockOn(X5H_CLOCK_ID_MDLC_TAUD0);
    R_StateManager_ClockOn(X5H_CLOCK_ID_MDLC_TAUD1);

    R_StateManager_GetClock(X5H_CLOCK_ID_CLK_BUSD8_SCP_MAIN, &rate);
    taud_set_src_clk_main(rate);
    set_one_time = true;

    return 0;
}

static e_taud_err_t r_taud_pwm_output_func_open(r_taud_ctrl_t * const p_ctrl)
{
    e_taud_err_t err = TAUD_SUCCESS;
    st_taud_instance_ctrl_t *p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    st_taud_pwm_cfg_t *p_pwm_cfg = &p_instance_ctrl->func_cfg.pwm;

    err = r_taud_setup_clk_src();
    

    p_instance_ctrl->channel_mask = (1U << p_pwm_cfg->master_ch);

    for (uint8_t i = 0; i < p_pwm_cfg->slave_num; i++)
    {
        p_instance_ctrl->channel_mask |= (1U << p_pwm_cfg->p_slave[i].ch);
    }


    p_instance_ctrl->p_irq_table = NULL;  

    err = r_taud_pwm_hardware_initialize(p_instance_ctrl->unit, p_instance_ctrl->func, p_pwm_cfg);

    if (p_pwm_cfg->cycle_end_irq.p_callback)
    {
        if(p_pwm_cfg->master_ch >= TAUD_CH_MAX)
        {
            return TAUD_ERR_UNIT_OR_CHANNEL_NOT_SUPORT;
        }
        r_taud_irq_setup(p_instance_ctrl->unit, p_pwm_cfg->master_ch, &p_pwm_cfg->cycle_end_irq);
    }

    for (uint8_t i = 0; i < p_pwm_cfg->slave_num; i++)
    {
        if (p_pwm_cfg->p_slave[i].duty_end_irq.p_callback)
        {
            if(p_pwm_cfg->p_slave[i].ch >= TAUD_CH_MAX)
            {
                return TAUD_ERR_UNIT_OR_CHANNEL_NOT_SUPORT;
            }
            r_taud_irq_setup(p_instance_ctrl->unit, p_pwm_cfg->p_slave[i].ch, &p_pwm_cfg->p_slave[i].duty_end_irq);
        }
    }

    return err;
}

static e_taud_err_t r_taud_pwm_output_func_start(r_taud_ctrl_t * const p_ctrl)
{
    e_taud_err_t err = TAUD_SUCCESS;
    st_taud_instance_ctrl_t *p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    err = taud_pwm_start(p_instance_ctrl->unit, p_instance_ctrl->channel_mask);
    return err;
}

static e_taud_err_t r_taud_pwm_output_func_stop(r_taud_ctrl_t * const p_ctrl)
{
    e_taud_err_t err = TAUD_SUCCESS;
    st_taud_instance_ctrl_t *p_instance_ctrl = (st_taud_instance_ctrl_t *) p_ctrl;
    err = taud_pwm_stop(p_instance_ctrl->unit, p_instance_ctrl->channel_mask);
    return err;
}

static uint8_t r_taud_pwm_output_master_ch_setup(e_taud_unit_t unit, taud_clk_src_t clk_src, st_taud_pwm_cfg_t *p_pwm_cfg)
{
    taud_channel_config_t ch_cfg;
    uint32_t clk_value = taud_pwm_get_clk(clk_src);

    ch_cfg.count = clk_value / p_pwm_cfg->freq_hz;

    ch_cfg.clk_src = clk_src;
    ch_cfg.unit = unit;
    ch_cfg.channel = p_pwm_cfg->master_ch;
    ch_cfg.is_master = TAUD_MASTER_CH;
    ch_cfg.output_en = 0;       //master only 0
     
    ch_cfg.mode = TAUD_CH_INTERVAL_TIMER_MODE;
    taud_pwm_channel_setup(&ch_cfg);

    return 0;
}

static uint8_t r_taud_pwm_output_slave_ch_setup(e_taud_unit_t unit, taud_clk_src_t clk_src, uint32_t freq_hz, st_taud_slave_ch_cfg_t * p_slave_cfg)
{
    taud_channel_config_t ch_cfg;
    uint32_t clk_value = taud_pwm_get_clk(clk_src);

    ch_cfg.count = (clk_value * p_slave_cfg->duty) / (freq_hz * 100) ;

    ch_cfg.clk_src = clk_src;
    ch_cfg.unit = unit;
    ch_cfg.channel = p_slave_cfg->ch;
    ch_cfg.is_master = TAUD_SLAVE_CH;
    ch_cfg.output_en = 0;       //slave 1
    ch_cfg.output_polarity = p_slave_cfg->output_polarity;
    
    ch_cfg.mode = TAUD_CH_ONE_COUNT_MODE;
    taud_pwm_channel_setup(&ch_cfg);

    return 0;
}

static e_taud_err_t r_taud_pwm_hardware_initialize(e_taud_unit_t unit, e_taud_func_t func, st_taud_pwm_cfg_t * p_pwm_cfg)
{
    // set clock CK
    tau_clock_t * clk_src;
    e_taud_err_t ret = TAUD_SUCCESS;

    clk_src = taud_pwm_set_clk(unit, p_pwm_cfg->freq_hz);
    if (clk_src == NULL)
    {
        return TAUD_ERR_SET_CLK;
    }
    
    p_pwm_cfg->clk_src = clk_src->clk_src;

    switch (func)
    {
    case TAUD_PWM_OUTPUT_FUNCTION:
        // setup master channne
        r_taud_pwm_output_master_ch_setup(unit, p_pwm_cfg->clk_src, p_pwm_cfg);

        for (int i = 0; i < p_pwm_cfg->slave_num; i++)
        {
            r_taud_pwm_output_slave_ch_setup(unit, p_pwm_cfg->clk_src,p_pwm_cfg->freq_hz, &p_pwm_cfg->p_slave[i]);
        }
        
        break;
    
    default:
        break;
    }

    return ret;
}

static uint8_t r_taud_irq_setup(e_taud_unit_t unit, e_taud_ch_t ch, st_taud_irq_Cfg_t *  p_irq_cfg)
{
    e_taud_err_t ret = TAUD_SUCCESS;
    unsigned int irq_id = taud_get_irq_id(unit, ch);
    /* Set Handler for Irq */
    Irq_SetupEntry(irq_id, p_irq_cfg->p_callback, p_irq_cfg->p_context);
    Irq_SetIntType(irq_id, TNT_TYPE_EDGE_TRIGGERED);
    /* Set priority for Irq */
    Irq_SetPriority(irq_id, IPRIORITY(3));
    /* Enable Irq */
    Irq_Enable(irq_id);

    return ret;
}

static unsigned int taud_get_irq_id(uint8_t unit, uint8_t channel)
{
    unsigned int id;
    id = INTID_TAUD0_CH0 + (unsigned int)channel + (unsigned int)(unit * 0x10);

    return id;
}
