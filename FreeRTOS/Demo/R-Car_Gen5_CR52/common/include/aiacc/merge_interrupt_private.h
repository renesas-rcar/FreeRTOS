/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

 /**
 * @defgroup MERGE_INTERRUPT_PRIVATE MERGE INTERRUPT PRIVATE
 * @{
 * @brief MERGE INTERRUPT of AIACC platform.
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

/* UCIE */
#define UCIE_STAT_OFFSET         (0x8780U)
#define UCIE_STAT_ERROR_OFFSET   (0x87D0U)
#define UCIE_MASK_OFFSET         (0x9780U)
#define UCIE_MASK_ERROR_OFFSET   (0x97D0U)
#define UCIE_STRIDE              (0x28U)
#define UCIE_ERROR_STRIDE        (0x4U)

/* VIN */
#define VIN_STAT_OFFSET          (0x854CU)
#define VIN_MASK_OFFSET          (0x954CU)
#define VIN_STRIDE               (0x4U)

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
};

#ifdef __cplusplus
}
#endif

/** @} */ // end of MERGE_INTERRUPT_PRIVATE

#endif /* MERGE_INTERRUPT_PRIVATE_H */
 