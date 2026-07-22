/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_SCMI_COMMON_H
#define RCAR_SCMI_COMMON_H

#include "cmsis_rcar_gen5.h"
//#define S2R_DRAFT_FLOW

#define SCMI_AGENT_ID_FRTOS_1ST	  2
#define SCMI_AGENT_ID_FRTOS_2ND   3
#define SCMI_AGENT_ID_AUTOSAR	  4
#define SCMI_AGENT_ID_CA_PSCI     8
#define SCMI_AGENT_ID_CA_OSPM_HV  9
#define SCMI_AGENT_ID_CA_OSPM_A   10
#define SCMI_AGENT_ID_CA_OSPM_B   11
#define SCMI_AGENT_ID_CA_OSPM_C   12
#define SCMI_AGENT_ID_CA_OSPM_D   13
#define SCMI_AGENT_ID_CA_OSPM_E   14

/* Describe R-Car X5H Specific transport using shared memory
 * and MFIS Mailbox 
 */
#define MAX_SHMEM_REGION			   2
/* Shared memory address */
#define SCMI_SHMEM_BASE_ADDR       (0xC1000000U)
#define SCMI_SHMEM_PLATFORM_MAIN   (SCMI_SHMEM_BASE_ADDR + (0x60200U))
#define SCMI_SHMEM_AGENT_MAIN      (SCMI_SHMEM_BASE_ADDR + (0x60300U))
#define SCMI_SHMEM_PLATFORM_2ND    (SCMI_SHMEM_BASE_ADDR + (0x60400U))
#define SCMI_SHMEM_AGENT_2ND       (SCMI_SHMEM_BASE_ADDR + (0x60500U))

/* Shared memory size */
#define SCMI_SHMEM_SIZE         (256U)

/* MFIS SCP Base Address */
#define MFIS_SCP_BASE           (0x18840000U)

/* MFIS SCP COMMON Base Address */
#define MFIS_SCP_COMMON_BASE    (0x189E1000U)


/* MFIS Write Protection Control Register */
#define MFIS_SCP_REG_MFISWPCNTR(base) (*((volatile uint32_t *)((base) + 0x900U)))

/*
 * MFIS CPU communication message register Realtime core[m](m=0-11)
 * to SCP core.
 */
#define MFIS_SCP_REG_MFISRSEMBR(base, m) \
    (*(volatile uint32_t *)(size_t)((base) + (0x1000U * (m)) + 0x44U + 0x20000U))

/*
 * MFIS CPU communication control register Realtime core[m]
 * to SCP Core(m=0-11).
 */
#define MFIS_SCP_REG_MFISRSEICR(base, m) \
    (*(volatile uint32_t *)(size_t)((base) + (0x1000U * (m)) + 0x04U + 0x20000U))

/* MFIS CPU communication control register SCP core
 * to Realtime core[m](m=0-11).
 */
#define MFIS_SCP_REG_MFISRSIICR(base, m) \
    (*(volatile uint32_t *)(size_t)((base) + (0x1000U * (m)) + 0x00U + 0x20000U))

/* MFIS Register access key */
#define MFIS_SCP_KEY_CODE           (0xACC00000U)

/* MFIS IRQ register source bits for interrupts generated (15-1bit used) */
#define MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION(val)    ((uint32_t)(0x00007FFFU & (val)) << 1)

/* MFIS IRQ register internal interrupt request bit (0bit used) */
#define MFIS_SCP_IRQ_REG_INT(n)         (0x00000001U & (n))

/* MFIS IRQ register mask bits (31-16bit unused) */
#define MFIS_SCP_IRQ_REG_MASK           (0x0000FFFFU)

#define CURRENT_CORE_MPIDR		(__get_MPIDR() & 0xF)
#define CURRENT_CLUSTER_MPIDR	((__get_MPIDR() & 0xF0) >> 8)
/* Realtime Core[m](m=0-11) for CR52 Agent */
#define CURRENT_CORE_IDX \
   (CURRENT_CLUSTER_MPIDR == 0 ? \
		CURRENT_CORE_MPIDR : \
		CURRENT_CORE_MPIDR + 4 * CURRENT_CLUSTER_MPIDR)

#endif /* RCAR_SCMI_COMMON_H */

