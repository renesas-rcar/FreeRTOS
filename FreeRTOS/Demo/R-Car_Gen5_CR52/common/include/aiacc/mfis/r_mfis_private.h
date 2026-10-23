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

/* MFIS SCP COMMON Base Address */
#define MFIS_SCP_COMMON_BASE        (0x189E3000U)
#define MFIS_SCP_UNLOCK_WRITE   	(MFIS_SCP_COMMON_BASE + 0x900U)

#define SCP2RT0_IRQ_ID              (0xD6U)
#define SCP2RT1_IRQ_ID              (0xD7U)

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