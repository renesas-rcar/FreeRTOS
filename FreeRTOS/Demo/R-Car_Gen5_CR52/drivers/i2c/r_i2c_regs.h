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

/* Offset I2C registers */
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
#define R_I2C_MST_BIT   (uint32_t)(1UL << 4)
#define R_I2C_MDE_BIT   (uint32_t)(1U << 3)
#define R_I2C_MDT_BIT   (1UL << 2)
#define R_I2C_MDR_BIT   (uint32_t)(1UL << 1)
#define R_I2C_MAT_BIT   (uint32_t)(1UL << 0)

#define R_I2C_ESG_BIT   (uint32_t)(1UL << 0)

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

#define R_I2C_ICCCR_100KHz              (0xAEU)
#define R_I2C_ICCCR_400KHz              (0x1EU)
#define R_I2C_ICCCR_1MHz                (0x06U)
#define R_I2C_ICCCR2_INITIAL_VAL        (0x00000000U)
#define R_I2C_ICCCR2_1MHz               (0X87U)
#define R_I2C_ICMPR_1MHz                (0x10U)
#define R_I2C_ICHPR_1MHz                (0x1DU)
#define R_I2C_ICLPR_1MHz                (0x28U)


#define R_I2C_FSDA_BIT (1UL << 5)

void        R_I2C_PRV_RegWrite32(uintptr_t Addr, uint32_t Data);
uint32_t    R_I2C_PRV_RegRead32(uintptr_t Addr);

#ifdef __cplusplus
}
#endif

#endif /* R_I2C_REGS_H */
