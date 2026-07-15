/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "dmac/dmac_common.h"
#include "interrupts.h"

#ifndef RDMAC_CTRL_COMMON_H
#define RDMAC_CTRL_COMMON_H

/* Return code of API */
#define drv_OK                                (0U)    /* API completed without any error. */
#define drv_FAIL                              (1U)    /* Failed. */

 /**
 *  @brief          DMA intialize
 *  @details
 *  @param[in]      mode
 *  @return         drv_OK
 *  @par    Refer   (none)
 *  @par    Modify  (none)
*/
uint16_t R_DMAC_RcarDmacCtrlInit(DMAC_t dev, rDmacPriorityMode_t mode);

/**
 *  @brief          DMA Execute
 *  @details
 *  @param[in]      cfg
 *  @param[in]      descCfg
 *  @param[in]      cb
 *  @return         drv_OK
 *  @return         drv_ERR_NOT_INITIALIZED
 *  @return         drv_RTDMAC_CH_BUSY
 *  @par    Refer   (none)
 *  @par    Modify  (none)
*/
uint16_t R_DMAC_RcarDmacExec(DMAC_t dev, uint8_t ch, rDmacCfg_t *cfg, rDmacDescCfg_t *descCfg);

/**
 *  @brief          DMA Stop
 *  @details
 *  @return         drv_OK
 *  @return         drv_ERR_NOT_INITIALIZED
 *  @par    Refer   (none)
 *  @par    Modify  (none)
*/
uint16_t R_DMAC_RcarDmacStop(DMAC_t dev, uint8_t ch);

/**
 * @brief Set a callback function for DMAC events.
 *
 * @param[in] p_ctrl Pointer to the control structure.
 * @param[in] p_callback Pointer to the callback function.
 * @param[in] p_context Pointer to the user context.
 *
 * @retval 0 on success.
 * @retval error code on failure.
 */
uint16_t R_DMAC_RcarCallBackSet(dmac_ctrl_t *const p_ctrl, void ( *p_callback)(void *), void * const p_context);

#endif  /* RDMAC_CTRL_COMMON_H */
