/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_I2C_REGS_H
#define R_I2C_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "board.h"

typedef enum {
    R_I2C_IF0 = 0,  /**< channel 0 */
    R_I2C_IF1,      /**< channel 1 */
    R_I2C_IF2,      /**< channel 2 */
    R_I2C_IF3,      /**< channel 3 */
    R_I2C_IF4,      /**< channel 4 */
    R_I2C_IF5,      /**< channel 5 */
    R_I2C_IF6,      /**< channel 6 */
    R_I2C_IF7,      /**< channel 7 */
    R_I2C_IF8,      /**< channel 8 */
    R_I2C_LAST      /**< delimiter */
} r_i2c_Unit_t;

#if (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2 || BOARD == X5H_VDK)
#define R_I2C_IF0_BASE    0xc11d0000
#define R_I2C_IF1_BASE    0xc06c0000
#define R_I2C_IF2_BASE    0xc06c8000
#define R_I2C_IF3_BASE    0xc06d0000
#define R_I2C_IF4_BASE    0xc06d8000
#define R_I2C_IF5_BASE    0xc06e0000
#define R_I2C_IF6_BASE    0xc06e8000
#define R_I2C_IF7_BASE    0xc06f0000
#define R_I2C_IF8_BASE    0xc06f8000
#else // (BOARD == MDP_AIACC_HIL || BOARD == MDP_AIACC_RFS2)
#define R_I2C_IF0_BASE    0x38040000
#define R_I2C_IF1_BASE    0x38048000
#define R_I2C_IF2_BASE    0x38050000
#define R_I2C_IF3_BASE    0x38058000
#endif

#define R_I2C_ICSCR       0x00UL
#define R_I2C_ICMCR       0x04UL
#define R_I2C_ICSSR       0x08UL
#define R_I2C_ICMSR       0x0cUL
#define R_I2C_ICSIER      0x10UL
#define R_I2C_ICMIER      0x14UL
#define R_I2C_ICCCR       0x18UL
#define R_I2C_ICSAR       0x1cUL
#define R_I2C_ICMAR       0x20UL
#define R_I2C_ICTXD       0x24UL
#define R_I2C_ICRXD       0x24UL
#define R_I2C_ICCCR2      0x28UL
#define R_I2C_ICMPR       0x2cUL
#define R_I2C_ICHPR       0x30UL
#define R_I2C_ICLPR       0x34UL
#define R_I2C_ICFBSCR     0x38UL
#define R_I2C_ICDMAER     0x3CUL

#define R_I2C_MNR_BIT   (1UL << 6)
#define R_I2C_MAL_BIT   (1UL << 5)
#define R_I2C_MST_BIT   (1UL << 4)
#define R_I2C_MDE_BIT   (1UL << 3)
#define R_I2C_MDT_BIT   (1UL << 2)
#define R_I2C_MDR_BIT   (1UL << 1)
#define R_I2C_MAT_BIT   (1UL << 0)

#define R_I2C_ESG_BIT   (1UL << 0)

#define R_I2C_GCAR      (1UL << 6)
#define R_I2C_STM       (1UL << 5)
#define R_I2C_SSR       (1UL << 4)
#define R_I2C_SDE       (1UL << 3)
#define R_I2C_SDT       (1UL << 2)
#define R_I2C_SDR       (1UL << 1)
#define R_I2C_SAR       (1UL << 0)

#define R_I2C_SDBS      (1UL << 3)
#define R_I2C_SIE       (1UL << 2)
#define R_I2C_GCAE      (1UL << 1)
#define R_I2C_FNA       (1UL << 0)

#define R_I2C_MDMACTSZ(x)    ((((x)-1)&0xFF) << 24)
#define R_I2C_RMDMATSZ(x)    ((((x)==256?0:(x))&0xFF) << 16)
#define R_I2C_TMDMATSZ(x)    ((((x)==256?0:(x))&0xFF) << 8)
#define R_I2C_TMDMACE        (1UL << 7)
#define R_I2C_RMDMACE        (1UL << 6)
#define R_I2C_RMDMAE         (1UL << 1)
#define R_I2C_TMDMAE         (1UL << 0)


#define R_I2C_FSDA_BIT (1UL << 5)

void        R_I2C_PRV_RegWrite32(uintptr_t Addr, uint32_t Data);
uint32_t    R_I2C_PRV_RegRead32(uintptr_t Addr);
uintptr_t   R_I2C_PRV_GetRegbase(r_i2c_Unit_t I2cUnit);

#ifdef __cplusplus
}
#endif

#endif /* R_I2C_REGS_H */
