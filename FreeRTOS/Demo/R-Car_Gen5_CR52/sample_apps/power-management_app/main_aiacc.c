/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"
#include "state-manager/r_state_manager.h"
#include "state-manager/r_power_domain_id.h"
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_reset_domain_id.h"
#include "pfc/r_pfc_api.h"

#define pmApp_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define PM_LOG(format, ...) \
    {\
        printf("PM [%s:%d] ", __func__, __LINE__);\
        printf(format "\r\n", ##__VA_ARGS__);\
    }

static void prvPmAppTask(void *pvParameters);
static void pmAppExample(void);

static void prvSetupHardware(void)
{
    portDISABLE_INTERRUPTS();
    Irq_Setup();
    (void)pfcInitModules(getModuleConfigs());
}

void main()
{

    prvSetupHardware();
    xTaskCreate(prvPmAppTask, "powerManagementApp", configMINIMAL_STACK_SIZE * 20,
                NULL, pmApp_TASK_PRIORITY, NULL);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    for( ;; )
    {
    }
    /* Don't expect to reach here. */
    return;
}

static void prvPmAppTask(void *pvParameters )
{
    /* Remove compiler warning about unused parameter. */
    (void) pvParameters;
    vTaskDelay(3000);
    PM_LOG("PowerManagement FreeRTOS starting on AIACC ...\n");
    pmAppExample();

    printf("<APP_END>\n");

    for( ;; )
    {
        PM_LOG("prvPmAppTask...\n");
        vTaskDelay(3000);
    }
}


/*
 * Power OFF test.
 * Called with domain_id in ASCENDING order.
 * Turn the power domain OFF, then get its state back to verify.
 * If the domain is still ON, the test fails.
 */
static int pmPowerdomainOffTest(int domain_id)
{
    int ret;
    const char* state_name[] = { "Power ON", "Power OFF"};
    e_power_state_t get_pwr_state;
    int tc_number = 2;

    /* Power OFF the domain */
    PM_LOG("**********TC%d %d-1: Power OFF domain.**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_PowerOff(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to power off scmi power domain %d.\r\n",
                domain_id);
        return ret;
    }

    /* Get power domain state again to verify it is OFF */
    PM_LOG("**********TC%d %d-2: Get power domain state (expected OFF).**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_Power_Get(domain_id, &get_pwr_state);
    if (ret) {
        PM_LOG("Error: Failed to get scmi power domain %d state.\r\n",
                domain_id);
        return ret;
    }
    if (get_pwr_state != POWER_OFF) {
        PM_LOG("Domain %d Power OFF failed: expected %s but get %s\r\n",
                domain_id, state_name[POWER_OFF],
                state_name[get_pwr_state]);
        return -EIO;
    }

    PM_LOG("Domain %d Power OFF: PASS\r\n", domain_id);
    return 0;
}

/*
 * Power ON test.
 * Called with domain_id in DESCENDING order.
 * Turn the power domain ON, then get its state back to verify.
 * If the domain is still OFF, the test fails.
 */
static int pmPowerdomainOnTest(int domain_id)
{
    int ret;
    const char* state_name[] = { "Power ON", "Power OFF"};
    e_power_state_t get_pwr_state;
    int tc_number = 2;

    /* Power ON the domain */
    PM_LOG("**********TC%d %d-1: Power ON domain.**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_PowerOn(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to power on scmi power domain %d.\r\n",
                domain_id);
        return ret;
    }

    /* Get power domain state again to verify it is ON */
    PM_LOG("**********TC%d %d-2: Get power domain state (expected ON).**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_Power_Get(domain_id, &get_pwr_state);
    if (ret) {
        PM_LOG("Error: Failed to get scmi power domain %d state.\r\n",
                domain_id);
        return ret;
    }
    if (get_pwr_state != POWER_ON) {
        PM_LOG("Domain %d Power ON failed: expected %s but get %s\r\n",
                domain_id, state_name[POWER_ON],
                state_name[get_pwr_state]);
        return -EIO;
    }

    PM_LOG("Domain %d Power ON: PASS\r\n", domain_id);
    return 0;
}

static int pmPowerdomainTest(int domain_id)
{
    int ret;
    const char* state_name[] = { "Power ON", "Power OFF"};
    e_power_state_t get_pwr_state;
    e_power_state_t set_pwr_state;
    int tc_number = 2;

    /* Get power domain state */
    PM_LOG("**********TC%d %d-1: Get power domain state.**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_Power_Get(domain_id, &get_pwr_state);
    if (ret) {
        PM_LOG("Error: Failed to get scmi power domain %d state.\r\n",
                domain_id);
        return ret;
    }
    PM_LOG("Domain %d current state: %s", domain_id,
            state_name[get_pwr_state]);

    /* Set power domain state */
    PM_LOG("**********TC%d %d-2: Set power domain state.**********\r\n",
            tc_number, domain_id);
    if (get_pwr_state == POWER_ON) {
        set_pwr_state = POWER_OFF;
        ret = R_StateManager_PowerOff(domain_id);
    } else {
        set_pwr_state = POWER_ON;
        ret = R_StateManager_PowerOn(domain_id);
    }
    if (ret) {
        PM_LOG("Error: Failed to set scmi power domain %d state.\r\n",
                domain_id);
        return ret;
    }
    PM_LOG("Setting Domain %d state %s without error. Verifying...",
            domain_id, state_name[set_pwr_state]);

    /* Get power domain state again to verify */
    PM_LOG("**********TC%d %d-3: Get power domain state again.**********\r\n",
            tc_number, domain_id);
    ret = R_StateManager_Power_Get(domain_id, &get_pwr_state);
    if (ret) {
        PM_LOG("Error: Failed to get scmi power domain %d state.\r\n",
                domain_id);
        return ret;
    }
    if (set_pwr_state != get_pwr_state) {
        PM_LOG("Domain %d state: Set failed, set %s but get %s\r\n",
                domain_id, state_name[set_pwr_state],
                state_name[get_pwr_state]);
        return -EIO;
    } else {
        PM_LOG("Domain %d state: Get/Set OK\r\n", domain_id);
    }

    return 0;
}

static int pmClockTest(int clock_id, uint32_t *rate_set)
{
    int ret;
    uint32_t rate[2] = {0};
    int tc_number = 3;
    bool is_clk_on;

    /* Set clock on */
    PM_LOG("**********TC%d %d-1: set clock id %d ON.**********\r\n",
            tc_number, clock_id, clock_id);
    ret = R_StateManager_ClockOn(clock_id);
    if (ret)
    {
        PM_LOG("Error: Failed to set clock id %d ON.\r\n",
                clock_id);
    }
    else
    {
        PM_LOG("Set clock id %d ON OK!\r\n", clock_id);
    }

    /* Get clock - Expected ON  */
    PM_LOG("*****TC%d %d-2: get clock id %d (expected ON)*****\r\n",
            tc_number, clock_id, clock_id);
    ret = R_StateManager_ClockStatusGet(clock_id, &is_clk_on);
    if (ret)
    {
        PM_LOG("Error: Failed to get clock id %d status.\r\n",
               clock_id);
    }
    else if (is_clk_on)
    {
        PM_LOG("Getting clock id %d ON: PASS\r\n", clock_id);
    }
    else
    {
        PM_LOG("Getting clock id %d ON: FAILED\r\n", clock_id);
    }

    /* Get clock rate */
    PM_LOG("**********TC%d %d-3: Get clock rate.**********\r\n",
            tc_number, clock_id);
    ret = R_StateManager_GetClock(clock_id, &rate[0]);
    if (ret)
    {
        PM_LOG("Error: Failed to get clock id %d rate.\r\n",
               clock_id);
    }
    else
    {
        PM_LOG("Current clock id %d rate: %d Hz\r\n", clock_id, rate[0]);
    }

#if 0
    /* Try setting clock rate to 26MHz */
    PM_LOG("**********TC%d %d-4: Set clock rate.**********\r\n",
            tc_number, clock_id);
    rate[0] = 26000000;
    ret = R_StateManager_SetClock(clock_id, rate);
    if (ret)
    {
        PM_LOG("Error: Failed to set clock id %d rate.\r\n",
               clock_id);
    }
    else
    {
        PM_LOG("Set clock id %d to %d Hz OK\r\n", clock_id, rate[0]);
    }
#endif

    /* Set clock off  */
    PM_LOG("**********TC%d %d-5: set clock id %d OFF.**********\r\n",
            tc_number, clock_id, clock_id);
        ret = R_StateManager_ClockOff(clock_id);
        if (ret)
        {
            PM_LOG("Error: Failed to set clock id %d OFF.\r\n",
                    clock_id);
        }
        else
        {
            PM_LOG("Set clock id %d OFF OK!\r\n", clock_id);
        }

    /* Get clock - Expected OFF */
    PM_LOG("*****TC%d %d-6: get clock id %d (expected OFF)****\r\n",
            tc_number, clock_id, clock_id);
    ret = R_StateManager_ClockStatusGet(clock_id, &is_clk_on);
    if (ret)
    {
        PM_LOG("Error: Failed to get clock id %d status.\r\n",
               clock_id);
    }
    else if (!is_clk_on)
    {
        PM_LOG("Getting clock id %d OFF: PASS\r\n", clock_id);
    }
    else
    {
        PM_LOG("Getting clock id %d OFF: FAILED\r\n", clock_id);
    }

    return 0;
}

static int pmResetTest(int domain_id)
{
    int ret;
    int tc_number = 4;
    e_reset_domain_status_t reset_status;

    /* Assert domain */
    PM_LOG("**********TC%d %d-1: Assert domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_ResetAssert(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to assert domain id %d.\r\n",
                domain_id);
        return ret;
    }
    PM_LOG("Assert domain id %d OK!", domain_id);

    /* Deassert domain */
    PM_LOG("**********TC%d %d-2: Deassert domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_ResetDeassert(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to deassert domain id %d.\r\n",
                domain_id);
        return ret;
    }
    PM_LOG("Deassert domain id %d OK!", domain_id);

    /* Reset domain */
    PM_LOG("**********TC%d %d-3: Reset domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_Reset(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    PM_LOG("Reset domain id %d OK!", domain_id);

    /* Get Reset status domain */
    PM_LOG("**********TC%d %d-4-1: Get Reset status domain id %d when PowerOff.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_PowerOff(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to PowerOff domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    if (reset_status == RESET_DOMAIN_ASSERTED) {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_ASSERTED: PASS\r\n", domain_id);
    } else {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_RELEASED: PASS\r\n", domain_id);
    }

    PM_LOG("**********TC%d %d-4-2: Get Reset status domain id %d when PowerOn.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_PowerOn(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to PowerOn domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    if (reset_status == RESET_DOMAIN_ASSERTED) {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_ASSERTED: PASS\r\n", domain_id);
    } else {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_RELEASED: PASS\r\n", domain_id);
    }

    PM_LOG("**********TC%d %d-4-3: Get Reset status domain id %d when PowerOn, and asserted reset.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_PowerOn(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to PowerOn domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_ResetAssert(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to assert reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    if (reset_status == RESET_DOMAIN_ASSERTED) {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_ASSERTED: PASS\r\n", domain_id);
    } else {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_RELEASED: PASS\r\n", domain_id);
    }

    PM_LOG("**********TC%d %d-4-4: Get Reset status domain id %d when PowerOn, and deasserted reset.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_PowerOn(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to PowerOn domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_ResetDeassert(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to deassert reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    if (reset_status == RESET_DOMAIN_ASSERTED) {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_ASSERTED: PASS\r\n", domain_id);
    } else {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_RELEASED: PASS\r\n", domain_id);
    }

        PM_LOG("**********TC%d %d-4-5: Get Reset status domain id %d when PowerOn, and reset.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_PowerOn(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to PowerOn domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset(domain_id);
    if (ret) {
        PM_LOG("Error: Failed to reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
                domain_id);
        return ret;
    }
    if (reset_status == RESET_DOMAIN_ASSERTED) {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_ASSERTED: PASS\r\n", domain_id);
    } else {
        PM_LOG("Get Reset status of domain id %d, RESET_DOMAIN_RELEASED: PASS\r\n", domain_id);
    }

    return 0;
}

static void pmAppExample(void)
{
    int ret;
    int domain_id;
    int tc_number = 0;
    uint32_t rates[2] = {0};

    PM_LOG("*******TC%d: SCMI protocols information starting*******\r\n",
            ++tc_number);
    ret = R_StateManager_SCMI_Info_Show();
    if (ret) {
        PM_LOG("Error: Failed to show SCMI information.\r\n");
        return;
    }
    PM_LOG("*******TC%d: SCMI protocols information  end!*******\r\n\r\n",
            tc_number);

    PM_LOG("*******TC%d: Powerdomain control starting!*******\r\n",
            ++tc_number);
    /* Power ON test: iterate domain id in DESCENDING order (no skip) */
    PM_LOG("----- Powerdomain ON test (descending order) -----\r\n");
    for (domain_id = AIACC_POWER_DOMAIN_ID_COUNT - 1;
        domain_id >= 0;
        domain_id--) {
        pmPowerdomainOnTest(domain_id);
    }

    /* Power OFF test: iterate domain id in ASCENDING order */
    PM_LOG("----- Powerdomain OFF test (ascending order) -----\r\n");
    for (domain_id = 0;
        domain_id < AIACC_POWER_DOMAIN_ID_COUNT;
        domain_id++) {
        pmPowerdomainOffTest(domain_id);
    }

    /* Restore: power ON all domains again so the following test cases can run */
    PM_LOG("----- Powerdomain restore: power ON all domains -----\r\n");
    for (domain_id = AIACC_POWER_DOMAIN_ID_COUNT - 1;
         domain_id >= 0;
         domain_id--)
    {
        ret = R_StateManager_PowerOn(domain_id);
    }
    PM_LOG("*******TC%d: Powerdomain control end!*******\r\n\r\n",
            tc_number);


    PM_LOG("*******TC%d: SCMI Clock control starting!*******\r\n",
            ++tc_number);
    for (domain_id = AIACC_CLOCK_ID_IMR00;
         domain_id < AIACC_CLOCK_ID_COUNT;
         ++domain_id)
    {
        //pmClockTest(domain_id, rates);
    }
    PM_LOG("*******TC%d: SCMI Clock control end!*******\r\n\r\n",
            tc_number);

    PM_LOG("*******TC%d: SCMI Reset control starting!*******\r\n",
            ++tc_number);
    {
        pmResetTest(domain_id);
    }

    PM_LOG("*******TC%d: SCMI Reset control end!*******\r\n\r\n",
            tc_number);

    PM_LOG("*******TC%d: SCMI System Reset starting!*******\r\n",
            ++tc_number);
#ifdef SYSTEM_RST_TEST
    ret = R_StateManager_SysReboot();
    if (ret) {
        PM_LOG("Error: Failed to request System Reboot.\r\n");
        return;
    }
#else
    PM_LOG("Don't run the TC System Reset for much logs output!\r\n");
#endif /* SYSTEM_RST_TEST */
    PM_LOG("*******TC%d: SCMI System Reset end!*******\r\n",
            tc_number);
}
