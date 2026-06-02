/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef arm_generic_timer_h
#define arm_generic_timer_h

#include "cmsis_rcar_gen5.h"
#define CNTFRQ_READ()         __get_CNTFRQ()
#define CNTFRQ_WRITE(v)       __set_CNTFRQ(v)
#define CNTPCT_READ()         __get_CNTPCT()
#define CNTP_CTL_WRITE(v)     __set_CNTP_CTL(v)
#define CNTP_CVAL_READ()      __get_CNTP_CVAL()
#define CNTP_CVAL_WRITE(v)    __set_CNTP_CVAL(v)

#if (BOARD == X5H_IRONHIDE || BOARD == MDP_AIACC_HIL)
#define GENERIC_TIMER_CLK     1066666666UL
#elif (BOARD == X5H_VDK || BOARD == X5H_RFS2 || BOARD == MDP_AIACC_RFS2)
#define GENERIC_TIMER_CLK     25000000UL
#else
/* The timer count up 16 counts with 66.667MHz clock.
It means the counter operate at 1066.667MHz in equivalent */
#define GENERIC_TIMER_CLK     1066666666    // Hz

#endif

#endif // arm_generic_timer_h
