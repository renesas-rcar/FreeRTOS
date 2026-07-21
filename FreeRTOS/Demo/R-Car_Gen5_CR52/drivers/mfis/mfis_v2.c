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


#define MFIS_RT_BCH_BASE            (0x18A40000U)
#define MFIS_RT_BASE                (0x18800000U)

#define MFIS_RT_COMMON_BASE         (0x189E1000U)
#define MFIS_RT_UNLOCK_WRITE        (MFIS_RT_COMMON_BASE + 0x900U) /* MFISWPCNTR */

#define MFISIMR_RR_A(i)             (MFIS_RT_BASE     + 0x808U + (0x1000U * (uint32_t)(i)))
#define MFISIMR_RR_B(i)             (MFIS_RT_BCH_BASE + 0x808U + (0x1000U * (uint32_t)(i)))

#define MFISCHN_CTRL_RR_A(i)        (MFIS_RT_BASE     + 0x81CU + (0x1000U * (uint32_t)(i)))
#define MFISCHN_CTRL_RR_B(i)        (MFIS_RT_BCH_BASE + 0x81CU + (0x1000U * (uint32_t)(i)))

#define MFISCHN_ACK_ST_RR_A(i)      (MFIS_RT_BASE     + 0x830U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_ACK_ST_RR_B(i)      (MFIS_RT_BCH_BASE + 0x830U + (0x1000U * (uint32_t)(i)))

#define MFISISR_RR_A(i)             (MFIS_RT_BASE     + 0x804U + (0x1000U * (uint32_t)(i)))
#define MFISISR_RR_B(i)             (MFIS_RT_BCH_BASE + 0x804U + (0x1000U * (uint32_t)(i)))

#define MFISCHN_SIG_RR_A(i)         (MFIS_RT_BASE     + 0x820U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_SIG_RR_B(i)         (MFIS_RT_BCH_BASE + 0x820U + (0x1000U * (uint32_t)(i)))

#define MFISICR_RR_A(i)             (MFIS_RT_BASE     + 0x80CU + (0x1000U * (uint32_t)(i)))
#define MFISICR_RR_B(i)             (MFIS_RT_BCH_BASE + 0x80CU + (0x1000U * (uint32_t)(i)))

#define MFISCHN_RCV_ACK_RR_A(i)     (MFIS_RT_BASE     + 0x840U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_RCV_ACK_RR_B(i)     (MFIS_RT_BCH_BASE + 0x840U + (0x1000U * (uint32_t)(i)))

/*----------------------------------------------------------------------*/

#define BIT(n)                              (1UL << (n))

#define MFISIMR_ACK_INT_BIT                 BIT(0U) /* bit 0 */
#define MFISIMR_RCV_INT_BIT                 BIT(1U) /* bit 1 */

#define MFISISR_ACK_INT                     BIT(0U)
#define MFISISR_RCV_INT_BIT                 BIT(1U)

#define MFISCHN_ACK_ST_PND_ACK_BIT          BIT(0U) /* bit 0 */
#define MFISCHN_SIG_SND_SIG_BIT             BIT(0U) /* bit 0 */

#define MFISICR_ACK_INT_BIT                 BIT(0U) /* bit 0 */
#define MFISICR_RCV_INT_BIT                 BIT(1U) /* bit 1 */

#define MFISCHN_CTRL_EN_AUTO_ACK_INTCLR     BIT(0U) /* bit 0 */
#define MFISCHN_RCV_ACK_RCV_ACK_BIT         BIT(0U) /* bit 0 */

/* NOTE: original file defined MFISISR_RCV_INT twice with different
 * names (MFISISR_RCV_INT and MFISISR_RCV_INT_BIT). The duplicate
 * definition has been removed to avoid a redefinition and to keep a
 * single, unambiguous symbol (MISRA C:2012 Rule 20.5 / Dir 4.10 -
 * avoid duplicate/ambiguous macro definitions).
 */
#define MFISISR_RCV_INT                     MFISISR_RCV_INT_BIT

/*--------------------------- MFIS Driver ---------------------------------*/

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
    /* ch is currently unused; kept in the prototype for API
     * consistency and future use. Cast to void to satisfy
     * MISRA C:2012 Rule 2.7 (unused parameter). */
    (void)ch;

    uint16_t result = 1U;
    return result;
}

/* Get message of a channel */
uint16_t mfis_get_message(struct mfis_channel *ch)
{
    /* ch is currently unused; kept in the prototype for API
     * consistency and future use. Cast to void to satisfy
     * MISRA C:2012 Rule 2.7 (unused parameter). */
    (void)ch;

    /* NOTE: the original function had no return statement, which is
     * undefined behaviour for a non-void function (MISRA C:2012
     * Rule 17.4). No message-data register was defined in this file,
     * so a safe default of 0 is returned here. Replace this with a
     * read of the actual MFIS message register once its address is
     * defined. */
    uint16_t result = 0U;
    return result;
}


/* Forward declaration required so that a compatible declaration is
 * visible before this externally-linked function is defined
 * (MISRA C:2012 Rule 8.4). Ideally this belongs in mfis_internal.h
 * alongside the other driver prototypes; it is declared here as a
 * local fallback in case the header does not already expose it. */
void mfis_interrupt_cb(void *data);

/* Interrupt callback sample */
void mfis_interrupt_cb(void *data)
{
    struct mfis_channel *ch = (struct mfis_channel *)data;

    /* Unlock MFIS register write protection */
    mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);

    if (ch->type == MFIS_TYPE_RECEVER)
    {
        if (mfis_read_bit(MFISISR_RR_A(ch->ch), MFISISR_ACK_INT) != 0U)
        {
            /* Step A5: Write 1 to MFISICR_A0.ACK_INT */
            mfis_set_bits(MFISICR_RR_A(ch->ch), MFISICR_ACK_INT_BIT);
        }

        if (mfis_read_bit(MFISISR_RR_A(ch->ch), MFISISR_RCV_INT) != 0U)
        {
            /* Step B4, B5 */
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
    else
    {
        if (mfis_read_bit(MFISISR_RR_B(ch->ch), MFISISR_RCV_INT) != 0U)
        {
            /* Step B4, B5 */
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
            /* Step A5: Write 1 to MFISICR_A0.ACK_INT */
            mfis_set_bits(MFISICR_RR_B(ch->ch), MFISICR_ACK_INT_BIT);
        }
    }
}

/* Initialize MFIS */
int mfis_init(struct mfis_channel *ch)
{
    /* Initialize */
    ch->int_source = 0U;
    ch->recv_message = 0U;

    uint32_t intid = (uint32_t)MFIS_INTID((uint32_t)ch->ch, (uint32_t)ch->type);

    /* Set callback function */
    Irq_SetupEntry(intid, (IrqHandlerFn)mfis_interrupt_cb, (void *)ch);
    /* Enable interrupt from Receiver to Sender */
    Irq_SetPriority(intid, IPRIORITY(2U));
    Irq_Enable(intid);

    /* Unlock MFIS register write protection */
    mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);

    if (ch->type == MFIS_TYPE_RECEVER)
    {
        mfis_set_bits(MFISIMR_RR_A(ch->ch), MFISIMR_ACK_INT_BIT); /* Step: A1 */
        mfis_set_bits(MFISIMR_RR_A(ch->ch), MFISIMR_RCV_INT_BIT);

        while (mfis_read_bit(MFISCHN_ACK_ST_RR_A(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {
            /* wait for pending ack to clear */
        }
    }
    else
    {
        mfis_set_bits(MFISIMR_RR_B(ch->ch), MFISIMR_RCV_INT_BIT); /* Step: A1 */
        mfis_set_bits(MFISIMR_RR_B(ch->ch), MFISIMR_ACK_INT_BIT);

        while (mfis_read_bit(MFISCHN_ACK_ST_RR_B(ch->ch), MFISCHN_ACK_ST_PND_ACK_BIT) != 0U)
        {
            /* wait for pending ack to clear */
        }
    }

    return 0;
}

int mfis_deinit(struct mfis_channel *ch)
{
    uint32_t intid = (uint32_t)MFIS_INTID((uint32_t)ch->ch, (uint32_t)ch->type);

    Irq_Disable(intid);
    Irq_RemoveEntry(intid);
    return 0;
}

/* Trigger interrupt to Receiver channel ch
 * int_number is 15-bit integer for interrupt source
 */
int mfis_trigger_interrupt(struct mfis_channel *ch, uint16_t int_number)
{
    if ((ch->ch > 63U) || (int_number > 0x7FFFU))
    {
        return -1;
    }

    /* Unlock MFIS register write protection */
    mfis_write32(MFIS_RT_UNLOCK_WRITE, 0xACC00001U);

    if (ch->type == MFIS_TYPE_RECEVER)
    {
        mfis_write32(MFISCHN_SIG_RR_A(ch->ch), MFISCHN_SIG_SND_SIG_BIT); /* Step: A3 */
    }
    else
    {
        mfis_write32(MFISCHN_SIG_RR_B(ch->ch), MFISCHN_SIG_SND_SIG_BIT); /* Step: A3 */
    }

    return 0;
}

/* Send a 32-bit int to Receiver via message register */
int mfis_send_message(struct mfis_channel *ch, uint32_t value)
{
    /* ch and value are currently unused; kept in the prototype for
     * API consistency and future use. Cast to void to satisfy
     * MISRA C:2012 Rule 2.7 (unused parameter). */
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