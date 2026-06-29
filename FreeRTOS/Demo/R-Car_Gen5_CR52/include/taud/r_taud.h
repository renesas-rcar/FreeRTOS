/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/

#ifndef R_TAUD_H
#define R_TAUD_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TAUD_PWM_MAX_CHANNEL_NUM_PER_GROUP          15

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @brief TAUD error codes 
 */
typedef enum e_taud_err{
    TAUD_SUCCESS = 0,
    TAUD_ERR_ALREADY_OPEN,
    TAUD_ERR_INVALID_ARG,
    TAUD_ERR_HW_LOCKED,
    TAUD_ERR_UNIT_OR_CHANNEL_NOT_SUPORT,
    TAUD_ERR_INVALID_POINTER,
    TAUD_ERR_SET_CLK,
    TAUD_ERR_NO_FUNC_SUPORT,
    TAUD_ERR_UNKNOWN,
} e_taud_err_t;

/**
 * @brief TAUD unit
 */
typedef enum e_taud_unit{
    TAUD0 = 0,
    TAUD1,
    TAUDMAX,
} e_taud_unit_t;

/**
 * @brief TAUD channel of unit
 */
typedef enum e_taud_ch{
    TAUD_CH0 = 0,
    TAUD_CH1,
    TAUD_CH2,
    TAUD_CH3,
    TAUD_CH4,
    TAUD_CH5,
    TAUD_CH6,
    TAUD_CH7,
    TAUD_CH8,
    TAUD_CH9,
    TAUD_CH10,
    TAUD_CH11,
    TAUD_CH12,
    TAUD_CH13,
    TAUD_CH14,
    TAUD_CH15,
    TAUD_CH_MAX,
} e_taud_ch_t;

typedef enum e_taud_func{
    TAUD_PWM_OUTPUT_FUNCTION = 0,
} e_taud_func_t;

typedef enum e_taud_pwm_output_level
{
    TAUD_PWM_OUTPUT_LEVEL_LOW  = 0U,    ///< Pin level low
    TAUD_PWM_OUTPUT_LEVEL_HIGH = 1U,    ///< Pin level high
} e_taud_pwm_output_level_t;

/** Timer output polarity */

typedef enum e_taud_pwm_output_polarity
{
    TAUD_PWM_OUTPUT_POLARITY_ACTIVE_HIGH = 0U, ///< Positive logic output (active-high)
    TAUD_PWM_OUTPUT_POLARITY_ACTIVE_LOW  = 1U, ///< Negative logic output (active-low)
} e_taud_pwm_output_polarity_t;

typedef enum e_taud_state{
    TAUD_STATE_READY,
    TAUD_STATE_RUNNING,
    TAUD_STATE_ERROR,
} e_taud_state_t;

typedef struct st_taud_irq_Cfg {
    void               (*p_callback)(void *);     /*!<  Pointer to the callback function */
    void                *p_context;                 /*!<  Pointer to context to be passed into callback */
} st_taud_irq_Cfg_t;

typedef struct st_taud_slave_ch_cfg
{
    e_taud_ch_t                     ch;
    uint32_t                        duty;
    uint32_t                        phase;
    e_taud_pwm_output_level_t       output_level;    ///< Setting of output level for TAU
    e_taud_pwm_output_polarity_t    output_polarity; ///< Setting of output polarity for TAU
    st_taud_irq_Cfg_t               duty_end_irq;
} st_taud_slave_ch_cfg_t;


typedef struct st_taud_pwm_cfg
{
    e_taud_ch_t             master_ch;
    uint8_t                 slave_num;
    st_taud_slave_ch_cfg_t *p_slave;
    uint32_t                freq_hz;
    st_taud_irq_Cfg_t       cycle_end_irq;
    uint8_t                 clk_src;
} st_taud_pwm_cfg_t;

typedef union u_tau_func_cfg
{
    st_taud_pwm_cfg_t    pwm;
} u_taud_func_cfg_t;


/**
 * @brief   TAUD configuration structure
 * @details This structure defines the common configuration parameters for a TAUD (Timer Array Unit D)
 *          instance. It includes the hardware unit, clock source, prescaler, operating function type,
 *          and a pointer to a function-specific configuration structure. It may also contain optional
 *          interrupt configuration parameters that apply to the entire TAUD unit.
 * @note    Each TAUD unit (e.g., TAUD0, TAUD1) can contain multiple channels, and each channel can be
 *          assigned a specific function such as PWM output, input capture, or interval timer.
 */
typedef struct st_taud_cfg
{
    e_taud_unit_t               unit;           /**< TAUD hardware unit (e.g., TAUD0, TAUD1, etc.) */
    e_taud_func_t               func;           /**< Functional mode of operation (PWM, CAPTURE, INTERVAL, etc.) */
    u_taud_func_cfg_t           func_cfg;       /**< Function-specific configuration structure */
} st_taud_cfg_t;

typedef void r_taud_ctrl_t;

/**
 * @brief   TAUD instance control structure
 * @details This structure maintains the runtime state and references for a TAUD instance.
 *          It is used internally by the driver to track the hardware unit, current configuration,
 *          and operational status of the instance.
 * @note    This structure should not be modified by the user directly.
 */
typedef struct st_taud_instance_ctrl
{
    bool                        open;           /**< Flag indicating whether this instance is initialized */
    st_taud_cfg_t const        *p_cfg;          /**< Pointer to the original configuration passed by user */
    e_taud_unit_t               unit;           /**< Associated TAUD unit (e.g., TAUD0, TAUD1, etc.) */
    e_taud_func_t               func;           /**< Active functional mode (PWM, CAPTURE, INTERVAL, etc.) */
    u_taud_func_cfg_t           func_cfg;       /**< Function-specific configuration structure */

    e_taud_state_t              state;          /**< Internal driver state (e.g., READY, RUNNING, ERROR) */

    /* Optional: For IRQ handling */
    st_taud_irq_Cfg_t          *p_irq_table;    /**< Pointer to IRQ config table per channel */
    
    /* Optional: For timing/runtime control */
    uint32_t                    channel_mask;

} st_taud_instance_ctrl_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
e_taud_err_t R_TAUD_PWM_Open(r_taud_ctrl_t * const p_ctrl, st_taud_cfg_t const * const p_cfg);
e_taud_err_t R_TAUD_PWM_Start(r_taud_ctrl_t * const p_ctrl);
e_taud_err_t R_TAUD_PWM_Stop(r_taud_ctrl_t * const p_ctrl);

e_taud_err_t R_TAUD_PWM_UpdateDuty(r_taud_ctrl_t * const p_ctrl, e_taud_ch_t ch, uint32_t duty);
e_taud_err_t R_TAUD_PWM_UpdateFreq(r_taud_ctrl_t * const p_ctrl, uint32_t freq_hz);

e_taud_err_t R_TAUD_PWM_CallbackSet(r_taud_ctrl_t * const p_ctrl,
                                e_taud_ch_t ch,
                                st_taud_irq_Cfg_t irg_cfg);

e_taud_err_t R_TAUD_PWM_Close(r_taud_ctrl_t * const p_ctrl);

#ifdef __cplusplus
}
#endif

#endif /* R_TAUD_H*/