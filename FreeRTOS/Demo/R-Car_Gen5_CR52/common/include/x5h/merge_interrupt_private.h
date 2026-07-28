/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

 /**
 * @defgroup MERGE_INTERRUPT_PRIVATE MERGE INTERRUPT PRIVATE
 * @{
 * @brief MERGE INTERRUPT of X5H platform.
 *
 */
#ifndef MERGE_INTERRUPT_PRIVATE_H
#define MERGE_INTERRUPT_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "r_merge_interrupt_common_private.h"

#define MERGED_REGISTER_ADDR                    (0x18A00000U)
#define RT_INTERRUPT_MASK_REGISTER_ADDR         (0x18A00000U)

/* DMA RT */
#define DMA_RT_MASK_OFFSET       (0x93C8U)
#define DMA_RT_STAT_OFFSET       (0x83C8U)
#define DMA_RT_STRIDE_X          (0x20U)
#define DMA_RT_STRIDE_Y          (0x4U)

/* SYS DMA */
#define SYS_DMA_MASK_OFFSET      (0x9468U)
#define SYS_DMA_STAT_OFFSET      (0x8468U)
#define SYS_DMA_STRIDE_X         (0x20U)
#define SYS_DMA_STRIDE_Y         (0x4U)

/* UCIE */
#define UCIE_STAT_OFFSET         (0x8770U)
#define UCIE_STAT_ERROR_OFFSET   (0x8778U)
#define UCIE_MASK_OFFSET         (0x9770U)
#define UCIE_MASK_ERROR_OFFSET   (0x9778U)
#define UCIE_STRIDE              (0x4U)
#define UCIE_ERROR_STRIDE        (0x4U)

/* VIN */
#define VIN_STAT_OFFSET          (0x854CU)
#define VIN_MASK_OFFSET          (0x954CU)
#define VIN_STRIDE               (0x4U)

/* UMF */
#define UMF_STAT_OFFSET          (0x874CU)
#define UMF_MASK_OFFSET          (0x974CU)
#define UMF_STRIDE               (0x4U)

/* DOC */
#define DOC_STAT_OFFSET          (0x8650U)
#define DOC_MASK_OFFSET          (0x9650U)
#define DOC_STRIDE               (0x4U)

static const irq_table merged_irq_table[] = {
    { 0x112, 0x3, STAT_DMA_RT(0,0), MASK_DMA_RT(0,0) },
    { 0x113, 0x3, STAT_DMA_RT(0,1), MASK_DMA_RT(0,1) },
    { 0x114, 0x3, STAT_DMA_RT(0,2), MASK_DMA_RT(0,2) },
    { 0x115, 0x3, STAT_DMA_RT(0,3), MASK_DMA_RT(0,3) },
    { 0x116, 0x3, STAT_DMA_RT(0,4), MASK_DMA_RT(0,4) },
    { 0x117, 0x3, STAT_DMA_RT(0,5), MASK_DMA_RT(0,5) },
    { 0x118, 0x3, STAT_DMA_RT(0,6), MASK_DMA_RT(0,6) },
    { 0x119, 0x3, STAT_DMA_RT(0,7), MASK_DMA_RT(0,7) },
    { 0x11A, 0x3, STAT_DMA_RT(1,0), MASK_DMA_RT(1,0) },
    { 0x11B, 0x3, STAT_DMA_RT(1,1), MASK_DMA_RT(1,1) },
    { 0x11C, 0x3, STAT_DMA_RT(1,2), MASK_DMA_RT(1,2) },
    { 0x11D, 0x3, STAT_DMA_RT(1,3), MASK_DMA_RT(1,3) },
    { 0x11E, 0x3, STAT_DMA_RT(1,4), MASK_DMA_RT(1,4) },
    { 0x11F, 0x3, STAT_DMA_RT(1,5), MASK_DMA_RT(1,5) },
    { 0x120, 0x3, STAT_DMA_RT(1,6), MASK_DMA_RT(1,6) },
    { 0x121, 0x3, STAT_DMA_RT(1,7), MASK_DMA_RT(1,7) },
    { 0x122, 0x3, STAT_DMA_RT(2,0), MASK_DMA_RT(2,0) },
    { 0x123, 0x3, STAT_DMA_RT(2,1), MASK_DMA_RT(2,1) },
    { 0x124, 0x3, STAT_DMA_RT(2,2), MASK_DMA_RT(2,2) },
    { 0x125, 0x3, STAT_DMA_RT(2,3), MASK_DMA_RT(2,3) },
    { 0x126, 0x3, STAT_DMA_RT(2,4), MASK_DMA_RT(2,4) },
    { 0x127, 0x3, STAT_DMA_RT(2,5), MASK_DMA_RT(2,5) },
    { 0x128, 0x3, STAT_DMA_RT(2,6), MASK_DMA_RT(2,6) },
    { 0x129, 0x3, STAT_DMA_RT(2,7), MASK_DMA_RT(2,7) },
    { 0x12A, 0x3, STAT_DMA_RT(3,0), MASK_DMA_RT(3,0) },
    { 0x12B, 0x3, STAT_DMA_RT(3,1), MASK_DMA_RT(3,1) },
    { 0x12C, 0x3, STAT_DMA_RT(3,2), MASK_DMA_RT(3,2) },
    { 0x12D, 0x3, STAT_DMA_RT(3,3), MASK_DMA_RT(3,3) },
    { 0x12E, 0x3, STAT_DMA_RT(3,4), MASK_DMA_RT(3,4) },
    { 0x12F, 0x3, STAT_DMA_RT(3,5), MASK_DMA_RT(3,5) },
    { 0x130, 0x3, STAT_DMA_RT(3,6), MASK_DMA_RT(3,6) },
    { 0x131, 0x3, STAT_DMA_RT(3,7), MASK_DMA_RT(3,7) },

    { 0x13A, 0x3, STAT_SYS_DMA(0,0), MASK_SYS_DMA(0,0) },
    { 0x13B, 0x3, STAT_SYS_DMA(0,1), MASK_SYS_DMA(0,1) },
    { 0x13C, 0x3, STAT_SYS_DMA(0,2), MASK_SYS_DMA(0,2) },
    { 0x13D, 0x3, STAT_SYS_DMA(0,3), MASK_SYS_DMA(0,3) },
    { 0x13E, 0x3, STAT_SYS_DMA(0,4), MASK_SYS_DMA(0,4) },
    { 0x13F, 0x3, STAT_SYS_DMA(0,5), MASK_SYS_DMA(0,5) },
    { 0x140, 0x3, STAT_SYS_DMA(0,6), MASK_SYS_DMA(0,6) },
    { 0x141, 0x3, STAT_SYS_DMA(0,7), MASK_SYS_DMA(0,7) },
    { 0x142, 0x3, STAT_SYS_DMA(1,0), MASK_SYS_DMA(1,0) },
    { 0x143, 0x3, STAT_SYS_DMA(1,1), MASK_SYS_DMA(1,1) },
    { 0x144, 0x3, STAT_SYS_DMA(1,2), MASK_SYS_DMA(1,2) },
    { 0x145, 0x3, STAT_SYS_DMA(1,3), MASK_SYS_DMA(1,3) },
    { 0x146, 0x3, STAT_SYS_DMA(1,4), MASK_SYS_DMA(1,4) },
    { 0x147, 0x3, STAT_SYS_DMA(1,5), MASK_SYS_DMA(1,5) },
    { 0x148, 0x3, STAT_SYS_DMA(1,6), MASK_SYS_DMA(1,6) },
    { 0x149, 0x3, STAT_SYS_DMA(1,7), MASK_SYS_DMA(1,7) },
    { 0x14A, 0x3, STAT_SYS_DMA(2,0), MASK_SYS_DMA(2,0) },
    { 0x14B, 0x3, STAT_SYS_DMA(2,1), MASK_SYS_DMA(2,1) },
    { 0x14C, 0x3, STAT_SYS_DMA(2,2), MASK_SYS_DMA(2,2) },
    { 0x14D, 0x3, STAT_SYS_DMA(2,3), MASK_SYS_DMA(2,3) },
    { 0x14E, 0x3, STAT_SYS_DMA(2,4), MASK_SYS_DMA(2,4) },
    { 0x14F, 0x3, STAT_SYS_DMA(2,5), MASK_SYS_DMA(2,5) },
    { 0x150, 0x3, STAT_SYS_DMA(2,6), MASK_SYS_DMA(2,6) },
    { 0x151, 0x3, STAT_SYS_DMA(2,7), MASK_SYS_DMA(2,7) },
    { 0x152, 0x3, STAT_SYS_DMA(3,0), MASK_SYS_DMA(3,0) },
    { 0x153, 0x3, STAT_SYS_DMA(3,1), MASK_SYS_DMA(3,1) },
    { 0x154, 0x3, STAT_SYS_DMA(3,2), MASK_SYS_DMA(3,2) },
    { 0x155, 0x3, STAT_SYS_DMA(3,3), MASK_SYS_DMA(3,3) },
    { 0x156, 0x3, STAT_SYS_DMA(3,4), MASK_SYS_DMA(3,4) },
    { 0x157, 0x3, STAT_SYS_DMA(3,5), MASK_SYS_DMA(3,5) },
    { 0x158, 0x3, STAT_SYS_DMA(3,6), MASK_SYS_DMA(3,6) },
    { 0x159, 0x3, STAT_SYS_DMA(3,7), MASK_SYS_DMA(3,7) },

    { 0x173, 0xFF, STAT_VIN(0), MASK_VIN(0) },
    { 0x174, 0xFF, STAT_VIN(1), MASK_VIN(1) },
    { 0x175, 0xFF, STAT_VIN(2), MASK_VIN(2) },
    { 0x176, 0xFF, STAT_VIN(3), MASK_VIN(3) },
    { 0x177, 0xFF, STAT_VIN(4), MASK_VIN(4) },
    { 0x178, 0xFF, STAT_VIN(5), MASK_VIN(5) },
    { 0x179, 0xFF, STAT_VIN(6), MASK_VIN(6) },
    { 0x17A, 0xFF, STAT_VIN(7), MASK_VIN(7) },
    { 0x17B, 0xFF, STAT_VIN(8), MASK_VIN(8) },
    { 0x17C, 0xFF, STAT_VIN(9), MASK_VIN(9) },
    { 0x17D, 0xFF, STAT_VIN(10), MASK_VIN(10) },
    { 0x17E, 0xFF, STAT_VIN(11), MASK_VIN(11) },

    { 0x1FC, 0x1F, STAT_UCIE(0), MASK_UCIE(0) },
    { 0x1FD, 0x1F, STAT_UCIE(1), MASK_UCIE(1) },
    { 0x1FE, 0x1FF, STAT_UCIE_ERROR(0), MASK_UCIE_ERROR(0) },
    { 0x1FF, 0x1FF, STAT_UCIE_ERROR(1), MASK_UCIE_ERROR(1) },

    { 0x1F3, 0x3, STAT_UMF(0), MASK_UMF(0) },
    { 0x1F4, 0x3, STAT_UMF(1), MASK_UMF(1) },
    { 0x1F5, 0x3, STAT_UMF(2), MASK_UMF(2) },
    { 0x1F6, 0x3, STAT_UMF(3), MASK_UMF(3) },

    { 0x1B4, 0x3, STAT_DOC(0), MASK_DOC(0) },
    { 0x1B5, 0x3, STAT_DOC(1), MASK_DOC(1) },
    { 0x1B6, 0x3, STAT_DOC(2), MASK_DOC(2) },
};

#ifdef __cplusplus
}
#endif

/** @} */ // end of MERGE_INTERRUPT_PRIVATE

#endif /* MERGE_INTERRUPT_PRIVATE_H */
 