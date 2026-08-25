/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "FreeRTOS.h"
#include "task.h"

#include "wcrc/r_wcrc_common.h"
#include "wcrc/r_wcrc.h"
#include "r_crc_wrapper.h"
#include "rcar_utils.h"
#include "state-manager/r_state_manager.h"
#include "state-manager/r_clock_domain_id.h"
#include <stdio.h>
#include "board.h"

#include "board.h"

#if (BOARD == MDP_AIACC_HIL)
#include "module_controller.h"
#endif

#define CRC_OPEN    (0x00000001U)
#define CRC_CLOSE   (0x00000002U)

static int wcrc_get_clock_ids(wcrc_unit_t uinit, uint32_t *wcrc_id, uint32_t *crc_id, uint32_t *kcrc_id);
static int wcrc_enable_clock(wcrc_cfg_t const * const p_cfg);

int R_CRC_Open(wcrc_ctrl_t * const p_ctrl, wcrc_cfg_t const * const p_cfg)
{
    int ret;

    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;

    /* Save the configuration */
    p_instance_ctrl->p_cfg = p_cfg;

    /* Enable clock */
    ret = wcrc_enable_clock(p_cfg);
    if (ret != 0)
    {
        return ret;
    }

    /* Mark driver as initialized by setting the open value to the ASCII equivalent of "CRC" */
    p_instance_ctrl->open = CRC_OPEN;

    /* Request WCRC driver to set mode */
    ret = wcrcSetMode(p_instance_ctrl);
    if (ret != 0) {
        printf("wcrcSetMode: FAILED\n");
        return ret;
    }

    return ret;
}

int R_CRC_Set_Callback(wcrc_sub_module_t module, wcrc_ctrl_t * const p_ctrl,
                       void (* p_callback)(void *), void * const p_context)
{
    int ret = 0;

    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        ret = -1;
        printf("%s: Invalid module\n", __func__);
    }

    ret = wcrc_set_callback(module, p_instance_ctrl,
                           p_callback, p_context);

    return ret;
}

int R_CRC_Calculate(wcrc_ctrl_t * const p_ctrl)
{
    int ret;
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;

    p_instance_ctrl->crc_data[CRC_SUB_MODULE].is_done  = false;
    p_instance_ctrl->crc_data[KCRC_SUB_MODULE].is_done = false;

    /* Start WCRC */
    ret = wcrcStart(p_instance_ctrl);
    if (ret != 0) {
        printf("wcrcStart: FAILED\n");
        return ret;
    }

    return ret;
}

int R_CRC_Close(wcrc_ctrl_t * p_ctrl)
{
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;
    int ret = 0;

    ret = wcrcClose(p_instance_ctrl);

    /* Mark driver as closed */
    p_instance_ctrl->open = CRC_CLOSE;
    p_instance_ctrl->crc_data[CRC_SUB_MODULE].is_done  = false;
    p_instance_ctrl->crc_data[KCRC_SUB_MODULE].is_done = false;

    return ret;
}

static int is_done(wcrc_sub_module_t module, wcrc_ctrl_t * p_ctrl, uint32_t timeout)
{
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;
    bool is_done = p_instance_ctrl->crc_data[module].is_done;

    while (timeout-- && (is_done != true)) {
        is_done = p_instance_ctrl->crc_data[module].is_done;
        vTaskDelay(1);
    }

    if (is_done != true) {
        return -1;
    }

    return 0;
}

int R_CRC_Wait_Operation(wcrc_ctrl_t * p_ctrl, uint32_t timeout)
{
    int ret = 0;
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;
    wcrc_cfg_t * p_cfg = (wcrc_cfg_t *) p_instance_ctrl->p_cfg;

    switch (p_cfg->sub_module) {
    case CRC_SUB_MODULE:
        ret = is_done(CRC_SUB_MODULE, p_ctrl, timeout);
        break;
    case KCRC_SUB_MODULE:
        ret = is_done(KCRC_SUB_MODULE, p_ctrl, timeout);
        break;
    case CRC_KCRC_SUB_MODULE:
        ret  = is_done(CRC_SUB_MODULE, p_ctrl, timeout);
        ret |= is_done(KCRC_SUB_MODULE, p_ctrl, timeout);
        break;
    default:
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        break;
    };

    if (ret != 0) {
        printf("%s: Waiting timeout\n", __func__);
    }

    return ret;
}

static int get_crc_data(crc_output_t const * const p_crc_data)
{
    int index;
    uint32_t num_data;
    uint32_t * p_data;

    if (p_crc_data->is_done == false) {
        printf("%s: crc is running\n", __func__);
        return -1;
    }

    num_data = p_crc_data->num_data;
    p_data = (uint32_t *)p_crc_data->p_output_buffer;

    R_UTILS_InvalidateDCache((uint32_t)p_crc_data->p_output_buffer, num_data*sizeof(uint32_t));

    for (index = 0; index < num_data; index++) {
        printf(" 0x%x\n", (*p_data));
        p_data++;
    }

    return 0;
}

static int get_kcrc_data(crc_output_t const * const p_kcrc_data)
{
    int index;
    uint32_t num_data;
    uint32_t * p_data;

    if (p_kcrc_data->is_done == false) {
        printf("%s: kcrc is running\n", __func__);
        return -1;
    }

    num_data = p_kcrc_data->num_data;
    p_data = (uint32_t *)p_kcrc_data->p_output_buffer;

    R_UTILS_InvalidateDCache((uint32_t)p_kcrc_data->p_output_buffer, num_data*sizeof(uint32_t));

    for (index = 0; index < num_data; index++) {
        printf(" 0x%x\n", (*p_data));
        p_data++;
    }

    return 0;
}

int R_CRC_Get_Generated_Value(wcrc_ctrl_t const * const p_ctrl)
{
    int ret = 0;
    wcrc_instance_ctrl_t const * const p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;
    wcrc_cfg_t const * const p_cfg = (wcrc_cfg_t *) p_instance_ctrl->p_cfg;
    crc_output_t const * const p_crc_data  = &p_instance_ctrl->crc_data[CRC_SUB_MODULE];
    crc_output_t const * const p_kcrc_data = &p_instance_ctrl->crc_data[KCRC_SUB_MODULE];

    switch (p_cfg->sub_module) {
    case CRC_SUB_MODULE:
        printf("**** CRC data ****\n");
        ret = get_crc_data(p_crc_data);
        break;
    case KCRC_SUB_MODULE:
        printf("**** KCRC data ****\n");
        ret = get_kcrc_data(p_kcrc_data);
        break;
    case CRC_KCRC_SUB_MODULE:
        printf("**** CRC data ****\n");
        ret = get_crc_data(p_crc_data);

        printf("**** KCRC data ****\n");
        ret |= get_kcrc_data(p_kcrc_data);
        break;
    default:
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        break;
    }
    
    return ret;
}

static int get_crc_input(wcrc_cfg_t const * const p_cfg)
{
    int index;
    crc_input_t const * const input_cfg = &p_cfg->crc_cfg.input_cfg;
    uint32_t num_data;
    uint32_t * p_input_buffer;
    
    num_data = input_cfg->num_data;
    p_input_buffer = (uint32_t *)input_cfg->p_input_buffer;

    for (index = 0; index < num_data; index++) {
        printf(" 0x%x\n", (*p_input_buffer));
        p_input_buffer++;
    }

    return 0;
}

static int get_kcrc_input(wcrc_cfg_t const * const p_cfg)
{
    int index;
    crc_input_t const * const input_cfg = &p_cfg->kcrc_cfg.input_cfg;
    uint32_t num_data;
    uint32_t * p_input_buffer;
    
    num_data = input_cfg->num_data;
    p_input_buffer = (uint32_t *)input_cfg->p_input_buffer;

    for (index = 0; index < num_data; index++) {
        printf(" 0x%x\n", (*p_input_buffer));
        p_input_buffer++;
    }

    return 0;
}

int R_CRC_Get_Input_Data(wcrc_ctrl_t const * const p_ctrl)
{
    int ret = 0;
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;
    wcrc_cfg_t const * const p_cfg = (wcrc_cfg_t *) p_instance_ctrl->p_cfg;

    switch (p_cfg->sub_module) {
    case CRC_SUB_MODULE:
        printf("**** CRC input ****\n");
        ret = get_crc_input(p_cfg);
        break;
    case KCRC_SUB_MODULE:
        printf("**** KCRC input ****\n");
        ret = get_kcrc_input(p_cfg);
        break;
    case CRC_KCRC_SUB_MODULE:
        printf("**** CRC input ****\n");
        ret = get_crc_input(p_cfg);

        printf("**** KCRC input ****\n");
        ret |= get_kcrc_input(p_cfg);
        break;
    default:
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        break;
    }

    return ret;
}

int R_CRC_Set_BufferAddress(wcrc_ctrl_t * const p_ctrl, uint32_t crc_addr, uint32_t kcrc_addr)
{
    int ret = 0;
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;

    ret  = wcrcSetBufferAddress(CRC_SUB_MODULE, p_instance_ctrl, crc_addr);
    ret |= wcrcSetBufferAddress(KCRC_SUB_MODULE, p_instance_ctrl, kcrc_addr);

    return ret;
}

uint32_t R_CRC_Get_BufferSize(wcrc_sub_module_t module, wcrc_ctrl_t * const p_ctrl,
                             uint32_t * buf_size)
{
    uint32_t ret = 0;
    wcrc_instance_ctrl_t * p_instance_ctrl = (wcrc_instance_ctrl_t *) p_ctrl;

    if (module != CRC_SUB_MODULE && module != KCRC_SUB_MODULE) {
        ret = -1;
        printf("%s: Invalid module\n", __func__);
    }

    ret = wcrcGetCrcSize(module, p_instance_ctrl, buf_size);
    return ret;
}

static int wcrc_get_clock_ids(wcrc_unit_t unit, uint32_t *wcrc_id, uint32_t *crc_id, uint32_t *kcrc_id)
{
    if ((wcrc_id == NULL) || (crc_id == NULL) || (kcrc_id == NULL))
    {
        return -1;
    }

#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
    switch (unit)
    {
        case WCRC_00:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC0;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC0;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC0;
            break;
            
        case WCRC_01:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC1;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC1;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC1;
            break;

        case WCRC_02:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC2;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC2;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC2;
            break;

        case WCRC_03:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC3;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC3;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC3;
            break;

        case WCRC_04:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC4;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC4;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC4;
            break;

        case WCRC_05:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC5;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC5;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC5;
            break;

        case WCRC_06:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC6;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC6;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC6;
            break;

        case WCRC_07:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC7;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC7;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC7;
            break;

        case WCRC_08:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC8;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC8;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC8;
            break;

        case WCRC_09:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC9;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC9;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC9;
            break;

        case WCRC_10:
            *wcrc_id = X5H_CLOCK_ID_MDLC_WCRC10;
            *crc_id  = X5H_CLOCK_ID_MDLC_CRC10;
            *kcrc_id = X5H_CLOCK_ID_MDLC_KCRC10;
            break;
        default:
            return -1;
    }
#endif
    return 0;
}

#if (BOARD == MDP_AIACC_HIL)
static void WcrcRequestClockOn()
{
    uint32_t module_num = MODULE_NUM_RT;

    //WCRC
    mdlc_ms_module_run_bit_f(module_num, 12, 26);
    mdlc_ms_module_run_bit_f(module_num, 12, 28);
    mdlc_ms_module_run_bit_f(module_num, 12, 30);

    //CRC
    mdlc_ms_module_run_bit_f(module_num, 13, 16);
    mdlc_ms_module_run_bit_f(module_num, 13, 18);
    mdlc_ms_module_run_bit_f(module_num, 13, 20);

    //KCRC
    mdlc_ms_module_run_bit_f(module_num, 14, 6);
    mdlc_ms_module_run_bit_f(module_num, 14, 8);
    mdlc_ms_module_run_bit_f(module_num, 14, 10);
}
#endif

static int wcrc_enable_clock(wcrc_cfg_t const * const p_cfg)
{
    int ret;
    uint32_t wcrc_id = 0, crc_id = 0, kcrc_id = 0;

    if (p_cfg == NULL)
    {
        return -1;
    }

#if (BOARD == MDP_AIACC_HIL)
    WcrcRequestClockOn();
#endif

    ret = wcrc_get_clock_ids(p_cfg->unit, &wcrc_id, &crc_id, &kcrc_id);
    if (ret != 0) {
        return ret;
    }

    ret = R_StateManager_ClockOn(wcrc_id); 
    if (ret != 0) {
        return ret;
    }

    ret = R_StateManager_ClockOn(crc_id);  
    if (ret != 0) {
        return ret;
    }

    ret = R_StateManager_ClockOn(kcrc_id); 
    if (ret != 0) {
        return ret; 
    }
    
    return 0;
}