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
#define X5H_SCMI_SHMEM_BASE_ADDR       (0xC1000000U)
#define X5H_SCMI_SHMEM_PLATFORM_MAIN   (X5H_SCMI_SHMEM_BASE_ADDR + (0x60200U))
#define X5H_SCMI_SHMEM_AGENT_MAIN      (X5H_SCMI_SHMEM_BASE_ADDR + (0x60300U))
#define X5H_SCMI_SHMEM_PLATFORM_2ND    (X5H_SCMI_SHMEM_BASE_ADDR + (0x60400U))
#define X5H_SCMI_SHMEM_AGENT_2ND       (X5H_SCMI_SHMEM_BASE_ADDR + (0x60500U))

/* Shared memory size */
#define X5H_SCMI_SHMEM_SIZE         (256U)

/* MFIS SCP Base Address */
#define X5H_MFIS_SCP_BASE           (0x18840000U)
/* MFIS SCP COMMON Base Address */
#define X5H_MFIS_SCP_COMMON_BASE    (0x189E1000U)

/* MFIS Register access key */
#define X5H_MFIS_SCP_KEY_CODE           (0xACC00000U)

/* MFIS Register disable write protect */
#define X5H_MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION \
    (X5H_MFIS_SCP_KEY_CODE | 0x00000001U)

/* MFIS IRQ register source bits for interrupts generated (15-1bit used) */
#define X5H_MFIS_SCP_IRQ_REG_SOURCE(val)    ((uint32_t)(0x00007FFFU & (val)) << 1)

/* MFIS IRQ register internal interrupt request bit (0bit used) */
#define X5H_MFIS_SCP_IRQ_REG_INT(n)         (0x00000001U & (n))

/* MFIS IRQ register mask bits (31-16bit unused) */
#define X5H_MFIS_SCP_IRQ_REG_MASK           (0x0000FFFFU)

#define CURRENT_CORE_MPIDR		(__get_MPIDR() & 0xF)
#define CURRENT_CLUSTER_MPIDR	((__get_MPIDR() & 0xF0) >> 8)
/* Realtime Core[m](m=0-11) for CR52 Agent */
#define CURRENT_CORE_IDX \
   (CURRENT_CLUSTER_MPIDR == 0 ? \
		CURRENT_CORE_MPIDR : \
		CURRENT_CORE_MPIDR + 4 * CURRENT_CLUSTER_MPIDR)

#endif /* RCAR_SCMI_COMMON_H */

