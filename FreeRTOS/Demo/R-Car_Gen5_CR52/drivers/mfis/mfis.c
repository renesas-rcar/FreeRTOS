/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "mfis/mfis.h"
#include "mfis/mfis_lock.h"
#include "mfis_internal.h"
#include "interrupts.h"
#include "rcar_utils.h"
#include "cmsis_rcar_gen5.h"
#include "mfis/r_mfis_private.h"

/* Get interrupt source number of a channel */
uint16_t mfis_get_int_source_num(struct mfis_channel *ch)
{
    uint16_t result = 0U;
    if(ch->type == MFIS_TYPE_RECEVER)
    {
        result = (uint16_t)(*(volatile uint32_t *) EICR(ch->ch)) >> 1U;
    }
    else
    {
        result = (uint16_t)(*(volatile uint32_t *) IICR(ch->ch)) >> 1U;
    }

    result = (result == 0) ? 0xff : result;
    return result;
}

/* Get message of a channel */
uint16_t mfis_get_message(struct mfis_channel *ch)
{
    if(ch->type == MFIS_TYPE_RECEVER)
    {
        return *(volatile uint16_t *) EMBR(ch->ch);
    }
    else
    {
        return *(volatile uint16_t *) IMBR(ch->ch);
    }
    
}

/* Interrupt callback sample */
void mfis_interrupt_cb(void* data)
{
    struct mfis_channel *ch = (struct mfis_channel*) data;
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
    uint32_t mfis_irq = (uint32_t) (MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION(0U) |
                        MFIS_SCP_IRQ_REG_INT(0U));
    uint32_t value;

    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        /* Unlock MFIS register write protection */
        *(volatile uint32_t *)(MFIS_SCP_X5H_UNLOCK_WRITE) = 0xACC00001U;
        MFIS_SCP_REG_MFISRSIICR(MFIS_SCP_BASE, mfis_rtcore_num) = 
                                                    mfis_irq & MFIS_SCP_IRQ_REG_MASK;
        if (ch->cb_function != NULL)
        {
            ch->cb_function(ch->arg);
        }
    }
    
    else
    {
        ch->int_source = mfis_get_int_source_num(ch);
        ch->recv_message = mfis_get_message(ch);

        if(ch->cb_function != (void*)0) {
            ch->cb_function(ch->arg);
        }

        /* Unlock MFIS register write protection */
        *(volatile uint32_t *)(MFIS_UNLOCK_WRITE) = 0xACC00001U;
    
        /* Clear interrupt flag */
        if(ch->type == MFIS_TYPE_RECEVER)
        {
            value = *(volatile uint32_t *)EICR(ch->ch);
            *(volatile uint32_t *)EICR(ch->ch) = value & 0xFFFEU;
        }
        else
        {
            value = *(volatile uint32_t *)EICR(ch->ch);
            *(volatile uint32_t *)IICR(ch->ch) = value & 0xFFFEU;
        }
    }
}

/* Initialize MFIS */
int mfis_init(struct mfis_channel *ch)
{
    unsigned int intid;
    //* Initialize */
    ch->int_source = 0U;
    ch->recv_message = 0U;
    
    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        intid = CURRENT_CORE_IDX + SCP2CR_INT_BASE_ID;
        *(volatile uint32_t *)(MFIS_SCP_X5H_UNLOCK_WRITE) = 0xACC00001U;
    }
    
    else
    {
        intid = MFIS_INTID(ch->ch, ch->type);
        /* Unlock MFIS register write protection */
        *(volatile uint32_t *)(MFIS_UNLOCK_WRITE) = 0xACC00001U;
    }
    
    /* Set callback function */
    Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void*) ch);
    /* Enable interrupt from Receiver to Sender */
    Irq_SetPriority(intid, IPRIORITY(5));
    Irq_Enable(intid); 

    return 0;
}

int mfis_deinit(struct mfis_channel *ch)
{
    uint32_t intid;

    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        intid = CURRENT_CORE_IDX + SCP2CR_INT_BASE_ID;
        Irq_Disable((unsigned int)intid);
        Irq_RemoveEntry((unsigned int)intid);
    }
    else
    {
        Irq_Disable((unsigned int)INTID_R_S(ch->ch));
        Irq_RemoveEntry((unsigned int)INTID_R_S(ch->ch));
    }
    
    return 0;
}

/* Trigger interrupt to Receiver channel ch
int_number is 15-bit integer for interrupt source 
*/
int mfis_trigger_interrupt(struct mfis_channel *ch, uint16_t int_number)
{
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
    uint32_t mfis_msg = 0U;
    uint32_t mfis_irq = (uint32_t) (MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION(0U) |
                    MFIS_SCP_IRQ_REG_INT(1U));

    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        *(volatile uint32_t *)(MFIS_SCP_X5H_UNLOCK_WRITE) = 0xACC00001U;
        MFIS_SCP_REG_MFISRSEMBR(MFIS_SCP_BASE, mfis_rtcore_num) = mfis_msg;
        MFIS_SCP_REG_MFISRSEICR(MFIS_SCP_BASE, mfis_rtcore_num) =
                                mfis_irq & MFIS_SCP_IRQ_REG_MASK;
    }
    
    else
    {
        if(ch->ch > (uint8_t) 63 || int_number > (uint16_t) 0x7FFF)
        {
            return -1;
        }
        /* Unlock MFIS register write protection */
        *(volatile uint32_t *)(MFIS_UNLOCK_WRITE) = 0xACC00001U;

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            *(volatile uint32_t *)IICR(ch->ch) = ((uint32_t)int_number << 1U) | 0x01U;
        }
        else
        {
            *(volatile uint32_t *)EICR(ch->ch) = ((uint32_t)int_number << 1U) | 0x01U;
        }
    }

    return 0;
}

/* Send a 32-bit int to Receiver via message register */
int mfis_send_message(struct mfis_channel *ch, uint32_t value)
{
    if(ch->ch > (uint8_t)63U)
    {
        return -1;
    }

    /* Unlock MFIS register write protection */
    *(volatile uint32_t *)(MFIS_UNLOCK_WRITE) = 0xACC00001U;
    if (ch->type == MFIS_TYPE_RECEVER)
    {
        *(volatile uint32_t *)IMBR(ch->ch) = value;
    }
    else
    {
        *(volatile uint32_t *)EMBR(ch->ch) = value;
    }
    return 0;
}

e_mfis_lock_status_t R_MFIS_LockAcquire(e_mfis_lock_id_t mfis_id, uint32_t timeout)
{
    e_mfis_lock_status_t ret;
    uint32_t timer_feq = R_UTILS_GetTimerFrequency();
    uint64_t start;
    uintptr_t mfis_lock_reg;

    if (mfis_id < MFIS_LOCK_ID_0 || mfis_id >= MFIS_LOCK_ID_MAX_NUM) {
        ret = MFIS_LOCK_ID_UNSUPPORTED;
        return ret;
    }

    if (mfis_id < MFIS_LOCK_ID_8) {
        mfis_lock_reg = (uintptr_t)MFIS_LOCK_0_7_BASE + (uintptr_t)mfis_id*4U;
    }
    else {
        mfis_lock_reg = (uintptr_t)MFIS_LOCK_8_63_BASE + (uintptr_t)mfis_id*4U;
    }

    start = R_UTILS_GetTimerCounter();
    while(1) {
        if ( *(volatile uint32_t*)mfis_lock_reg == MFIS_LOCK_IS_ACQUIRED ) {
            if ( (timeout == 0) ||
                ((R_UTILS_GetTimerCounter() - start)*1000U/timer_feq < timeout) )
            {
                continue;
            }
            else
            {
                ret = MFIS_LOCK_TIMEOUT;
                break;
            }
        }
        else {
            ret = MFIS_LOCK_SUCCESS;
            break;
        }
    }

    return ret;
}

e_mfis_lock_status_t R_MFIS_LockRelease(e_mfis_lock_id_t mfis_id)
{
    e_mfis_lock_status_t ret;
    uintptr_t mfis_lock_reg;

    if (mfis_id < MFIS_LOCK_ID_0 || mfis_id >= MFIS_LOCK_ID_MAX_NUM) {
        ret = MFIS_LOCK_ID_UNSUPPORTED;
        return ret;
    }

    if (mfis_id < MFIS_LOCK_ID_8) {
        mfis_lock_reg = (uintptr_t)MFIS_LOCK_0_7_BASE + (uintptr_t)mfis_id*4U;
    }
    else {
        mfis_lock_reg = (uintptr_t)MFIS_LOCK_8_63_BASE + (uintptr_t)mfis_id*4U;
    }

    *(volatile uint32_t*)MFIS_UNLOCK_WRITE = 0xACCE0001U;
    *(volatile uint32_t*)mfis_lock_reg = MFIS_LOCK_RELEASE;
    *(volatile uint32_t*)MFIS_UNLOCK_WRITE = 0xACC00000U;

    return MFIS_LOCK_SUCCESS;
}
