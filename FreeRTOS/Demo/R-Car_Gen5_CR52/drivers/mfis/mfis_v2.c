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
#include "stdbool.h"
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

/**
 * @brief Validate the channel ID for CR-CA or CR-CR communication.
 *
 * @param[in] ch MFIS channel configuration.
 *
 * @return true if the channel ID is valid, false if it is outside the supported range.
 */
static inline bool r_mfis_is_channel_id_valid(struct mfis_channel *ch)
{
    if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
    {
        if (ch->ch >= (uint8_t)MFIS_CR_CA_CHMAX)
        {
            LogError(("%s: CR-CA MFIS ID channel is invalid, out of support range.\n", __func__));
            return false;
        }
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
    {
        if (ch->ch >= (uint8_t)MFIS_CR_CR_CHMAX)
        {
            LogError(("%s: CR-CR MFIS ID channel is invalid, out of support range.\n",__func__));
            return false;
        }
    }
    else
    {
        /* Do nothing */
    }
    return true;
}

/**
 * @brief Check whether the MFIS channel and interrupt number are valid for CR-CA or CR-CR communication.
 *
 * @param[in] ch         MFIS channel configuration.
 * @param[in] int_number Interrupt source number; must fit in 15 bits.
 *
 * @return true if valid, false otherwise.
 */
static inline bool r_mfis_is_trigger_interrupt_valid(struct mfis_channel *ch, uint16_t int_number)
{
    if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
    {
        if((ch->ch >= (uint8_t)MFIS_CR_CA_CHMAX) || (int_number > (uint16_t) 0x7FFF))
        {
            return false;
        }
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
    {
        if ((ch->ch >= (uint8_t)MFIS_CR_CR_CHMAX) || (int_number > (uint16_t)0x7FFF))
        {
            return false;
        }
    }
    else
    {
        /*
        * CR-CR is the legacy/default mode.
        * This fallback preserves compatibility with applications
        * that do not initialize core_pair.
        */
        if ((ch->ch >= (uint8_t)MFIS_CR_CR_CHMAX) || (int_number > (uint16_t)0x7FFF))
        {
            return false;
        }
    }

    return true;
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

    if (!r_mfis_is_channel_id_valid(ch))
    {
        return;
    }

    if (ch->core_pair== MFIS_CORE_PAIR_CR_SCP)
    {
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

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
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            if (mfis_read_bit(MFISISR_R_A(ch->ch), MFISISR_ACK_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_A(ch->ch), MFISICR_ACK_INT_BIT);
            }

            if (mfis_read_bit(MFISISR_R_A(ch->ch), MFISISR_RCV_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_A(ch->ch), MFISICR_RCV_INT_BIT);
                mfis_set_bits(MFISCHN_RCV_ACK_R_A(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

                ch->int_source = mfis_get_int_source_num(ch);
                ch->recv_message = mfis_get_message(ch);

                if (ch->cb_function != NULL)
                {
                    ch->cb_function(ch->arg);
                }
            }
        }
        else
        {
            if (mfis_read_bit(MFISISR_R_B(ch->ch), MFISISR_RCV_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_B(ch->ch), MFISICR_RCV_INT_BIT);
                mfis_set_bits(MFISCHN_RCV_ACK_R_B(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

                ch->int_source = mfis_get_int_source_num(ch);
                ch->recv_message = mfis_get_message(ch);

                if (ch->cb_function != NULL)
                {
                    ch->cb_function(ch->arg);
                }
            }

            if (mfis_read_bit(MFISISR_R_B(ch->ch), MFISISR_ACK_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_B(ch->ch), MFISICR_ACK_INT_BIT);
            }
        }
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
    {
        if((MFIS_RT_UNLOCK_WRITE_RA == 0U) && (MFISISR_RA_B(ch->ch) == 0U) && (MFISCHN_RCV_ACK_RA_B(ch->ch) == 0U))
        {
            return;
        }
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE_RA, MFIS_PROTECT_WRITE_DISABLE);

        if (mfis_read_bit(MFISISR_RA_B(ch->ch), MFISISR_RCV_INT) != 0U)
        {
            mfis_set_bits(MFISICR_RA_B(ch->ch), MFISICR_RCV_INT_BIT);
            mfis_set_bits(MFISCHN_RCV_ACK_RA_B(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

            ch->int_source = mfis_get_int_source_num(ch);
            ch->recv_message = mfis_get_message(ch);

            if (ch->cb_function != NULL)
            {
                    ch->cb_function(ch->arg);
            }
        }

        if (mfis_read_bit(MFISISR_RA_B(ch->ch), MFISISR_ACK_INT) != 0U)
        {
            /* Write 1 to MFISICR_A0.ACK_INT */
            mfis_set_bits(MFISICR_RA_B(ch->ch), MFISICR_ACK_INT_BIT);
        }
    }
    else
    {
        /*
        * CR-CR is the legacy/default mode.
        * This fallback preserves compatibility with applications
        * that do not initialize core_pair.
        */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            if (mfis_read_bit(MFISISR_R_A(ch->ch), MFISISR_ACK_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_A(ch->ch), MFISICR_ACK_INT_BIT);
            }

            if (mfis_read_bit(MFISISR_R_A(ch->ch), MFISISR_RCV_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_A(ch->ch), MFISICR_RCV_INT_BIT);
                mfis_set_bits(MFISCHN_RCV_ACK_R_A(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

                ch->int_source = mfis_get_int_source_num(ch);
                ch->recv_message = mfis_get_message(ch);

                if (ch->cb_function != NULL)
                {
                    ch->cb_function(ch->arg);
                }
            }
        }
        else
        {
            if (mfis_read_bit(MFISISR_R_B(ch->ch), MFISISR_RCV_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_B(ch->ch), MFISICR_RCV_INT_BIT);
                mfis_set_bits(MFISCHN_RCV_ACK_R_B(ch->ch), MFISCHN_RCV_ACK_RCV_ACK_BIT);

                ch->int_source = mfis_get_int_source_num(ch);
                ch->recv_message = mfis_get_message(ch);

                if (ch->cb_function != NULL)
                {
                    ch->cb_function(ch->arg);
                }
            }

            if (mfis_read_bit(MFISISR_R_B(ch->ch), MFISISR_ACK_INT) != 0U)
            {
                mfis_set_bits(MFISICR_R_B(ch->ch), MFISICR_ACK_INT_BIT);
            }
        }
    }
}

/* Initialize MFIS */
int mfis_init(struct mfis_channel *ch)
{
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;
    uint32_t timeout_ms = 1000U;
    uint64_t start;
    uint32_t timer_feq = R_UTILS_GetTimerFrequency();

    int intid;
    int ret = 0;

    /* Initialize */
    ch->int_source = 0U;
    ch->recv_message = 0U;

    if (!r_mfis_is_channel_id_valid(ch))
    {
        return -1;
    }

    if (ch->core_pair== MFIS_CORE_PAIR_CR_SCP)
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
        /* Set Handler for Irq */
        Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void *)ch);
        /* Set priority for Irq */
        Irq_SetPriority(intid, IPRIORITY(5U));
        /* Enable Irq */
        Irq_Enable(intid);
    }
    else
    {
        if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
        {
            intid = (uint32_t)MFIS_INTID_RR((uint32_t)ch->ch, (uint32_t)ch->type);
        }
        else if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
        {
            if (MFIS_INTID_RA((uint32_t)ch->ch) == 0U)
            {
                ret = -1;
                return ret;
            }
            else
            {
                intid = (uint32_t)MFIS_INTID_RA((uint32_t)ch->ch);
            }
        }
        else
        {
            /*
            * CR-CR is the legacy/default mode.
            * This fallback preserves compatibility with applications
            * that do not initialize core_pair.
            */
            intid = (uint32_t)MFIS_INTID_RR((uint32_t)ch->ch, (uint32_t)ch->type);
        }

        Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void *)ch);
        /* Enable interrupt from Receiver to Sender */
        Irq_SetPriority(intid, IPRIORITY(2U));
        Irq_Enable(intid);
    }

    if (ch->core_pair == MFIS_CORE_PAIR_CR_SCP)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

        mfis_set_bits(MFISIMR_RS_B(mfis_rtcore_num), MFISIMR_ACK_INT_BIT); // Enable ACK interrupt

        start = R_UTILS_GetTimerCounter();
        while (mfis_read_bit(MFISCHN_ACK_ST_RS_B(mfis_rtcore_num), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {
            if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
            {
                LogError(("%s: timeout waiting for SCP ACK to clear (RT core %u)\n", __func__, mfis_rtcore_num));
                ret = -1;
                return ret;
            }
        }
        mfis_set_bits(MFISIMR_RS_B(mfis_rtcore_num), MFISIMR_RCV_INT_BIT); // Enable receive interrupt
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            mfis_set_bits(MFISIMR_R_A(ch->ch), MFISIMR_ACK_INT_BIT); /* Step: A1 */
            mfis_set_bits(MFISIMR_R_A(ch->ch), MFISIMR_RCV_INT_BIT);

            start = R_UTILS_GetTimerCounter();
            while (mfis_read_bit(MFISCHN_ACK_ST_R_A(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
            {
                /* wait for pending ack to clear */
                if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
                {
                    LogError(("%s: timeout waiting for ACK to clear (RT core %u, CR-CR A side channel %u)\n", __func__, mfis_rtcore_num, ch->ch));
                    ret = -1;
                    return ret;
                }
            }
        }
        else
        {
            mfis_set_bits(MFISIMR_R_B(ch->ch), MFISIMR_RCV_INT_BIT);
            mfis_set_bits(MFISIMR_R_B(ch->ch), MFISIMR_ACK_INT_BIT);

            start = R_UTILS_GetTimerCounter();
            while (mfis_read_bit(MFISCHN_ACK_ST_R_B(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
            {
                /* wait for pending ack to clear */
                if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
                {
                    LogError(("%s: timeout waiting for ACK to clear (RT core %u, CR-CR B side channel %u)\n", __func__, mfis_rtcore_num, ch->ch));
                    ret = -1;
                    return ret;
                }
            }
        }
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
    {
        if ((MFISIMR_RA_B(ch->ch) == 0U) && (MFISCHN_ACK_ST_RA_B(ch->ch) == 0U))
        {
            ret = -1;
            return ret;
        }
        else
        {
            /* Unlock MFIS register write protection */
            mfis_write32(MFIS_RT_UNLOCK_WRITE_RA, MFIS_PROTECT_WRITE_DISABLE);

            mfis_set_bits(MFISIMR_RA_B(ch->ch), MFISIMR_RCV_INT_BIT);
            mfis_set_bits(MFISIMR_RA_B(ch->ch), MFISIMR_ACK_INT_BIT);

            start = R_UTILS_GetTimerCounter();
            while (mfis_read_bit(MFISCHN_ACK_ST_RA_B(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
            {
                /* wait for pending ack to clear */
                if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
                {
                    LogError(("%s: timeout waiting for ACK to clear (RT core %u, CA-CR channel %u)\n", __func__, mfis_rtcore_num, ch->ch));
                    ret = -1;
                    return ret;
                }
            }
        }
    }
    else
    {
        /*
        * CR-CR is the legacy/default mode.
        * This fallback preserves compatibility with applications
        * that do not initialize core_pair.
        */
        mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

        if (ch->type == MFIS_TYPE_RECEVER)
        {
            mfis_set_bits(MFISIMR_R_A(ch->ch), MFISIMR_ACK_INT_BIT);
            mfis_set_bits(MFISIMR_R_A(ch->ch), MFISIMR_RCV_INT_BIT);

            start = R_UTILS_GetTimerCounter();
            while (mfis_read_bit(MFISCHN_ACK_ST_R_A(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
            {
                /* wait for pending ack to clear */
                if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
                {
                    LogError(("%s: timeout waiting for ACK to clear (RT core %u, CR-CR A side channel %u)\n", __func__, mfis_rtcore_num, ch->ch));
                    ret = -1;
                    return ret;
                }
            }
        }
        else
        {
            mfis_set_bits(MFISIMR_R_B(ch->ch), MFISIMR_RCV_INT_BIT);
            mfis_set_bits(MFISIMR_R_B(ch->ch), MFISIMR_ACK_INT_BIT);

            start = R_UTILS_GetTimerCounter();
            while (mfis_read_bit(MFISCHN_ACK_ST_R_B(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
            {
                /* wait for pending ack to clear */
                if (((R_UTILS_GetTimerCounter() - start) * 1000U / timer_feq) >= timeout_ms)
                {
                    LogError(("%s: timeout waiting for ACK to clear (RT core %u, CR-CR B side channel %u)\n", __func__, mfis_rtcore_num, ch->ch));
                    ret = -1;
                    return ret;
                }
            }
        }
    }

    return ret;
}

int mfis_deinit(struct mfis_channel *ch)
{
    int ret = 0;
    uint32_t intid;
    uint32_t mfis_rtcore_num = CURRENT_CORE_IDX;

    if (!r_mfis_is_channel_id_valid(ch))
    {
        return -1;
    }

    if (ch->core_pair== MFIS_CORE_PAIR_CR_SCP)
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
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
    {
        intid = (uint32_t)MFIS_INTID_RR((uint32_t)ch->ch, (uint32_t)ch->type);
    }
    else if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
    {
        if (MFIS_INTID_RA((uint32_t)ch->ch) == 0U)
        {
            ret = -1;
            return ret;
        }
        else
        {
            intid = (uint32_t)MFIS_INTID_RA((uint32_t)ch->ch);
        }
    }
    else
    {
        /*
        * CR-CR is the legacy/default mode.
        * This fallback preserves compatibility with applications
        * that do not initialize core_pair.
        */
        intid = (uint32_t)MFIS_INTID_RR((uint32_t)ch->ch, (uint32_t)ch->type);
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

    if (!r_mfis_is_channel_id_valid(ch))
    {
        return -1;
    }

    if (ch->core_pair== MFIS_CORE_PAIR_CR_SCP)
    {
        /* Unlock MFIS register write protection */
        mfis_write32(MFIS_SCP_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);
        mfis_write32(MFISCHN_SIG_RS_B(mfis_rtcore_num), MFISCHN_SIG_SND_SIG_BIT);
    }
     else
    {
        if (!r_mfis_is_trigger_interrupt_valid(ch, int_number))
        {
            return -1;
        }

        if (ch->core_pair == MFIS_CORE_PAIR_CR_CR)
        {
            /* Unlock MFIS register write protection */
            mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

            if (ch->type == MFIS_TYPE_RECEVER)
            {
                mfis_write32(MFISCHN_SIG_R_A(ch->ch), MFISCHN_SIG_SND_SIG_BIT); 
            }
            else
            {
                mfis_write32(MFISCHN_SIG_R_B(ch->ch), MFISCHN_SIG_SND_SIG_BIT); 
            }
        }
        else if (ch->core_pair == MFIS_CORE_PAIR_CR_CA)
        {
            if ((MFISCHN_SIG_RA_B(ch->ch) == 0U) || (MFIS_INTID_RA((uint32_t)ch->ch) == 0U))
            {
                return -1;
            }
            else
            {
                /* Unlock MFIS register write protection */
                mfis_write32(MFIS_RT_UNLOCK_WRITE_RA, MFIS_PROTECT_WRITE_DISABLE);

                mfis_write32(MFISCHN_SIG_RA_B(ch->ch), MFISCHN_SIG_SND_SIG_BIT);
            }
        }
        else
        {
            /* Unlock MFIS register write protection */
            mfis_write32(MFIS_RT_UNLOCK_WRITE, MFIS_PROTECT_WRITE_DISABLE);

            if (ch->type == MFIS_TYPE_RECEVER)
            {
                mfis_write32(MFISCHN_SIG_R_A(ch->ch), MFISCHN_SIG_SND_SIG_BIT); /* Step: A3 */
            }
            else
            {
                mfis_write32(MFISCHN_SIG_R_B(ch->ch), MFISCHN_SIG_SND_SIG_BIT); /* Step: A3 */
            }
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