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

/* Describe R-Car AIACC Specific transport using shared memory
 * and MFIS Mailbox 
 */
#define MAX_SHMEM_REGION			   2
/* Shared memory address */
#define SCMI_SHMEM_BASE_ADDR     (0xC1000000U)
#define SCMI_SHMEM_PLATFORM_MAIN (SCMI_SHMEM_BASE_ADDR + (0x62000U))
#define SCMI_SHMEM_AGENT_MAIN    (SCMI_SHMEM_BASE_ADDR + (0x63000U))
#define SCMI_SHMEM_PLATFORM_2ND  (SCMI_SHMEM_BASE_ADDR + (0x64000U))
#define SCMI_SHMEM_AGENT_2ND     (SCMI_SHMEM_BASE_ADDR + (0x65000U))

/* Shared memory size */
#define SCMI_SHMEM_SIZE         (256U)

/* MFIS SCP Base Address */
#define MFIS_SCP_BASE         (0x18A80000U)

/* MFIS SCP COMMON Base Address */
#define MFIS_SCP_COMMON_BASE  (0x189E3000U)


#define SCP2RT0_IRQ_ID			 (0xD6)

#define MFIS_SCP_BCH_BASE        (0x18A80000)

#define MFIS_UNLOCK_WRITE   	 (MFIS_SCP_COMMON_BASE + 0x900) //MFISWPCNTR_SCP

#define MFISIMR_RS_A(i)          (MFIS_SCP_BASE + 0x808 + 0x20000 + 0x1000 * (i))
#define MFISIMR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x808 + 0x20000 + 0x1000 * (i))

#define MFISCHN_CTRL_RS_A(i)     (MFIS_SCP_BASE + 0x81C + 0x20000 + 0x1000 * (i))
#define MFISCHN_CTRL_RS_B(i)     (MFIS_SCP_BCH_BASE + 0x81C + 0x20000 + 0x1000 * (i))

#define MFISCHN_ACK_ST_RS_A(i)   (MFIS_SCP_BASE + 0x830 + 0x20000 + 0x1000 * (i))
#define MFISCHN_ACK_ST_RS_B(i)   (MFIS_SCP_BCH_BASE + 0x830 + 0x20000 + 0x1000 * (i))

#define MFISISR_RS_A(i)          (MFIS_SCP_BASE + 0x804 + 0x20000 + 0x1000 * (i))
#define MFISISR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x804 + 0x20000 + 0x1000 * (i))

#define MFISCHN_SIG_RS_A(i)      (MFIS_SCP_BASE + 0x820 + 0x20000 + 0x1000 * (i))
#define MFISCHN_SIG_RS_B(i)      (MFIS_SCP_BCH_BASE + 0x820 + 0x20000 + 0x1000 * (i))

#define MFISICR_RS_A(i)          (MFIS_SCP_BASE + 0x80C + 0x20000 + 0x1000 * (i))
#define MFISICR_RS_B(i)          (MFIS_SCP_BCH_BASE + 0x80C + 0x20000 + 0x1000 * (i))

#define MFISCHN_RCV_ACK_RS_A(i)  (MFIS_SCP_BASE + 0x840 + 0x20000 + 0x1000 * (i))
#define MFISCHN_RCV_ACK_RS_B(i)  (MFIS_SCP_BCH_BASE + 0x840 + 0x20000 + 0x1000 * (i))

//////////////////////////////////////
#define BIT_MASK(n)               	 (1U << (n))

#define MFISIMR_ACK_INT_BIT          BIT_MASK(0)
#define MFISIMR_RCV_INT_BIT          BIT_MASK(1)

#define MFISISR_ACK_INT              BIT_MASK(0)
#define MFISISR_RCV_INT              BIT_MASK(1)

#define MFISCHN_ACK_ST_PND_ACK_BIT   BIT_MASK(0)
#define MFISCHN_SIG_SND_SIG_BIT      BIT_MASK(0)

#define MFISICR_ACK_INT_BIT          BIT_MASK(0)
#define MFISICR_RCV_INT_BIT          BIT_MASK(1)

#define MFISCHN_CTRL_EN_AUTO_ACK_INTCLR  BIT_MASK(0)
#define MFISISR_RCV_INT_BIT          BIT_MASK(1)
#define MFISCHN_RCV_ACK_RCV_ACK_BIT  BIT_MASK(0)

/* MFIS IRQ register source bits for interrupts generated (15-1bit used) */
#define MFIS_SCP_DISABLE_MFIS_WRITE_PROTECTION(val)    ((uint32_t)(0x00007FFFU & (val)) << 1)

/* MFIS IRQ register internal interrupt request bit (0bit used) */
#define MFIS_SCP_IRQ_REG_INT(n)         (0x00000001U & (n))

#define CURRENT_CORE_MPIDR		(__get_MPIDR() & 0xF)
#define CURRENT_CLUSTER_MPIDR	((__get_MPIDR() & 0xF0) >> 8)
/* Realtime Core[m](m=0-11) for CR52 Agent */
#define CURRENT_CORE_IDX \
   (CURRENT_CLUSTER_MPIDR == 0 ? \
		CURRENT_CORE_MPIDR : \
		CURRENT_CORE_MPIDR + 4 * CURRENT_CLUSTER_MPIDR)

#endif /* RCAR_SCMI_COMMON_H */

