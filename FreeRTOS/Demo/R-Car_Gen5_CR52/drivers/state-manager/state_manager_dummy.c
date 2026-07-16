/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "scmi/inc/rcar_scmi_common.h"
#include "scmi/inc/common.h"
#include "scmi/inc/base.h"
#include "scmi/inc/power.h"
#include "scmi/inc/clock.h"
#include "scmi/inc/reset.h"
#include "scmi/inc/system.h"
#include "scmi/inc/vendor-rcar.h"
#include "state-manager/r_state_manager.h"
#include "state-manager/r_power_domain_id.h"
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"

#include "FreeRTOS.h"

static uint32_t max_clockdomain_num;
static uint32_t max_powerdomain_num;
static uint32_t max_resetdomain_num;
static bool initialized = false;

static const char* agentid2str(int agent_id)
{
	switch (agent_id) {
		case SCMI_AGENT_ID_FRTOS_1ST:
			return "Main FreeRTOS (CR)";

		case SCMI_AGENT_ID_FRTOS_2ND:
			return "Secondary FreeRTOS (CR)";

		case SCMI_AGENT_ID_AUTOSAR:
			return "Classic Autosar (CR)";

        case SCMI_AGENT_ID_CA_PSCI:
            return "PSCI (BL31)";

        case SCMI_AGENT_ID_CA_OSPM_HV:
            return "Hypervisor";

        case SCMI_AGENT_ID_CA_OSPM_A:
            return "OSPM_A";

        case SCMI_AGENT_ID_CA_OSPM_B:
            return "OSPM_B";

        case SCMI_AGENT_ID_CA_OSPM_C:
            return "OSPM_C";

        case SCMI_AGENT_ID_CA_OSPM_D:
            return "OSPM_D";

        case SCMI_AGENT_ID_CA_OSPM_E:
            return "OSPM_E";

		default:
			break;
	}

	return "Unknown Agent";
}

static void system_notification(void *data)
{
    (void)data;
}

int R_StateManager_Init(void)
{
    return 0;
}

int R_StateManager_SCMI_Info_Show(void)
{
    return 0;
}

int R_StateManager_RequestDeepStop(void)
{
    return 0;
}

int R_StateManager_SysReboot(void)
{
    return 0;
}

int R_StateManager_SysPowerOff(void)
{
    return 0;
}

int R_StateManager_Power_Get(int domain_id, e_power_state_t *state)
{
    /*
     * Due to no SCP support for RFS2 environment,
     * also all modules are ON by default, the return
     * value is set to POWER_ON.
     */
    (void)domain_id;
    (void)state;
    *state = POWER_ON;
    return 0;
}

int R_StateManager_PowerOff(int domain_id)
{
    (void)domain_id;
    return 0;
}

int R_StateManager_PowerOn(int domain_id)
{
    (void)domain_id;
    return 0;
}

int R_StateManager_SetClock(int clock_id, uint32_t *rates)
{
    (void)clock_id;
    (void)rates;
    return 0;
}

int R_StateManager_GetClock(int clock_id, uint32_t *rates)
{
    (void)clock_id;
    (void)rates;
    return 0;
}

int R_StateManager_ClockOff(int clock_id)
{
    (void)clock_id;
    return 0;
}

int R_StateManager_ClockOn(int clock_id)
{
    (void)clock_id;
    return 0;
}

int R_StateManager_ClockStatusGet(int clock_id, bool *status)
{
    (void)clock_id;
    (void)status;
    return 0;
}

int R_StateManager_ResetAssert(int domain_id)
{
    (void)domain_id;
    return 0;
}

int R_StateManager_ResetDeassert(int domain_id)
{
    (void)domain_id;
    return 0;
}

int R_StateManager_Reset(int domain_id)
{
    (void)domain_id;
    return 0;
}

int R_StateManager_Reset_Status_Get(int domain_id, e_reset_domain_status_t *status) {
	(void) domain_id;
	(void) status;
	return 0;
}
