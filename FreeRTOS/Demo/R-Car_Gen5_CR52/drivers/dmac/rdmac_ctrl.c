/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>
 #include <stdio.h>
 #include "dmac_ctrl_common.h"
 #include "board.h"
 
 extern int printf_delay(const char *format, ...);
 
 /* Return code of API */
 #define drv_OK                                (0U)    /* API completed without any error. */
 #define drv_FAIL                              (1U)    /* Failed. */
 
 /*!
  *  @brief          RT-DMA intialize
  *  @details
  *  @param[in]      mode
  *  @return         drv_OK
  *  @par    Refer   (none)
  *  @par    Modify  (none)
 */
 uint16_t R_RTDMAC_RcarDmacCtrlInit(DMAC_t dev, rDmacPriorityMode_t mode)
 {
     uint8_t Value;
 
 
     Value = R_DMAC_RcarDmacCtrlInit(dev, mode);
     if (Value)
         printf_delay("RTDMAC initialization failed\n");
 
     return drv_OK;
 }
 
 /*!
  *  @brief          RT-DMA Execute
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
 uint16_t R_RTDMAC_RcarDmacExec(DMAC_t dev, uint8_t ch, rDmacCfg_t *cfg, rDmacDescCfg_t *descCfg)
 {
     uint8_t Value;
     uint16_t ret = drv_OK;

    #if (BOARD == X5H_IRONHIDE)
    /* Workaround for temporarily limitation of rt-dmac.
     * Workaround: transfer unit must be 16 bytes or less.
     */
    if ((cfg->mTransferUnit == DRV_RTDMAC_TRANS_UNIT_32BYTE) || (cfg->mTransferUnit == DRV_RTDMAC_TRANS_UNIT_64BYTE) ||
        (cfg->mTransferUnit == DRV_RTDMAC_TRANS_UNIT_MAX))
    {
        printf("RTDMAC error: transfer unit must be 16 bytes or less\n");
        return drv_FAIL;
    }

    /* Workaround for temporarily limitation of rt-dmac.
     * Workaround: source address must be 16-byte aligned to avoid 16-byte boundary crossing.
     */
    /* Workaround for no descriptor mode */
    if (cfg->mDMAMode == DRV_DMAC_DMA_NO_DESCRIPTOR)
    {
        if ((cfg->mSrcAddr & 0xFU) != 0U)
        {
            printf("RTDMAC error: source address 0x%08x must be 16-byte aligned\n", cfg->mSrcAddr);
            return drv_FAIL;
        }
    }
    /* Workaround for descriptor mode */
    else
    {
        rDmacDescMemCfg_t *p_desc_table = (rDmacDescMemCfg_t *)(uintptr_t)descCfg->mDescBaseAddr; // descriptor table base address
        for (int i = 0; i < descCfg->mDescCount; i++)
        {
            if ((p_desc_table[i].SAR & 0xFU) != 0U)
            {
                printf("RTDMAC error: descriptor[%d].SAR=0x%08x must be 16-byte aligned\n", i, p_desc_table[i].SAR);
                return drv_FAIL;
            } 
        }
    }
    #endif

     Value = R_DMAC_RcarDmacExec(dev, ch, cfg, descCfg);
     if (Value) {
         printf_delay("RTDMAC execution failed\n");
         return Value;
     }
     
     return ret;
 }
 
 /*!
  *  @brief          RT-DMA Stop
  *  @details
  *  @return         drv_OK
  *  @return         drv_ERR_NOT_INITIALIZED
  *  @par    Refer   (none)
  *  @par    Modify  (none)
  */
 uint16_t R_RTDMAC_RcarDmacStop(DMAC_t dev, uint8_t ch)
 {
     uint32_t Value;
     uint16_t ret = drv_OK;
 
     Value = R_DMAC_RcarDmacStop(dev, ch);
     if (Value) {
         printf_delay("RTDMAC stop failed\n");
         return Value;
     }
     
     return ret;
}

 /*!
 *  @brief          InterruptHandler
 *  @details
 *  @param[in]      p_context     context irq handle.
 *  @par    Refer   (none)
 *  @par    Modify  (none)
 */
uint16_t R_RTDMAC_RcarCallBackSet(dmac_ctrl_t * const p_ctrl, void ( *p_callback)(void *), void * const p_context)
{
    uint32_t Value;
    uint16_t ret = drv_OK;

    Value = R_DMAC_RcarCallBackSet(p_ctrl, p_callback, p_context);

    if (Value) {
        printf_delay("RTDMAC IRQ callback failed\n");
        return Value;
    }

    return ret;
}
