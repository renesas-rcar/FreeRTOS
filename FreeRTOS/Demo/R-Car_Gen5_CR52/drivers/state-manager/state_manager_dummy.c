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

#define SM_LOG_INFO(format, ...) \
    {\
        printf("SM: I [%s:%d] ", __func__, __LINE__);\
        printf(format "\r\n", ##__VA_ARGS__);\
    }

#define SM_LOG_ERR(format, ...) \
    {\
        printf("SM: E [%s:%d] ", __func__, __LINE__);\
        printf(format "\r\n", ##__VA_ARGS__);\
    }

#define VALIDATE_ID(id, max) \
    do { \
        if ((id) >= max) { \
            SM_LOG_ERR("Invalid ID\n\r"); \
            return -1; \
        } \
    } while(0)

uint32_t max_clockdomain_num;
uint32_t max_powerdomain_num;
uint32_t max_resetdomain_num;
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
	const char* flags_to_str[] = {"forceful", "graceful", "invalid"};
    const char* system_state_to_str[] = {
		"shutdown", "coldreset", "warmreset", "powerup", "suspend", "MAX"};
	int ret;
	scmi_syspower_state_notifier_t *notifier =
		(scmi_syspower_state_notifier_t *)data;
	static int cnt = 0;

	SM_LOG_INFO("%s has transited to %s %s with timeout %d ms",
			agentid2str(notifier->agent_id),
			flags_to_str[notifier->flags],
			system_state_to_str[notifier->system_state], notifier->timeout);

	if (((SYSTEM_STATE_SUSPEND == notifier->system_state) ||
		(SYSTEM_STATE_SHUTDOWN == notifier->system_state)) &&
		(SCMI_AGENT_ID_FRTOS_1ST != notifier->agent_id)) {
		/* Send suspend command to SCP FW
         * FreeRTOS agents other than the main one.
         */
		ret = scmi_system_power_state_set(notifier->flags, SYSTEM_STATE_SHUTDOWN);
		if (ret < 0) {
			SM_LOG_ERR("Error: Failed to request system notification %d (ret %d).\r\n",
					notifier->system_state, ret);
			return;
		}
		/* Post shutdown or suspend */
	}
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
