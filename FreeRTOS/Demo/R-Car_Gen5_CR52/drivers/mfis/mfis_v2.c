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
#include "stdio.h"
#include "mfis/r_mfis_private.h"

#define CLUSTER0_CORE0              0U
#define CLUSTER0_CORE1              1U

static inline uint32_t mfis_read32(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void mfis_write32(uintptr_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t mfis_read_bit(uintptr_t addr, uint32_t mask)
{
    return (*(volatile uint32_t *)addr) & mask;
}

static inline void mfis_set_bits(uintptr_t addr, uint32_t mask)
{
    *(volatile uint32_t *)addr |= mask;
}

static inline void mfis_clear_bits(uintptr_t addr, uint32_t mask)
{
    *(volatile uint32_t *)addr &= ~mask;
}

/* Get interrupt source number of a channel */
uint16_t mfis_get_int_source_num(struct mfis_channel *ch)
{
    (void)ch;

    uint16_t result = 1U;
    return result;
}

/* Get message of a channel */
uint16_t mfis_get_message(struct mfis_channel *ch)
{
    (void)ch;

    uint16_t result = 0U;
    return result;
}

void mfis_interrupt_cb(void *data);

/* Interrupt callback sample */
void mfis_interrupt_cb(void *data)
{
    struct mfis_channel *ch = (struct mfis_channel *)data;
	uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;

    if (ch->type == MFIS_TYPE_RECEVER)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);
        if (mfis_read_bit(MFISISR_RR_A(ch->ch), MFISISR_ACK_INT) != 0U)
        {
            /* Write 1 to MFISICR_A0.ACK_INT */
            mfis_set_bits(MFISICR_RR_A(ch->ch), MFISICR_ACK_INT_BIT);
        }

        if (mfis_read_bit(MFISISR_RR_A(ch->ch), MFISISR_RCV_INT) != 0U)
        {
            mfis_set_bits(MFISICR_RR_A(ch->ch), MFISICR_RCV_INT_BIT);
            mfis_set_bits(MFISCHN_RCV_ACK_RR_A(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

            ch->int_source = mfis_get_int_source_num(ch);
            ch->recv_message = mfis_get_message(ch);

            if (ch->cb_function != NULL)
            {
                ch->cb_function(ch->arg);
            }
        }
    }
    else if (ch->type == MFIS_TYPE_SENDER)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);
        if (mfis_read_bit(MFISISR_RR_B(ch->ch), MFISISR_RCV_INT) != 0U)
        {
            mfis_set_bits(MFISICR_RR_B(ch->ch), MFISICR_RCV_INT_BIT);
            mfis_set_bits(MFISCHN_RCV_ACK_RR_B(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

            ch->int_source = mfis_get_int_source_num(ch);
            ch->recv_message = mfis_get_message(ch);

            if (ch->cb_function != NULL)
            {
                ch->cb_function(ch->arg);
            }
        }

        if (mfis_read_bit(MFISISR_RR_B(ch->ch), MFISISR_ACK_INT) != 0U)
        {
            /* Write 1 to MFISICR_A0.ACK_INT */
            mfis_set_bits(MFISICR_RR_B(ch->ch), MFISICR_ACK_INT_BIT);
        }
    }

    else
    {
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, 0xACC00001U);
        if (mfis_read_bit(MFISISR_RS_B(mfis_rtcore_num), MFISISR_ACK_INT) != 0U)
        {
            mfis_set_bits(MFISICR_RS_B(mfis_rtcore_num), MFISICR_ACK_INT_BIT);
        }

        if (mfis_read_bit(MFISISR_RS_B(mfis_rtcore_num), MFISISR_RCV_INT) != 0U)
        {
            mfis_set_bits(MFISICR_RS_B(mfis_rtcore_num), MFISICR_RCV_INT_BIT);
            if (ch->cb_function != NULL)
            {
                ch->cb_function(ch->arg);
            }  
        }
    }
}

/* Initialize MFIS */
int mfis_init(struct mfis_channel *ch)
{
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
    int intid;
    int ret = 0;

    /* Initialize */
    ch->int_source = 0U;
    ch->recv_message = 0U;

    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, 0xACC00001U);
        if (mfis_rtcore_num == CLUSTER0_CORE0)
        {
            intid = SCP2RT0_IRQ_ID;
        }
        else if (mfis_rtcore_num == CLUSTER0_CORE1)
        {
            intid = SCP2RT1_IRQ_ID;
        }
        else
        {
            return -1;
        }
        /* Set Handler for Irq */
        Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void *)ch);
        /* Set priority for Irq */
        Irq_SetPriority(intid, IPRIORITY(5U));
        /* Enable Irq */
        Irq_Enable(intid);

        mfis_set_bits(MFISIMR_RS_B(mfis_rtcore_num), MFISIMR_ACK_INT_BIT); // Enable ACK interrupt
        while (mfis_read_bit(MFISCHN_ACK_ST_RS_B(mfis_rtcore_num), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {};
        mfis_set_bits(MFISIMR_RS_B(mfis_rtcore_num), MFISIMR_RCV_INT_BIT); // Enable receive interrupt
    }
    
    else
    {
        intid = (uint32_t)MFIS_INTID((uint32_t)ch->ch, (uint32_t)ch->type);
        /* Set callback function */
        Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void *)ch);
        /* Enable interrupt from Receiver to Sender */
        Irq_SetPriority(intid, IPRIORITY(2U));
        Irq_Enable(intid);
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);
    }


    if (ch->type == MFIS_TYPE_RECEVER)
    {
        mfis_set_bits(MFISIMR_RR_A(ch->ch), MFISIMR_ACK_INT_BIT);
        mfis_set_bits(MFISIMR_RR_A(ch->ch), MFISIMR_RCV_INT_BIT);

        while (mfis_read_bit(MFISCHN_ACK_ST_RR_A(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {
            /* wait for pending ack to clear */
        }
    }
    else if (ch->type == MFIS_TYPE_SENDER)
    {
        mfis_set_bits(MFISIMR_RR_B(ch->ch), MFISIMR_RCV_INT_BIT);
        mfis_set_bits(MFISIMR_RR_B(ch->ch), MFISIMR_ACK_INT_BIT);

        while (mfis_read_bit(MFISCHN_ACK_ST_RR_B(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {
            /* wait for pending ack to clear */
        }
    } else {
        /* Do nothing */
    }

    return ret;
}

int mfis_deinit(struct mfis_channel *ch)
{
    int ret = 0;
    uint32_t intid;
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;

    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        if (mfis_rtcore_num == CLUSTER0_CORE0)
        {
            intid = SCP2RT0_IRQ_ID;
        }
        else if (mfis_rtcore_num == CLUSTER0_CORE1)
        {
            intid = SCP2RT1_IRQ_ID;
        }
        else
        {
            return -1;
        } 
    }
    
    else
    {
        intid = (uint32_t)MFIS_INTID((uint32_t)ch->ch, (uint32_t)ch->type);
    }
    Irq_Disable(intid);
    Irq_RemoveEntry(intid);

    return ret;
}

/* Trigger interrupt to Receiver channel ch
 * int_number is 15-bit integer for interrupt source
 */
int mfis_trigger_interrupt(struct mfis_channel *ch, uint16_t int_number)
{
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
    if (ch->type == MFIS_TYPE_CR_SCP)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, 0xACC00001U);
        mfis_write32(MFISCHN_SIG_RS_B(mfis_rtcore_num), MFISCHN_SIG_SND_SIG_BIT);
    }

    else
    {
        if ((ch->ch > 63U) || (int_number > 0x7FFFU))
        {
            return -1;
        }
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            mfis_write32(MFISCHN_SIG_RR_A(ch->ch), MFISCHN_SIG_SND_SIG_BIT);
        }
        else
        {
            mfis_write32(MFISCHN_SIG_RR_B(ch->ch), MFISCHN_SIG_SND_SIG_BIT);
        }
    }
    
    return 0;
}

/* Send a 32-bit int to Receiver via message register */
int mfis_send_message(struct mfis_channel *ch, uint32_t value)
{
    (void)ch;
    (void)value;

    return 0;
}

e_mfis_lock_status_t R_MFIS_LockAcquire(e_mfis_lock_id_t mfis_id, uint32_t timeout)
{
    e_mfis_lock_status_t ret;
    uint32_t timer_feq = R_UTILS_GetTimerFrequency();
    uint64_t start;
    uintptr_t mfis_lock_reg;
    uint32_t base;
    uintptr_t offset;

    if ((mfis_id < MFIS_LOCK_ID_0) || (mfis_id >= MFIS_LOCK_ID_MAX_NUM))
    {
        ret = MFIS_LOCK_ID_UNSUPPORTED;
        return ret;
    }

    if (mfis_id < MFIS_LOCK_ID_8)
    {
        base = MFIS_LOCK_0_7_BASE;
    }
    else
    {
        base = MFIS_LOCK_8_63_BASE;
    }

    offset = (uintptr_t)mfis_id * 4U;
    mfis_lock_reg = base + offset;

    start = R_UTILS_GetTimerCounter();
    for (;;)
    {
        if (mfis_read32(mfis_lock_reg) == (uint32_t)MFIS_LOCK_IS_ACQUIRED)
        {
            if ((timeout == 0U) ||
                (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) < timeout))
            {
                continue;
            }
            else
            {
                ret = MFIS_LOCK_TIMEOUT;
                break;
            }
        }
        else
        {
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
    uint32_t base;
    uintptr_t offset;

    if ((mfis_id < MFIS_LOCK_ID_0) || (mfis_id >= MFIS_LOCK_ID_MAX_NUM))
    {
        ret = MFIS_LOCK_ID_UNSUPPORTED;
        return ret;
    }

    if (mfis_id < MFIS_LOCK_ID_8)
    {
        base = MFIS_LOCK_0_7_BASE;
    }
    else
    {
        base = MFIS_LOCK_8_63_BASE;
    }

    offset = (uintptr_t)mfis_id * 4U;
    mfis_lock_reg = base + offset;

    mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACCE0001U);
    mfis_write32(mfis_lock_reg, (uint32_t)MFIS_LOCK_RELEASE);
    mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00000U);

    return MFIS_LOCK_SUCCESS;
}