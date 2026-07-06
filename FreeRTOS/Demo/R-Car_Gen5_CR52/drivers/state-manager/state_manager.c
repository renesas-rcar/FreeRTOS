/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "scmi/rcar_scmi_common.h"
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
#include "board.h"
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
        if ((id) >= (max)) { \
            SM_LOG_ERR("Invalid ID\n\r"); \
            return -1; \
        } \
    } while(0)

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
		if (ret != 0) {
			SM_LOG_ERR("Error: Failed to request system notification %d (ret %d).\r\n",
					notifier->system_state, ret);
			return;
		}
		/* Post shutdown or suspend */
	}
}

int R_StateManager_Init(void)
{
	int ret;
	struct scmi_protocol *proto;
	uint32_t version = 0U;
	uint32_t attributes;

    if (initialized) {
        SCMI_LOG_INFO("State Manager is initialized already!\r\n");
        return 0;
    }

	ret = scmi_driver_init();
	if (ret) {
		SM_LOG_ERR("Error: Failed to init scmi driver.");
		return ret;
	}

	ret = scmi_base_version_get(&version);
	if (ret) {
		SM_LOG_ERR("Error: Failed to get scmi base protocol version.\r\n");
		return ret;
	}
	SCMI_LOG_INFO("SCMI protocol version=0x%x", version);

	ret = scmi_power_protocol_attributes(&attributes);
	if (ret) {
		SM_LOG_ERR("Error: Failed to get scmi power protocol attr.\r\n");
		return ret;
	}
	max_powerdomain_num = attributes;
	SCMI_LOG_INFO("Number of supported power domains: %d", max_powerdomain_num);

	ret = scmi_clock_protocol_attributes(&attributes);
	if (ret) {
		SM_LOG_ERR("Error: Failed to get scmi clock protocol attr.\r\n");
		return ret;
	}
	max_clockdomain_num = attributes;
	SCMI_LOG_INFO("Number of supported clock domains: %d", max_clockdomain_num);

	ret = scmi_reset_protocol_attributes(&attributes);
	if (ret) {
		SM_LOG_ERR("Error: Failed to get scmi reset protocol attr.\r\n");
		return ret;
	}
	max_resetdomain_num = attributes;
	SCMI_LOG_INFO("Number of supported reset domains: %d", max_resetdomain_num);

	ret = scmi_system_request_notify(true);
	if (ret) {
		SM_LOG_ERR("Error: Failed to request system notification.\r\n");
		return ret;
	}
	proto = scmi_system_proto_get();
	scmi_notifier_callback_register(proto, system_notification);

    initialized = true;

	return ret;
}

/* Show SCMI Protocols' information */
int R_StateManager_SCMI_Info_Show(void)
{
    /* This function only displays logs, and since the default level is ERROR, SM_LOG_ERR must be used to print. */
    
	int ret;
	uint32_t version = 0U;

	{
		uint8_t num_protocols = 0, num_agents = 0;
		ret = scmi_base_attributes_get(&num_protocols, &num_agents);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol attributes.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol num protos: %d, num agents: %d",
				num_protocols, num_agents);
	}
	{
		uint8_t vendor_id[16];
		ret = scmi_base_vendorid_get(false, vendor_id);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol vendor id.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol vendor id: %s", vendor_id);

		memset(vendor_id, 0, 16);
		ret = scmi_base_vendorid_get(true, vendor_id);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol sub vendor id.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol sub vendor id: %s", vendor_id);
	}
	{
		uint32_t impl_version = 0;
		ret = scmi_base_implementation_version_get(&impl_version);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol impl version.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol impl_version: 0x%08x", impl_version);
	}
	{
		uint32_t num_protocols;
		uint8_t *protocols = NULL;
		int i;
		ret = scmi_base_discover_list_protocols(&num_protocols, &protocols);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol list protos.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol num_protocols: %d", num_protocols);
		for (i = 0; i < num_protocols; ++i) {
			SM_LOG_INFO("protocols[%d] = %d", i, protocols[i]);
		}

        if (protocols != NULL)
        {
            vPortFree(protocols);
            protocols = NULL;
        }
	}
	{
		uint32_t agent_id = 0;
		uint8_t name[16];
		ret = scmi_base_discover_agent_get(0xFFFFFFFFU, &agent_id, name);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi base protocol agent get.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI base protocol agent_id: %d, name: %s\r\n", agent_id, name);
	}
	{
		ret = scmi_system_version_get(&version);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi system protocol version.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI system protocol version=0x%x", version);
	}
	{
		ret = scmi_power_version_get(&version);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi power domain protocol version.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI PD protocol version=0x%x", version);
	}
	{
		ret = scmi_clock_version_get(&version);
		if (ret) {
			SM_LOG_ERR("Error: Failed to get scmi clock protocol version.\r\n");
			return ret;
		}
		SM_LOG_INFO("SCMI clock protocol version=0x%x", version);
	}

	return 0;
}

int R_StateManager_RequestDeepStop(void)
{
	int ret;

	if (0 != CURRENT_CORE_IDX) {
		SM_LOG_ERR("Only main FreeRTOS can request Deep Stop!");
		return -1;
	}
	SM_LOG_INFO("System is suspending...");
	ret = scmi_system_power_state_set(FLAGS_GRACEFUL, SYSTEM_STATE_SUSPEND);
	if (ret) {
		SM_LOG_ERR("Error: Failed to request S2R");
		return ret;
	}

	return 0;
}

int R_StateManager_SysReboot(void)
{
	int ret;

	SM_LOG_INFO("System is resetting...");
	ret = scmi_system_power_state_set(FLAGS_GRACEFUL, SYSTEM_STATE_COLD_RESET);
	if (ret) {
		SM_LOG_ERR("Error: Failed to set system suspend gracefully.");
		return ret;
	}

	return 0;
}

int R_StateManager_SysPowerOff(void)
{
	int ret;

	SM_LOG_INFO("System is shutting down...");
	ret = scmi_system_power_state_set(FLAGS_FORCEFUL, SYSTEM_STATE_SHUTDOWN);
	if (ret) {
		SM_LOG_ERR("Error: Failed to shutdown system forcefully.");
		return ret;
	}

	return 0;
}

int R_StateManager_Power_Get(int domain_id, e_power_state_t *state)
{
	struct scmi_power_state_config pwr_cfg;
	int ret;

	VALIDATE_ID(domain_id, max_powerdomain_num);
	pwr_cfg.domain_id = domain_id;
	ret = scmi_power_state_get(&pwr_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to get power domain %d (%d)\r\n", domain_id, ret);
		return ret;
	}

	if (pwr_cfg.power_state == SCMI_POWER_STATE_ON) {
		*state = POWER_ON;
	} else if (pwr_cfg.power_state == SCMI_POWER_STATE_OFF) {
		*state = POWER_OFF;
	}

	return 0;
}

int R_StateManager_PowerOff(int domain_id)
{
	struct scmi_power_state_config pwr_cfg;
	int ret;

	VALIDATE_ID(domain_id, max_powerdomain_num);
	pwr_cfg.domain_id = domain_id;
#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	if ((X5H_POWER_DOMAIN_ID_VIPN <= domain_id) &&
			(X5H_POWER_DOMAIN_ID_P_RPU_CORE00 > domain_id)) {
		pwr_cfg.flags = 0;
	} else if ((X5H_POWER_DOMAIN_ID_P_RPU_CORE00 <= domain_id) &&
			(X5H_POWER_DOMAIN_ID_Q_APU_P07 >= domain_id)) {
		pwr_cfg.flags = SCMI_POWER_STATE_SET_FLAGS_ASYNC;
	} else {
		SM_LOG_ERR("Invalid power domain ID.\r\n");
		return -EINVAL;
	}
#else
    pwr_cfg.flags = 0;
#endif
	pwr_cfg.power_state= SCMI_POWER_STATE_OFF;

	ret = scmi_power_state_set(&pwr_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to set power domain %d OFF (%d)\r\n", domain_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_PowerOn(int domain_id)
{
	struct scmi_power_state_config pwr_cfg;
	int ret;

	VALIDATE_ID(domain_id, max_powerdomain_num);
	pwr_cfg.domain_id = domain_id;
#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
	if ((X5H_POWER_DOMAIN_ID_VIPN <= domain_id) &&
			(X5H_POWER_DOMAIN_ID_P_RPU_CORE00 > domain_id)) {
		pwr_cfg.flags = 0;
	} else if ((X5H_POWER_DOMAIN_ID_P_RPU_CORE00 <= domain_id) &&
			(X5H_POWER_DOMAIN_ID_Q_APU_P07 >= domain_id)) {
		pwr_cfg.flags = SCMI_POWER_STATE_SET_FLAGS_ASYNC;
	} else {
		SM_LOG_ERR("Invalid power domain ID.\r\n");
		return -EINVAL;
	}
#else
    pwr_cfg.flags = 0;
#endif
	pwr_cfg.power_state= SCMI_POWER_STATE_ON;

	ret = scmi_power_state_set(&pwr_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to set power domain %d ON (%d)\r\n", domain_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_SetClock(int clock_id, uint32_t *rates)
{
	struct scmi_clock_rate_config clk_cfg = {0};
	int ret;

	VALIDATE_ID(clock_id, max_clockdomain_num);
	clk_cfg.clk_id = clock_id;
	clk_cfg.flags = SCMI_CLK_RATE_SET_FLAGS_ROUNDS_AUTO;
	clk_cfg.rate[0] = rates[0];

	ret = scmi_clock_rate_set(&clk_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to set clock ID %d rate (%d)\r\n", clock_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_GetClock(int clock_id, uint32_t *rates)
{
	int ret;

	VALIDATE_ID(clock_id, max_clockdomain_num);
	ret = scmi_clock_rate_get(clock_id, rates);
	if (ret) {
		SM_LOG_ERR("Failed to get clock ID %d rate (%d)\r\n", clock_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_ClockOff(int clock_id)
{
	struct scmi_clock_config clk_cfg = {0};
	int ret;

	VALIDATE_ID(clock_id, max_clockdomain_num);
	clk_cfg.clk_id = clock_id;
	clk_cfg.attributes = SCMI_CLK_CONFIG_ENABLE_DISABLE(0);

	ret = scmi_clock_config_set(&clk_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to set clock ID %d OFF (%d)\r\n", clock_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_ClockOn(int clock_id)
{
	struct scmi_clock_config clk_cfg = {0};
	int ret;

	VALIDATE_ID(clock_id, max_clockdomain_num);
	clk_cfg.clk_id = clock_id;
	clk_cfg.attributes = SCMI_CLK_CONFIG_ENABLE_DISABLE(1);

	ret = scmi_clock_config_set(&clk_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to set clock ID %d ON (%d)\r\n", clock_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_ClockStatusGet(int clock_id, bool *status)
{
	struct scmi_clock_config_get clk_cfg_get = {0};
    uint32_t flags = 0; /* Currently unused */
	int ret;

	VALIDATE_ID(clock_id, max_clockdomain_num);

	ret = scmi_clock_config_get(clock_id, flags, &clk_cfg_get);
	if (ret) {
		SM_LOG_ERR("Failed to get clock ID %d status (%d)\r\n", clock_id, ret);
		return ret;
	}

    *status = (clk_cfg_get.config & 0x1) ? true : false;

	return 0;
}

int R_StateManager_ResetAssert(int domain_id)
{
	struct scmi_reset_domain_request_config rst_cfg = {0};
	int ret;

	VALIDATE_ID(domain_id, max_resetdomain_num);
	rst_cfg.domain_id = domain_id;
	rst_cfg.flags = RESET_DOMAIN_FLAGS_EXPLICIT;

	ret = scmi_reset_domain_request(rst_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to assert domain ID %d (%d)\r\n", domain_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_ResetDeassert(int domain_id)
{
	struct scmi_reset_domain_request_config rst_cfg = {0};
	int ret;

	VALIDATE_ID(domain_id, max_resetdomain_num);
	rst_cfg.domain_id = domain_id;
	rst_cfg.flags = 0;

	ret = scmi_reset_domain_request(rst_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to deassert domain ID %d (%d)\r\n", domain_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_Reset(int domain_id)
{
	struct scmi_reset_domain_request_config rst_cfg = {0};
	int ret;

	VALIDATE_ID(domain_id, max_resetdomain_num);
	rst_cfg.domain_id = domain_id;
	rst_cfg.flags = RESET_DOMAIN_FLAGS_AUTO;

	ret = scmi_reset_domain_request(rst_cfg);
	if (ret) {
		SM_LOG_ERR("Failed to reset domain ID %d (%d)\r\n", domain_id, ret);
		return ret;
	}

	return 0;
}

int R_StateManager_Reset_Status_Get(int domain_id, e_reset_domain_status_t *status) {
    struct scmi_vendor_request_config rst_cfg = {0};
    int ret = RET_OK;

    VALIDATE_ID(domain_id, max_resetdomain_num);
    rst_cfg.domain_id = domain_id;
    
    ret = scmi_vendor_reset_domain_status_get(&rst_cfg);
    if (ret != RET_OK) {
        printf("Failed to get status of reset domain ID %d (%d)\r\n", domain_id, ret);
    } else {
        *status = (rst_cfg.reset_status == 0 ? RESET_DOMAIN_ASSERTED : RESET_DOMAIN_RELEASED);
    }

    return ret;
}
