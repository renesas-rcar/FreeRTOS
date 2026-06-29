/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef MPU_H
#define MPU_H

#include "armv8r_mpu.h"

/** \brief MPU area number. Cortex R52 supports up to 24 programmable regions. */
typedef enum {
    REGION_0,
    REGION_1,
    REGION_2,
    REGION_3,
    REGION_4,
    REGION_5,
    REGION_6,
    REGION_7,
    REGION_8,
    REGION_9,
    REGION_10,
    REGION_11,
    REGION_12,
    REGION_13,
    REGION_14,
    REGION_15,
    REGION_16,
    REGION_17,
    REGION_18,
    REGION_19,
    REGION_20,
    REGION_21,
    REGION_22,
    REGION_23,
} e_region_index_t;

/* Global MAIR configurations */
#define MPU_MAIR_INDEX_DEVICE			0U
#define MPU_MAIR_ATTR_DEVICE			(ARM_MPU_ATTR_DEVICE_nGnRnE)

#define MPU_MAIR_INDEX_FLASH			1U
#define MPU_MAIR_ATTR_FLASH				(ARM_MPU_ATTR(ARM_MPU_ATTR_NORMAL_OUTER_WT_RA, ARM_MPU_ATTR_NORMAL_INNER_WT_RA))

#define MPU_MAIR_INDEX_SRAM				2U
#define MPU_MAIR_ATTR_SRAM				(ARM_MPU_ATTR(ARM_MPU_ATTR_NORMAL_OUTER_WB_RA_WA, ARM_MPU_ATTR_NORMAL_INNER_WB_RA_WA))

#define MPU_MAIR_INDEX_SRAM_NOCACHE		3U
#define MPU_MAIR_ATTR_SRAM_NOCACHE		(ARM_MPU_ATTR(ARM_MPU_ATTR_NORMAL_OUTER_NON_CACHEABLE, ARM_MPU_ATTR_NORMAL_INNER_NON_CACHEABLE))

#define MPU_MAIR_DEFAULT_ATTRS						 \
	((MPU_MAIR_ATTR_DEVICE << ((MPU_MAIR_INDEX_DEVICE) * 8)) | \
	 (MPU_MAIR_ATTR_FLASH << ((MPU_MAIR_INDEX_FLASH) * 8)) |	 \
	 (MPU_MAIR_ATTR_SRAM << ((MPU_MAIR_INDEX_SRAM) * 8)) |	 \
	 (MPU_MAIR_ATTR_SRAM_NOCACHE << ((MPU_MAIR_INDEX_SRAM_NOCACHE) * 8)))

/*---------- Some helper defines for common regions. ----------*/

/** \brief Non shareable - EL1 RW EL0 None - Execute-never - Device nGnRnE 
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_DEVICE_ATTR(BASE, SIZE)						      	\
	(ARM_MPU_Region_t){							      			          		\
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_None, ARM_MPU_XN), \
		.prlar = ARM_MPU_SET_PRLAR((unsigned int)(BASE + SIZE - 1), MPU_MAIR_INDEX_DEVICE),		  \
	}

/** \brief Non shareable - EL1 RW EL0 None - Execute-never - Outer NT-WB-RA-WA, Inner NT-WB-RA-WA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_RAM_ATTR(BASE, SIZE)							\
	(ARM_MPU_Region_t){								\
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_None, ARM_MPU_XN), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM),			\
	}

/** \brief Non shareable - EL1 RW EL0 None - Execute-never - Outer Non Cacheable, Inner Non Cacheable
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_RAM_NOCACHE_ATTR(BASE, SIZE)				      \
	(ARM_MPU_Region_t){							      \
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_None, ARM_MPU_XN), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM_NOCACHE),	      \
	}

/** \brief Non shareable - EL1 RW EL0 None - Execute - Outer Non Cacheable, Inner Non Cacheable
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_TCM_ATTR(BASE, SIZE)				      \
	(ARM_MPU_Region_t){							      \
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_None, ARM_MPU_EX), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM_NOCACHE),	      \
	}

/** \brief Non shareable - EL1 RO EL0 RO - Execute - Outer NT-WB-RA-WA, Inner NT-WB-RA-WA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_RAM_TEXT_ATTR(BASE, SIZE)					\
	(ARM_MPU_Region_t){							\
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RO_EL0_RO, ARM_MPU_EX),	\
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM),		\
	}

/** \brief Non shareable - EL1 RO EL0 RO - Execute-never - Outer NT-WB-RA-WA, Inner NT-WB-RA-WA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_RAM_RO_ATTR(BASE, SIZE)						\
	(ARM_MPU_Region_t){								\
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RO_EL0_RO, ARM_MPU_XN), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM),			\
	}

/** \brief Non shareable - EL1 RW EL0 RW - Execute - Outer NT-WB-RA-WA, Inner NT-WB-RA-WA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_SRAM_ATTR(BASE, SIZE)                          \
    (ARM_MPU_Region_t){                                 \
        .prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_RW, ARM_MPU_EX), \
        .prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_SRAM),                \
    }

#ifdef CONFIG_ARM_MPU_ALLOW_FLASH_WRITE
/** \brief Non shareable - EL1 RW EL0 RW - Execute - Outer NT-WT-RA, Inner NT-WT-RA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_FLASH_ATTR(BASE, SIZE)						    \
	(ARM_MPU_Region_t){								    \
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RW_EL0_RW, ARM_MPU_EX), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_FLASH),			    \
	}
#else /* CONFIG_ARM_MPU_ALLOW_FLASH_WRITE */
/** \brief Non shareable - EL1 RO EL0 RO - Execute - Outer NT-WT-RA, Inner NT-WT-RA
 * \param BASE  Region Base address
 * \param LIMIT Region Size
*/
#define REGION_FLASH_ATTR(BASE, SIZE)						    \
	(ARM_MPU_Region_t){								    \
		.prbar = ARM_MPU_SET_PRBAR(BASE, ARM_MPU_SH_NON, ARM_MPU_AP_EL1_RO_EL0_RO, ARM_MPU_EX), \
		.prlar = ARM_MPU_SET_PRLAR(BASE + SIZE - 1, MPU_MAIR_INDEX_FLASH),			    \
	}
#endif /* CONFIG_ARM_MPU_ALLOW_FLASH_WRITE */
/*-------------------------------------------------------------------------------------------------*/

/** Enable MPU 
 */
void MPU_Enable(void);

/** Disable MPU Block 
*/
void MPU_Disable(void);

/** ARM MPU Driver Initial Setup
 *
 * Configure the cache-ability attributes for all the
 * different types of memory regions.
 * 
 * Device region(s): Attribute-0
 * Flash region(s): Attribute-1
 * SRAM region(s): Attribute-2
 * SRAM no cache-able regions(s): Attribute-3
 */
void MPU_Init(void);

/** @brief Configure the given MPU region. And Enable this region
 * 
 * @param[in] rnr - Region index.
 * @param[in] region_attr - The structure containing the prbar and prlar register values ​​will be set.
 *
 * @retval 0 if successful 
 * @retval 1 if region is exists
 * @retval 2 if region is out of max mpu region supported 
 */
uint8_t MPU_SetRegion_ByIndex(e_region_index_t rnr, ARM_MPU_Region_t region_attr);

/** @brief Automatically select indexes to configure the given MPU region. And Enable this region
 *
 * @param[in] region_attr - The structure containing the prbar and prlar register values <200b><200b>will be set.
 *
 * @retval 0 if successful
 * @retval other if set mpu region fail
 */
uint8_t MPU_SetRegion(ARM_MPU_Region_t region_attr);

/** @brief Clear and disable the given MPU region.
 * @param[in] rnr - Region index to be cleared.
 * @retval 0 if successful
 * @retval other if clear mpu region fail
 */
uint8_t MPU_ClrRegion_ByIndex(e_region_index_t rnr);

#endif /* MPU_H*/
