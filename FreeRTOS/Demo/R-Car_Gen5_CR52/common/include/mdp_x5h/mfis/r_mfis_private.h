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

/* MFIS IRQ register mask bits (31-16bit unused) */
#define MFIS_SCP_IRQ_REG_MASK           (0x0000FFFFU)

/* MFIS CPU communication control register SCP core
 * to Realtime core[m](m=0-11).
 */
#define MFIS_SCP_REG_MFISRSIICR(base, m) \
    (*(volatile uint32_t *)(size_t)((base) + ((size_t)0x1000U * (size_t)(m)) + (size_t)0x00U + (size_t)0x20000U))

/* MFIS SCP Base Address */
#define MFIS_SCP_BASE           (0x18840000U)

/* MFIS IRQ register source bits for interrupts generated (15-1bit used) */
#define MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION(val)    ((uint32_t)(0x00007FFFU & (uint32_t)(val)) << 1U)

/* MFIS IRQ register internal interrupt request bit (0bit used) */
#define MFIS_SCP_IRQ_REG_INT(n)         (0x00000001U & (n))

/*
 * MFIS CPU communication message register Realtime core[m](m=0-11)
 * to SCP core.
 */
#define MFIS_SCP_REG_MFISRSEMBR(base, m) \
    (*(volatile uint32_t *)((size_t)(base) + ((size_t)0x1000U * (size_t)(m)) + (size_t)0x44U + (size_t)0x20000U))

/*
 * MFIS CPU communication control register Realtime core[m]
 * to SCP Core(m=0-11).
 */
#define MFIS_SCP_REG_MFISRSEICR(base, m) \
    (*(volatile uint32_t *)((size_t)(base) + ((size_t)0x1000U * (size_t)(m)) + (size_t)0x04U + (size_t)0x20000U))

#define MFIS_SCP_COMMON_X5H_BASE    0x189E1000U
#define MFIS_SCP_X5H_UNLOCK_WRITE   (MFIS_SCP_COMMON_X5H_BASE + 0x900U) // MFISWPCNTR_SCP
#define MFIS_UNLOCK_WRITE   (0x189E0900U)

#ifdef __cplusplus
}
#endif

/** @} */ // end

#endif /* R_MFIS_PRIVATE_H */