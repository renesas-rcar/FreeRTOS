/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RCAR_SCMI_COMMON_H
#define RCAR_SCMI_COMMON_H

#include "cmsis_rcar_gen5.h"

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

#define CURRENT_CORE_MPIDR		(__get_MPIDR() & 0xF)
#define CURRENT_CLUSTER_MPIDR	((__get_MPIDR() & 0xF0) >> 8)
/* Realtime Core[m](m=0-11) for CR52 Agent */
#define CURRENT_CORE_IDX \
   (CURRENT_CLUSTER_MPIDR == 0 ? \
		CURRENT_CORE_MPIDR : \
		CURRENT_CORE_MPIDR + 4 * CURRENT_CLUSTER_MPIDR)

#endif /* RCAR_SCMI_COMMON_H */

