/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_MFIS_PRIVATE_H
#define R_MFIS_PRIVATE_H

/**
 * @defgroup
 * @{
 * @brief .
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "r_mfis_common_private.h"

/* Logging Function include. */
#include "logging_stack.h"

//--------------- MFIS AIACC Registers ---------------
// ch[i]=[0-37]
#define MFIS_RT_BCH_BASE            (0x18A40000U)
#define MFIS_RT_BASE                (0x18800000U)

#define MFIS_RT_COMMON_BASE         (0x189E1000U)

/* MFIS SCP COMMON Base Address */
#define MFIS_SCP_COMMON_BASE        (0x189E3000U)

/* MFIS SCP Base Address */
#define MFIS_SCP_BASE           (0x18840000U)

#define MFIS_SCP_UNLOCK_WRITE   	(MFIS_SCP_COMMON_BASE + 0x900U)
#define MFIS_RT_UNLOCK_WRITE        (MFIS_RT_COMMON_BASE + 0x900U) /* MFISWPCNTR */

#define MFISIMR_R_A(i)             (MFIS_RT_BASE     + 0x808U + (0x1000U * (uint32_t)(i)))
#define MFISIMR_R_B(i)             (MFIS_RT_BCH_BASE + 0x808U + (0x1000U * (uint32_t)(i)))

#define MFISCHN_CTRL_R_A(i)        (MFIS_RT_BASE     + 0x81CU + (0x1000U * (uint32_t)(i)))
#define MFISCHN_CTRL_R_B(i)        (MFIS_RT_BCH_BASE + 0x81CU + (0x1000U * (uint32_t)(i)))

#define MFISCHN_ACK_ST_R_A(i)      (MFIS_RT_BASE     + 0x830U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_ACK_ST_R_B(i)      (MFIS_RT_BCH_BASE + 0x830U + (0x1000U * (uint32_t)(i)))

#define MFISISR_R_A(i)             (MFIS_RT_BASE     + 0x804U + (0x1000U * (uint32_t)(i)))
#define MFISISR_R_B(i)             (MFIS_RT_BCH_BASE + 0x804U + (0x1000U * (uint32_t)(i)))

#define MFISCHN_SIG_R_A(i)         (MFIS_RT_BASE     + 0x820U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_SIG_R_B(i)         (MFIS_RT_BCH_BASE + 0x820U + (0x1000U * (uint32_t)(i)))

#define MFISICR_R_A(i)             (MFIS_RT_BASE     + 0x80CU + (0x1000U * (uint32_t)(i)))
#define MFISICR_R_B(i)             (MFIS_RT_BCH_BASE + 0x80CU + (0x1000U * (uint32_t)(i)))

#define MFISCHN_RCV_ACK_R_A(i)     (MFIS_RT_BASE     + 0x840U + (0x1000U * (uint32_t)(i)))
#define MFISCHN_RCV_ACK_R_B(i)     (MFIS_RT_BCH_BASE + 0x840U + (0x1000U * (uint32_t)(i)))
//--------------- MFIS AIACC Registers ---------------

//--------------- MFIS AIACC INT ---------------
#define MFIS_INTID_RR(i, type) ((uint32_t)(0x0057U) + (((uint32_t)(i)) * 2U) - ((uint32_t)(type))) // Common INTID ch[i] from Sender to Receiver, unused
//--------------- MFIS AIACC INT ---------------

/* Set to NULL because these registers are not defined on this platform */
#define MFIS_RT_UNLOCK_WRITE_RA        (0U) /* MFISWPCNTR */

#define MFISIMR_RA_A(j)             (0U)
#define MFISIMR_RA_B(j)             (0U)

#define MFISCHN_CTRL_RA_A(j)        (0U)
#define MFISCHN_CTRL_RA_B(j)        (0U)

#define MFISCHN_ACK_ST_RA_A(j)      (0U)
#define MFISCHN_ACK_ST_RA_B(j)      (0U)

#define MFISISR_RA_A(j)             (0U)
#define MFISISR_RA_B(j)             (0U)

#define MFISCHN_SIG_RA_A(j)         (0U)
#define MFISCHN_SIG_RA_B(j)         (0U)

#define MFISICR_RA_A(j)             (0U)
#define MFISICR_RA_B(j)             (0U)

#define MFISCHN_RCV_ACK_RA_A(j)     (0U)
#define MFISCHN_RCV_ACK_RA_B(j)     (0U)

#define MFIS_INTID_RA(i)    (0U)

/* NOTE: original file defined MFISISR_RCV_INT twice with different
 * names (MFISISR_RCV_INT and MFISISR_RCV_INT_BIT). The duplicate
 * definition has been removed to avoid a redefinition and to keep a
 * single, unambiguous symbol (MISRA C:2012 Rule 20.5 / Dir 4.10 -
 * avoid duplicate/ambiguous macro definitions).
 */
#define MFISISR_RCV_INT                     MFISISR_RCV_INT_BIT
#define MFIS_SCP_BCH_BASE        (0x18A80000UL)
#define MFISIMR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x808U + 0x20000U + 0x1000U * (i))
#define MFISCHN_ACK_ST_RS_B(i)   (MFIS_SCP_BCH_BASE + 0x830U + 0x20000U + 0x1000U * (i))
#define MFISISR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x804U + 0x20000U + 0x1000U * (i))
#define MFISICR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x80CU + 0x20000U + 0x1000U * (i))
#define MFISCHN_SIG_RS_B(i)      (MFIS_SCP_BCH_BASE + 0x820U + 0x20000U + 0x1000U * (i))

#ifdef __cplusplus
}
#endif

/** @} */ // end

#endif /* R_MFIS_PRIVATE_H */