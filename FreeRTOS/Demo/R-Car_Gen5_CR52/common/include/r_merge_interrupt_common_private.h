/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup MERGE_INTERRUPT_COMMON_PRIVATE MERGE INTERRUPT COMMON PRIVATE
 * @{
 * @brief MERGE INTERRUPT COMMON PRIVATE.
 *
 */

#ifndef R_MERGE_INTERRUPT_COMMON_PRIVATE_H
#define R_MERGE_INTERRUPT_COMMON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MASK_DMA_RT(x, y) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + DMA_RT_MASK_OFFSET + \
     ((uint32_t)(x) * DMA_RT_STRIDE_X) + \
     ((uint32_t)(y) * DMA_RT_STRIDE_Y))

#define STAT_DMA_RT(x, y) \
    (MERGED_REGISTER_ADDR + DMA_RT_STAT_OFFSET + \
     ((uint32_t)(x) * DMA_RT_STRIDE_X) + \
     ((uint32_t)(y) * DMA_RT_STRIDE_Y))

#define MASK_SYS_DMA(x, y) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + SYS_DMA_MASK_OFFSET + \
     ((uint32_t)(x) * SYS_DMA_STRIDE_X) + \
     ((uint32_t)(y) * SYS_DMA_STRIDE_Y))

#define STAT_SYS_DMA(x, y) \
    (MERGED_REGISTER_ADDR + SYS_DMA_STAT_OFFSET + \
     ((uint32_t)(x) * SYS_DMA_STRIDE_X) + \
     ((uint32_t)(y) * SYS_DMA_STRIDE_Y))

#define STAT_UCIE(x) \
    (MERGED_REGISTER_ADDR + UCIE_STAT_OFFSET + \
     ((uint32_t)(x) * UCIE_STRIDE))

#define STAT_UCIE_ERROR(x) \
    (MERGED_REGISTER_ADDR + UCIE_STAT_ERROR_OFFSET + \
     ((uint32_t)(x) * UCIE_ERROR_STRIDE))

#define MASK_UCIE(x) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + UCIE_MASK_OFFSET + \
     ((uint32_t)(x) * UCIE_STRIDE))

#define MASK_UCIE_ERROR(x) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + UCIE_MASK_ERROR_OFFSET + \
     ((uint32_t)(x) * UCIE_ERROR_STRIDE))

#define STAT_VIN(x) \
    (MERGED_REGISTER_ADDR + VIN_STAT_OFFSET + \
     ((uint32_t)(x) * VIN_STRIDE))

#define MASK_VIN(x) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + VIN_MASK_OFFSET + \
     ((uint32_t)(x) * VIN_STRIDE))

#define STAT_UMF(x) \
    (MERGED_REGISTER_ADDR + UMF_STAT_OFFSET + \
     ((uint32_t)(x) * UMF_STRIDE))

#define MASK_UMF(x) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + UMF_MASK_OFFSET + \
     ((uint32_t)(x) * UMF_STRIDE))

#define STAT_DOC(x) \
    (MERGED_REGISTER_ADDR + DOC_STAT_OFFSET + \
     ((uint32_t)(x) * DOC_STRIDE))

#define MASK_DOC(x) \
    (RT_INTERRUPT_MASK_REGISTER_ADDR + DOC_MASK_OFFSET + \
     ((uint32_t)(x) * DOC_STRIDE))

typedef struct
{
    uint32_t irq;
    uint32_t mask_val;
    uint32_t status_reg;
    uint32_t mask_reg;
} irq_table;

#ifdef __cplusplus
}
#endif

#endif /* R_MERGE_INTERRUPT_COMMON_PRIVATE_H */
