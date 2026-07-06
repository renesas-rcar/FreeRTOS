/******************************************************************************
 * @file     mpu_armv8.h
 * @brief    CMSIS MPU API for Armv8-M and Armv8.1-M MPU
 * @version  V5.9.0
 * @date     11. April 2023
 ******************************************************************************/
/*
 * Copyright 2025 Renesas Electronics Corporation and/or its affiliates. All Rights Reserved.
 *
 * This file is based on the "CMSIS/Core/Include/mpu_armv8.h"
 *
 * Changes:
 * Renesas Electronics Corporation on 2025-01-17
 *    - Changed to be related to Cortex-R52
 */
/*
 * Copyright (c) 2017-2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARMV8R_ARM_MPU_H
#define ARMV8R_ARM_MPU_H
#include <stdint.h>
#include "cmsis_gcc.h"
/*----- CMSIS compiler specific defines -----*/
/* Put it to CMSIS */
/* ARM_MPU Region Base Address Register Definitions */
#define ARM_MPU_PRBAR_BASE_Pos                   6U                                            /*!< ARM_MPU PRBAR: BASE Position */
#define ARM_MPU_PRBAR_BASE_Msk                  (0x3FFFFFFUL << ARM_MPU_PRBAR_BASE_Pos)            /*!< ARM_MPU PRBAR: BASE Mask */

#define ARM_MPU_PRBAR_SH_Pos                     3U                                            /*!< ARM_MPU PRBAR: SH Position */
#define ARM_MPU_PRBAR_SH_Msk                    (0x3UL << ARM_MPU_PRBAR_SH_Pos)                    /*!< ARM_MPU PRBAR: SH Mask */

#define ARM_MPU_PRBAR_AP_Pos                     1U                                            /*!< ARM_MPU PRBAR: AP Position */
#define ARM_MPU_PRBAR_AP_Msk                    (0x3UL << ARM_MPU_PRBAR_AP_Pos)                    /*!< ARM_MPU PRBAR: AP Mask */

#define ARM_MPU_PRBAR_XN_Pos                     0U                                            /*!< ARM_MPU PRBAR: XN Position */
#define ARM_MPU_PRBAR_XN_Msk                    (01UL /*<< ARM_MPU_PRBAR_XN_Pos*/)                 /*!< ARM_MPU PRBAR: XN Mask */

/* ARM_MPU Region Limit Address Register Definitions */
#define ARM_MPU_PRLAR_LIMIT_Pos                  6U                                            /*!< ARM_MPU PRLAR: LIMIT Position */
#define ARM_MPU_PRLAR_LIMIT_Msk                 (0x3FFFFFFUL << ARM_MPU_PRLAR_LIMIT_Pos)           /*!< ARM_MPU PRLAR: LIMIT Mask */

#define ARM_MPU_PRLAR_AttrIndx_Pos               1U                                            /*!< ARM_MPU PRLAR: AttrIndx Position */
#define ARM_MPU_PRLAR_AttrIndx_Msk              (0x7UL << ARM_MPU_PRLAR_AttrIndx_Pos)              /*!< ARM_MPU PRLAR: AttrIndx Mask */

#define ARM_MPU_PRLAR_EN_Pos                     0U                                            /*!< ARM_MPU PRLAR: Region enable bit Position */
#define ARM_MPU_PRLAR_EN_Msk                    (1UL /*<< ARM_MPU_PRLAR_EN_Pos*/)                  /*!< ARM_MPU PRLAR: Region enable bit Disable Mask */

/* ARM_MPU Control bit fields */
#define ARM_MPU_ENABLE_Pos                     	 0U                                            /*!< ARM_MPU enable bit Position */
#define ARM_MPU_ENABLE_Msk                      (1UL /*<< ARM_MPU_ENABLE_Pos*/)                    /*!< ARM_MPU enable bit Enable Mask */

/* ARM_MPU Background Region bit fields */
#define ARM_MPU_BR_Pos                     	 	 17U                                            /*!< ARM_MPU Background Region enable bit Position */
#define ARM_MPU_BR_Msk                      	(1UL << ARM_MPU_BR_Pos)                             /*!< ARM_MPU Background Region enable bit Enable Mask */

/* ARM_MPU Cache bit fields */
#define ARM_MPU_DATA_CACHE_Pos					 2U											   /*!< ARM_MPU Data Cache bit Position */
#define ARM_MPU_DATA_CACHE_Msk					(1UL << ARM_MPU_DATA_CACHE_Pos)					   /*!< ARM_MPU Data Cache bit Enable Mask */

#define ARM_MPU_INS_CACHE_Pos					 12U										   /*!< ARM_MPU Instruction Cache bit Position */
#define ARM_MPU_INS_CACHE_Msk					(1UL << ARM_MPU_INS_CACHE_Pos)					   /*!< ARM_MPU Instruction Cache bit Enable Mask */

/*
 * Include common core functions to access Coprocessor 15 registers
 */
//#define __get_CP(cp, op1, Rt, CRn, CRm, op2) __asm volatile("MRC p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : "=r" (Rt) : : "memory")
//#define __set_CP(cp, op1, Rt, CRn, CRm, op2) __asm volatile("MCR p" # cp ", " # op1 ", %0, c" # CRn ", c" # CRm ", " # op2 : : "r" (Rt) : "memory")

/*-------------- End Put to CMSIS --------------*/

/** MAIRx Register Definitions */
/** \brief Attribute for device memory (outer only) */
#define ARM_MPU_ATTR_DEVICE                     (0U)

/** \brief Device memory type non Gathering, non Re-ordering, non Early Write Acknowledgement */
#define ARM_MPU_ATTR_DEVICE_nGnRnE 				(0U)

/** \brief Device memory type non Gathering, non Re-ordering, Early Write Acknowledgement */
#define ARM_MPU_ATTR_DEVICE_nGnRE  				(4U)

/** \brief Device memory type non Gathering, Re-ordering, Early Write Acknowledgement */
#define ARM_MPU_ATTR_DEVICE_nGRE   				(8U)

/** \brief Device memory type Gathering, Re-ordering, Early Write Acknowledgement */
#define ARM_MPU_ATTR_DEVICE_GRE    				(12U)

/** \brief Attribute for non-cacheable, normal memory (Outer and Inner) */
#define ARM_MPU_ATTR_NON_CACHEABLE              (4U)

/** \brief Attribute for Normal memory, Outer and Inner cacheability.
* \param NT Transient: Set to 1 for Non-transient data. Set to 0 for Transient data.
* \param WB Write-Back: Set to 1 to use a Write-Back policy. Set to 0 to use a Write-Through policy.
* \param RA Read Allocation: Set to 1 to enable cache allocation on read miss. Set to 0 to disable cache allocation on read miss.
* \param WA Write Allocation: Set to 1 to enable cache allocation on write miss. Set to 0 to disable cache allocation on write miss.
*/
#define ARM_MPU_ATTR_MEMORY_(NT, WB, RA, WA) \
  ((((NT) & 1U) << 3U) | (((WB) & 1U) << 2U) | (((RA) & 1U) << 1U) | ((WA) & 1U))

/** \brief Normal memory outer-cacheable and inner-cacheable attributes
* WT = Write Through, WB = Write Back, TR = Transient, RA = Read-Allocate, WA = Write Allocate
*/
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_TR_RA      (0b0010)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_TR_WA      (0b0001)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_TR_RA_WA   (0b0011)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT            (0b1000)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_RA         (0b1010)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_WA         (0b1001)
#define ARM_MPU_ATTR_NORMAL_OUTER_WT_RA_WA      (0b1011)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_TR_RA      (0b0110)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_TR_WA      (0b0101)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_TR_RA_WA   (0b0111)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB            (0b1100)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_RA         (0b1110)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_WA         (0b1101)
#define ARM_MPU_ATTR_NORMAL_OUTER_WB_RA_WA      (0b1111)
#define ARM_MPU_ATTR_NORMAL_OUTER_NON_CACHEABLE (0b0100)

#define ARM_MPU_ATTR_NORMAL_INNER_WT_TR_RA      (0b0010)
#define ARM_MPU_ATTR_NORMAL_INNER_WT_TR_WA      (0b0001)
#define ARM_MPU_ATTR_NORMAL_INNER_WT_TR_RA_WA   (0b0011)
#define ARM_MPU_ATTR_NORMAL_INNER_WT            (0b1000)
#define ARM_MPU_ATTR_NORMAL_INNER_WT_RA         (0b1010)
#define ARM_MPU_ATTR_NORMAL_INNER_WT_WA         (0b1001)
#define ARM_MPU_ATTR_NORMAL_INNER_WT_RA_WA      (0b1011)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_TR_RA      (0b0110)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_TR_WA      (0b0101)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_TR_RA_WA   (0b0111)
#define ARM_MPU_ATTR_NORMAL_INNER_WB            (0b1100)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_RA         (0b1110)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_WA         (0b1101)
#define ARM_MPU_ATTR_NORMAL_INNER_WB_RA_WA      (0b1111)
#define ARM_MPU_ATTR_NORMAL_INNER_NON_CACHEABLE (0b0100)

/** \brief Memory Attribute
* \param O Outer memory attributes
* \param I Inner memory attributes
*/
#define ARM_MPU_ATTR(O, I) ((((O) & 0xFU) << 4U) | ((I) & 0xFU))

/* \brief Specifies MAIR_ATTR number */
#define MAIR_ATTR(x)       (((x) > 7 || (x) < 0) ? 0 : (x))

/* MPU Region Base Address Register bit assignments */ 
/**
 * Shareability
 */
/** \brief Normal memory, non-shareable  */
#define ARM_MPU_SH_NON   (0U)

/** \brief Normal memory, outer shareable  */
#define ARM_MPU_SH_OUTER (2U)

/** \brief Normal memory, inner shareable  */
#define ARM_MPU_SH_INNER (3U)

/**
 * Access permissions
 * AP = Data access permissions, RO = Read-only, RW = Read/Write
 */
/** \brief Access from EL1 RW, Access from EL0 None */
#define ARM_MPU_AP_EL1_RW_EL0_None  (0U)

/** \brief Access from EL1 RW, Access from EL0 RW */
#define ARM_MPU_AP_EL1_RW_EL0_RW    (1U)

/** \brief Access from EL1 RO, Access from EL0 None */
#define ARM_MPU_AP_EL1_RO_EL0_None  (2U)

/** \brief Access from EL1 RO, Access from EL0 RO */
#define ARM_MPU_AP_EL1_RO_EL0_RO    (3U)

/*
 * Execute-never
 * XN = Execute-never, EX = Executable
 */
/** \brief Execution only permitted if read permitted */
#define ARM_MPU_XN (1U)

/** \brief Execution only permitted if read permitted */
#define ARM_MPU_EX (0U)

/** \brief Region Base Address Register value
* \param BASE The base address bits [31:6] of a memory region. The value is zero extended
* \param SH Defines the Shareability domain for this memory region.
* \param AP Data access permissions, RO = Read-only, RW = Read/Write
* \param XN eXecute Never: Set to 1 for a non-executable memory region. Set to 0 for an executable memory region.
*/
#define ARM_MPU_SET_PRBAR(BASE, SH, AP, XN) \
	(((BASE) & ARM_MPU_PRBAR_BASE_Msk) | \
	(((SH) << ARM_MPU_PRBAR_SH_Pos) & ARM_MPU_PRBAR_SH_Msk) | \
	(((AP) << ARM_MPU_PRBAR_AP_Pos) & ARM_MPU_PRBAR_AP_Msk) | \
	(((XN) << ARM_MPU_PRBAR_XN_Pos) & ARM_MPU_PRBAR_XN_Msk))

/** \brief Region Limit Address Register value
* \param LIMIT The limit address bits [31:6] for this memory region. The value is one extended.
* \param IDX The attribute index to be associated with this memory region.
*/
#define ARM_MPU_SET_PRLAR(LIMIT, IDX) \
	(((LIMIT) & ARM_MPU_PRLAR_LIMIT_Msk) | \
	((MAIR_ATTR(IDX) << ARM_MPU_PRLAR_AttrIndx_Pos) & ARM_MPU_PRLAR_AttrIndx_Msk) | \
	(ARM_MPU_PRLAR_EN_Msk))

/**
* Struct for a single ARM_MPU Region
*/
typedef struct {
	uint32_t prbar;                   /*!< Region Base Address Register value */
	uint32_t prlar;                   /*!< Region Limit Address Register value */
} ARM_MPU_Region_t;

/*-------------------------------------------------------------------------------------------------*/
/*									MPU Function Prototype										   */
/*-------------------------------------------------------------------------------------------------*/

/** \brief Enable MPU Block 
 * \param None 
*/
static inline void ARM_MPU_Enable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value |= (ARM_MPU_ENABLE_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Disable MPU Block 
 * \param None 
*/
static inline void ARM_MPU_Disable(void) {
	uint32_t SCTLR_Value;
	__get_CP(15, 0, SCTLR_Value, 1, 0, 0);
	SCTLR_Value &= ~(ARM_MPU_ENABLE_Msk);
	__set_CP(15, 0, SCTLR_Value, 1, 0, 0);
}

/** \brief Enable MPU Background Region
 * \param None 
*/
static inline void ARM_MPU_BackgroundRegionEnable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value |= (ARM_MPU_BR_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Disable MPU Background Region
 * \param None 
*/
static inline void ARM_MPU_BackgroundRegionDisable(void) {
	uint32_t SCTLR_Value;
	__get_CP(15, 0, SCTLR_Value, 1, 0, 0);
	SCTLR_Value &= ~(ARM_MPU_BR_Msk);
	__set_CP(15, 0, SCTLR_Value, 1, 0, 0);
}

/** \brief Enable Background Region for Instruction access
 * \param None
 */
static inline void ARM_MPU_InsBackgroundRegionEnable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value |= (uint32_t)(ARM_MPU_INS_CACHE_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Disable Background Region for Instruction access
 * \param None
 */
static inline void ARM_MPU_InsBackgroundRegionDisable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value &= ~(ARM_MPU_INS_CACHE_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Enable Background Region for Data access
 * \param None
 */
static inline void ARM_MPU_DataBackgroundRegionEnable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value |= (uint32_t)(ARM_MPU_DATA_CACHE_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Disable Background Region for Data access
 * \param None
 */
static inline void ARM_MPU_DataBackgroundRegionDisable(void) {
	uint32_t SCTLR_value;
	__get_CP(15, 0, SCTLR_value, 1, 0, 0);
	SCTLR_value &= ~(ARM_MPU_DATA_CACHE_Msk);
	__set_CP(15, 0, SCTLR_value, 1, 0, 0);
}

/** \brief Get number of programmable memory regions implemented by the EL1-controlled MPU.
 * \param None
 * \return Number of programmed regions
 */
static inline uint32_t ARM_MPU_Get_Number_Of_Regions(void) {
	uint32_t result;
	__get_CP(15, 0, result, 0, 0, 4);
	result = ((result << 16) >> 24);
	return result;
}

/** \brief	Set the memory attribute encoding to the given MPU MAIR0 register.
 * \param idx The attribute index to be set [0-3].
 * \param attr The attribute value to be set.
 */
static inline void ARM_MPU_SetMAIR0Attr(uint8_t idx, uint8_t attr) {
	const uint8_t pos = ((idx % 4U) * 8U);
	const uint32_t mask = 0xFFU << pos;
 
	uint32_t MAIRx_Value;
	__get_CP(15, 0, MAIRx_Value, 10, 2, 0);
	MAIRx_Value = (MAIRx_Value & (~mask)) | ((attr << pos) & mask);
	__set_CP(15, 0, MAIRx_Value, 10, 2, 0);
}

/** \brief	Set the memory attribute encoding to the given MPU MAIR1 register.
 * \param idx The attribute index to be set [0-3].
 * \param attr The attribute value to be set.
 */
static inline void ARM_MPU_SetMAIR1Attr(uint8_t idx, uint8_t attr) {
	const uint8_t pos = ((idx % 4U) * 8U);
	const uint32_t mask = 0xFFU << pos;
 
	uint32_t MAIRx_Value;
	__get_CP(15, 0, MAIRx_Value, 10, 2, 1);
	MAIRx_Value = (MAIRx_Value & (~mask)) | ((attr << pos) & mask);
	__set_CP(15, 0, MAIRx_Value, 10, 2, 1);
}

/** \brief Configure the given MPU region. And Enable this region
 * \param nr Region number to be configured [0 : (MAX_MPU_REGION_SUPPORTED - 1)].
 * \param region The structure containing the prbar and prlar register values ​​will be set.
 */
#define MAX_MPU_REGION_SUPPORTED ARM_MPU_Get_Number_Of_Regions()
static inline void ARM_MPU_SetRegion(uint8_t nr, ARM_MPU_Region_t region) {
	uint32_t PRBAR_Value = region.prbar;
	uint32_t PRLAR_Value = region.prlar;

	/* Write to PRSELR register */
	__set_CP(15, 0, nr, 6, 2, 1);

	/* Write to PRBAR register */
	__set_CP(15, 0, PRBAR_Value, 6, 3, 0);

	/* Write to PRLAR register */
	__set_CP(15, 0, PRLAR_Value, 6, 3, 1);
}

/** \brief Clear and disable the given MPU region.
 * \param nr Region number to be cleared.
 */
static inline void ARM_MPU_ClrRegion(uint8_t nr) {
	uint32_t PRBAR_Value = 0U;
	uint32_t PRLAR_Value = 0U;

	/* Write to PRSELR register */
	__set_CP(15, 0, nr, 6, 2, 1);

	/* Write to PRBAR register */
	__set_CP(15, 0, PRBAR_Value, 6, 3, 0);

	/* Write to PRLAR register */
	__set_CP(15, 0, PRLAR_Value, 6, 3, 1);
}

#endif /* ARMV8R_ARM_MPU_H */
