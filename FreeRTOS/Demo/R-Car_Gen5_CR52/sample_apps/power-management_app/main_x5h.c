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
#include "rcar_utils.h"

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
    PM_LOG("PowerManagement FreeRTOS starting on X5H...\n");
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

static int pmPowerDomainSkip(int domain_id) {
    int ret = 0;
    int cpu_id = R_UTILS_GetCpuID();
    if (((cpu_id == 0) &&
        (X5H_POWER_DOMAIN_ID_RC00 == domain_id)) ||
        ((cpu_id == 1) &&
        (X5H_POWER_DOMAIN_ID_RC01 == domain_id))) {
        /* Do nothing */
    } else {
        ret = 1;
    }
    return ret;
}

static int pmResetDomainSkip(int domain_id) {
    int ret = 0;
    int cpu_id = R_UTILS_GetCpuID();
    if (((cpu_id == 0) &&
        ((X5H_RESET_DOMAIN_ID_CR52TOP0 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52TOP0_BD == domain_id) ||
        (X5H_RESET_DOMAIN_ID_CR52CORE0 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52CORE0_PO == domain_id) ||
        (X5H_RESET_DOMAIN_ID_CR52SHADOW0 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52SHADOW0_PO == domain_id)))
        ||
        ((cpu_id == 1) &&
        ((X5H_RESET_DOMAIN_ID_CR52TOP1 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52TOP1_DEB == domain_id) ||
        (X5H_RESET_DOMAIN_ID_CR52CORE1 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52CORE1_PO == domain_id) ||
        (X5H_RESET_DOMAIN_ID_CR52SHADOW1 == domain_id) || (X5H_RESET_DOMAIN_ID_CR52SHADOW1_PO == domain_id)))) {
        /* Do nothing */
    } else {
        ret = 1;
    }
    return ret;
}

static int pmClockDomainSkip(int clock_id) {
    int ret = 0;
    int cpu_id = R_UTILS_GetCpuID();
    if (((cpu_id == 0) &&
        ((X5H_CLOCK_ID_MDLC_CR52CORE0 == clock_id) || (X5H_CLOCK_ID_MDLC_CR52CORE0_PO == clock_id) ||
        (X5H_CLOCK_ID_MDLC_CR52SHADOW0 == clock_id) || (X5H_CLOCK_ID_MDLC_CR52SHADOW0_PO == clock_id)))
        ||
        ((cpu_id == 1) &&
        ((X5H_CLOCK_ID_MDLC_CR52CORE1 == clock_id) || (X5H_CLOCK_ID_MDLC_CR52CORE1_PO == clock_id) ||
        (X5H_CLOCK_ID_MDLC_CR52SHADOW1 == clock_id) || (X5H_CLOCK_ID_MDLC_CR52SHADOW1_PO == clock_id)))) {
        /* Do nothing */
    } else {
        ret = 1;
    }
    return ret;
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
    if ((X5H_CLOCK_ID_MDLC_VIPN_MSYNC == clock_id) ||
        (X5H_CLOCK_ID_MDLC_VIPS_MSYNC == clock_id)) {
        PM_LOG("Currently not supported this operation.\r\n");
    } else {
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

    /* Get Reset status domain */
    PM_LOG("**********TC%d %d-4-2: Get Reset status domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
            domain_id);
    return ret;
    }
    if (RESET_DOMAIN_ASSERTED != reset_status){
        PM_LOG("Error: Reset status is not asserted.\r\n");
        return ret;
    }

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

    /* Get Reset status domain */
    PM_LOG("**********TC%d %d-4-2: Get Reset status domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
            domain_id);
        return ret;
    }
    if (RESET_DOMAIN_RELEASED != reset_status){
        PM_LOG("Error: Reset status is not released.\r\n");
        return ret;
    }

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
    PM_LOG("**********TC%d %d-4-2: Get Reset status domain id %d.**********\r\n",
            tc_number, domain_id, domain_id);
    ret = R_StateManager_Reset_Status_Get(domain_id, &reset_status);
    if (ret) {
        PM_LOG("Error: Failed to get status of reset domain id %d.\r\n",
            domain_id);
        return ret;
    }
    if (RESET_DOMAIN_RELEASED != reset_status){
        PM_LOG("Error: Reset status is not released.\r\n");
        return ret;
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
    for (domain_id = X5H_POWER_DOMAIN_ID_COUNT - 1;
        domain_id >= 0;
        domain_id--) {
        if (((X5H_POWER_DOMAIN_ID_P_RPU_CORE00 <= domain_id) &&
            (X5H_POWER_DOMAIN_ID_Q_APU_P07 >= domain_id))) {
            PM_LOG("Skip domain id %d \r\n", domain_id);
        } else if ((0 == pmPowerDomainSkip(domain_id)) ||
        ((X5H_POWER_DOMAIN_ID_RC00 == domain_id) || (X5H_POWER_DOMAIN_ID_RC01 == domain_id))) {
            PM_LOG("Skip domain id %d, because this sample is running on core %d \r\n",
                domain_id, R_UTILS_GetCpuID());
        } else {
            pmPowerdomainOnTest(domain_id);
        }
    }

    /* Power OFF test: iterate domain id in ASCENDING order */
    PM_LOG("----- Powerdomain OFF test (ascending order) -----\r\n");
    for (domain_id = 0;
        domain_id < X5H_POWER_DOMAIN_ID_COUNT;
        domain_id++)
    {
        if (((X5H_POWER_DOMAIN_ID_P_RPU_CORE00 <= domain_id) &&
            (X5H_POWER_DOMAIN_ID_Q_APU_P07 >= domain_id))) {
            PM_LOG("Skip domain id %d \r\n", domain_id);
        } else if ((0 == pmPowerDomainSkip(domain_id)) ||
         ((X5H_POWER_DOMAIN_ID_RC00 == domain_id) || (X5H_POWER_DOMAIN_ID_RC01 == domain_id))) {
            PM_LOG("Skip domain id %d, because this sample is running on core %d \r\n",
                domain_id, R_UTILS_GetCpuID());
        } else {
            pmPowerdomainOffTest(domain_id);
        }
    }

    /* Restore: power ON all domains again so the following test cases can run */
    PM_LOG("----- Powerdomain restore: power ON all domains -----\r\n");
    for (domain_id = X5H_POWER_DOMAIN_ID_COUNT - 1;
         domain_id >= 0;
         domain_id--)
    {
        if (((X5H_POWER_DOMAIN_ID_P_RPU_CORE00 <= domain_id) &&
            (X5H_POWER_DOMAIN_ID_Q_APU_P07 >= domain_id))) {
            // Skip domain id because this is core's domain ids
            continue;
        } else if ((X5H_POWER_DOMAIN_ID_RC00 <= domain_id) && (X5H_POWER_DOMAIN_ID_RC11 >= domain_id)) {
            PM_LOG("Skip domain id %d, because re-powering on any core may cause the system to hang \r\n",
                domain_id);
            continue;
        } else {
            ret = R_StateManager_PowerOn(domain_id);
        }
        if (ret) {
            PM_LOG("Error: Failed to restore (power on) domain %d.\r\n",
                    domain_id);
        }
    }

    PM_LOG("*******TC%d: Powerdomain control end!*******\r\n\r\n",
            tc_number);


    PM_LOG("*******TC%d: SCMI Clock control starting!*******\r\n",
            ++tc_number);
    for (domain_id = X5H_CLOCK_ID_MDLC_VIPN_FCPCS0;
         domain_id < X5H_CLOCK_ID_COUNT;
         ++domain_id)
    {
        if ((X5H_CLOCK_ID_MDLC_HSCIF0 == domain_id) ||
            (X5H_CLOCK_ID_MDLC_SCIF0 == domain_id) ||
            (X5H_CLOCK_ID_MDLC_SCIF1 == domain_id) ||
            (X5H_CLOCK_ID_MDLC_INTAP1 <= domain_id) ||
            (0 == pmClockDomainSkip(domain_id))) {
            continue;
        }

        pmClockTest(domain_id, rates);
    }
    PM_LOG("Following clock id are skipped testing due to board hang:\n"
           "- X5H_CLOCK_ID_MDLC_HSCIF0\n"
           "- X5H_CLOCK_ID_MDLC_SCIF0,\n"
           "- From X5H_CLOCK_ID_MDLC_INTAP1 to the end\r\n");
    PM_LOG("*******TC%d: SCMI Clock control end!*******\r\n\r\n",
            tc_number);

    PM_LOG("*******TC%d: SCMI Reset control starting!*******\r\n",
            ++tc_number);
    for (domain_id = X5H_RESET_DOMAIN_ID_VIPN_FCPCS0;
         domain_id < X5H_RESET_DOMAIN_ID_COUNT;
         ++domain_id)
    {
        if ((X5H_RESET_DOMAIN_ID_HSCIF0 == domain_id) ||
            (X5H_RESET_DOMAIN_ID_SCIF0 == domain_id) ||
            (X5H_RESET_DOMAIN_ID_SCIF1 == domain_id) ||
            (X5H_RESET_DOMAIN_ID_CSITOP2 == domain_id) ||
            (domain_id >= X5H_RESET_DOMAIN_ID_VCON0 && domain_id <= X5H_RESET_DOMAIN_ID_VCON9) ||
            (X5H_RESET_DOMAIN_ID_INTAP1 <= domain_id) ||
            (0 == pmResetDomainSkip(domain_id)) ||
            (domain_id >= X5H_RESET_DOMAIN_ID_CR52TOP0 && domain_id <= X5H_RESET_DOMAIN_ID_CR52TOP1_DEB)
            ) {
                printf("Skip domain id %d \r\n", domain_id);
                continue;
        }
        pmResetTest(domain_id);
    }
    PM_LOG("Following reset id are skipped testing due to board hang:\n"
           "- X5H_RESET_DOMAIN_ID_HSCIF0\n"
           "- X5H_RESET_DOMAIN_ID_SCIF0,\n"
           "- From X5H_RESET_DOMAIN_ID_INTAP1 to the end\r\n");
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
