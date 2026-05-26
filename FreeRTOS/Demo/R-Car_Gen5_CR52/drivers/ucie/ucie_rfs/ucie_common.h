/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_COMMON_H_
#define R_UCIE_COMMON_H_

#define UCIE_APB_BASE(n)    (0xDC000000U + (n) * 0x1000000)

#define UCIE_UPPER_SRC_ADDR_OFF (0x50)
#define UCIE_LOWER_SRC_ADDR_OFF (0x54)
#define UCIE_UPPER_DST_ADDR_OFF (0x58)
#define UCIE_LOWER_DST_ADDR_OFF (0x5C)
#define UCIE_MAPPING_SIZE_OFF   (0x60)
#define UCIE_MAPPING_EN_OFF     (0x64)
#define UCIE_CLR_INT_OFF        (0x70)
#define UCIE_INT_STS_OFF        (0x74)
#define UCIE_RQ_SYNC_TO_OFF     (0x78)
#define UCIE_RQ_SYNC_FROM_OFF   (0x7C)
#define UCIE_MAPPING_CLR_OFF    (0x80)

#define UCIE_MAPPING_EN_BIT     (1)
#define UCIE_MAPPING_STATUS_BIT (2)
#define UCIE_MAPPING_INT_BIT    (14)

#define UCIE_ADDR_MASK          (0xFFF)

#define IATU_RGN_OFFSET         (0x80000)

#endif /* R_UCIE_COMMON_H_ */
