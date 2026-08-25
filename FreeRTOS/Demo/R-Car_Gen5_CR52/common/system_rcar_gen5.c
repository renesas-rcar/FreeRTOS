/*
 * Copyright (C) 2019-2020 Renesas Electronics Europe Ltd. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "interrupts.h"
#include "board.h"
#include "cmsis_rcar_gen5.h"
#include "mpu.h"
#include "state-manager/r_state_manager.h"
#include "memory_map/memory_map.h"
#include "tcm.h"
#include "serial/r_serial.h"
#include "pfc/r_pfc_api.h"
#include "ucie/r_ucie.h"
#include "ucie_common.h"
#include "rcar_utils.h"

#define CNTCR_ADDR   ((volatile uint32_t *)0x1C000000) // Counter Control Register
#define SILENT_CONSOLE_ON (1U)

extern const unsigned int __bss_start__;
extern const unsigned int __bss_end__;
extern const unsigned int _STACK_SIZE;

extern char _RAM_START;
extern const uint32_t _RAM_SIZE;
extern const uint32_t _TCM_SIZE;

extern uint32_t __tcm_start__, __tcm_end__;
extern const uint32_t __kernel_region_start__, __kernel_region_end__;
static int is_linker_tcm_symbols_define = 0;

extern uint32_t __CONFIG_SILENT_CONSOLE__ __attribute__((weak)); /* Weak linker symbol for log control, NULL if not defined in linker script */
extern uint32_t _Reset;
uint32_t resource_table;
#if ETHER_ENABLE
extern uint32_t eth_non_cache_start;
#endif

#if RAM_CONSOLE_ENABLE
char ram_console[1024] = "";
#endif

extern int main(void);

extern void __libc_init_array(void) ;

static int init_linker_symbols_check(void)
{
#if (TCM_ENABLE == 1)
    if ((__kernel_region_start__ != 0u) && (__kernel_region_end__ != 0u) &&
        (__tcm_start__ != 0u) && (__tcm_end__ != 0u) &&
        (uint32_t)&_TCM_SIZE != 0u &&
        ((uint32_t)&__kernel_region_end__ > (uint32_t)&__kernel_region_start__) &&
        ((uint32_t)&__tcm_end__ > (uint32_t)&__tcm_start__))
    {
        return 1;
    }
    return 0;
#else
    return 0;
#endif
}

static void Init_MPU(void)
{
//    uint32_t entry_address = (uint32_t) &_RAM_START;
    /* Disable MPU */
//    MPU_Disable();

    MPU_Init();

    is_linker_tcm_symbols_define = init_linker_symbols_check();

#if (TCM_ENABLE == 1)    
    if (is_linker_tcm_symbols_define == 1) 
    {
        MPU_SetRegion(REGION_SRAM_ATTR((uint32_t) &__kernel_region_start__, (uint32_t) &__kernel_region_end__ - (uint32_t) &__kernel_region_start__));
        MPU_SetRegion(REGION_TCM_ATTR((uint32_t) &__tcm_start__, (uint32_t) &_TCM_SIZE));
        MPU_SetRegion(REGION_SRAM_ATTR((uint32_t) &__tcm_end__, (uint32_t) &_RAM_SIZE - ((uint32_t) &__tcm_end__ - (uint32_t) &__kernel_region_start__)));
    }
    else
    {
        MPU_SetRegion(REGION_SRAM_ATTR((uint32_t) &_RAM_START, (uint32_t) &_RAM_SIZE));
    }
#else
    MPU_SetRegion(REGION_SRAM_ATTR((uint32_t) &_RAM_START, (uint32_t) &_RAM_SIZE));
#endif

    for (int i = 0; i < sizeof(RCAR_MEMMORY_ARR)/sizeof(st_memory_region_t); i++) {
       
        uint8_t ret = 0;

        // Avoid duplicating execution and IO memory.
        if (RCAR_MEMMORY_ARR[i].mem_addr.base_address == (unsigned int)(uintptr_t)&_RAM_START) {
            continue;
        }

        switch (RCAR_MEMMORY_ARR[i].attr) {
            case DEVICE_ATTR:
                ret = MPU_SetRegion(REGION_DEVICE_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case RAM_ATTR:
                ret = MPU_SetRegion(REGION_RAM_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case RAM_NOCACHE_ATTR:
                ret = MPU_SetRegion(REGION_RAM_NOCACHE_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case RAM_TEXT_ATTR:
                ret = MPU_SetRegion(REGION_RAM_TEXT_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case RAM_RO_ATTR:
                ret = MPU_SetRegion(REGION_RAM_RO_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case SRAM_ATTR:
                ret = MPU_SetRegion(REGION_SRAM_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case FLASH_ATTR:
                ret = MPU_SetRegion(REGION_FLASH_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            case TCM_ATTR:
                ret = MPU_SetRegion(REGION_TCM_ATTR(RCAR_MEMMORY_ARR[i].mem_addr.base_address, RCAR_MEMMORY_ARR[i].mem_addr.size));
                break;

            default:
#if RAM_CONSOLE_ENABLE
                snprintf(ram_console + strlen(ram_console), sizeof(ram_console) - strlen(ram_console), "Set MPU region index %d FAIL. Memory attribute isn't supported;", i + 1);
#endif
			    break;
        }

        if (ret != 0U) {
#if RAM_CONSOLE_ENABLE
            snprintf(ram_console + strlen(ram_console), sizeof(ram_console) - strlen(ram_console), "Set MPU region index %d FAIL. Exceeded number of MPU regions supported;", i + 1);
#endif
        } 
    } 

    /* Enable MPU */
    MPU_Enable();
}

__STATIC_INLINE void bss_init(unsigned int* section_begin, unsigned int* section_end)
{
  // Iterate and clear word by word.
  // It is assumed that the pointers are word aligned.
  unsigned int *p = section_begin;
    while (p < section_end) {
    *p++ = 0;
    }
}

static void FPU_Enable(void)
{
#define BSP_CPCAR_CP_ENABLE             (0x00F00000)
#define BSP_FPEXC_EN_ENABLE             (0x40000000)
    uint32_t apacr;
    uint32_t fpexc;

    /* Enables cp10 and cp11 accessing */
    apacr  = __get_CPACR();
    apacr |= BSP_CPCAR_CP_ENABLE;
    __set_CPACR(apacr);
    __ISB();

    /* Enables the FPU */
    fpexc  = __get_FPEXC();
    fpexc |= BSP_FPEXC_EN_ENABLE;
    __set_FPEXC(fpexc);
    __ISB();

}

/* Enable data and instruction cache in SVC mode */
void EnableCache()
{
    uint32_t reg_val = 0;

    // Invalidate all Instruction Caches to PoU (ICIALLU)
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r"(0) : "memory");

    // Invalidate all entries from branch predictors (BPIALL)
    __asm__ volatile ("mcr p15, 0, %0, c7, c5, 6" : : "r"(0) : "memory");

    // Read System Control Register (SCTLR)
    __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(reg_val) : : "memory");

    // instruction cache enable (SCTLR.I), data cache enable (SCTLR.C)
    reg_val |= BIT(12) | BIT (2);

    // Enabled instruction and data cache (SCTLR)
    __asm__ volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r"(reg_val) : "memory");

    __DSB();
    __ISB();
}

static void EnablePMU(void)
{
    uint32_t value;

    // Enable PMU and reset event and cycle counters
    value = ((uint32_t)1 << 0)  // E: All counters are enabled.
          | ((uint32_t)1 << 2); // C: Reset cycle counter.
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r"(value));

    // Enable cycle counter (counter 31)
    value = ((uint32_t)1 << 31);
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r"(value));
}

#if (BOARD == MDP_AIACC_HIL || BOARD == MDP_X5H_HIL)
void ucie0_setup_task( void *pvParameters )
{
    (void)pvParameters;
    e_ucie_linkup_status_t ret;

    st_ucie_ctrl_t ucie_conf = ucie_get_config(UCIE_CH0);

    uint32_t cpu_id = R_UTILS_GetCpuID();

    if (ucie_conf.init_with_system && cpu_id == 0)
    {
        if (ucie_conf.mode == UCIE_MODE_RC)
        {
            vTaskDelay(2000);
        }

        ret = Ucie_Setup_PCIE_Wait_LinkUp(UCIE_CH0);
        if (ret == LINKUP_SUCCESS) {
            /* Power off UCIe linkup by IPL and relinkup */
            Ucie_PowerOFF(UCIE_CH0);
        }

        ucie_set_setup_flag(UCIE_CH0);

        Ucie_PowerOn(UCIE_CH0);
        Ucie_Setup_Pre(UCIE_CH0, ucie_conf.mode);
        Ucie_Start_Linkup(UCIE_CH0, ucie_conf.mode, ucie_conf.speed);
        Ucie_Wait_FreqChange_Req(UCIE_CH0);
        Ucie_Ack_FreqChange(UCIE_CH0, ucie_conf.speed);
        while(Ucie_Wait_Linkup(UCIE_CH0))
        {
            vTaskDelay(10);
        }
        Ucie_Setup_PCIE_Pre(UCIE_CH0, ucie_conf.mode);
        Ucie_Setup_PCIE_Start_LinkUp(UCIE_CH0, ucie_conf.mode);
        while(Ucie_Setup_PCIE_Wait_LinkUp(UCIE_CH0))
        {
            vTaskDelay(10);
        }
        Ucie_Setup_PCIE_Post(UCIE_CH0, ucie_conf.mode);
    }

    vTaskDelete(NULL);
}

void ucie1_setup_task( void *pvParameters )
{
    (void)pvParameters;
    e_ucie_linkup_status_t ret;

    st_ucie_ctrl_t ucie_conf = ucie_get_config(UCIE_CH1);

    uint32_t cpu_id = R_UTILS_GetCpuID();

    if (ucie_conf.init_with_system && cpu_id == 0)
    {
        if (ucie_conf.mode == UCIE_MODE_RC)
        {
            vTaskDelay(2000);
        }

        ret = Ucie_Setup_PCIE_Wait_LinkUp(UCIE_CH1);
        if (ret == LINKUP_SUCCESS) {
            /* Power off UCIe linkup by IPL and relinkup */
            Ucie_PowerOFF(UCIE_CH1);
        }

        ucie_set_setup_flag(UCIE_CH1);

        Ucie_PowerOn(UCIE_CH1);
        Ucie_Setup_Pre(UCIE_CH1, ucie_conf.mode);
        Ucie_Start_Linkup(UCIE_CH1, ucie_conf.mode, ucie_conf.speed);
        Ucie_Wait_FreqChange_Req(UCIE_CH1);
        Ucie_Ack_FreqChange(UCIE_CH1, ucie_conf.speed);
        while(Ucie_Wait_Linkup(UCIE_CH1))
        {
            vTaskDelay(10);
        }
        Ucie_Setup_PCIE_Pre(UCIE_CH1, ucie_conf.mode);
        Ucie_Setup_PCIE_Start_LinkUp(UCIE_CH1, ucie_conf.mode);
        while(Ucie_Setup_PCIE_Wait_LinkUp(UCIE_CH1))
        {
            vTaskDelay(10);
        }
        Ucie_Setup_PCIE_Post(UCIE_CH1, ucie_conf.mode);
    }

    vTaskDelete(NULL);
}
#endif

void SystemInit(void)
{
    bss_init((unsigned int *)&__bss_start__, (unsigned int *)&__bss_end__);
#if (defined(__FPU_USED) && (__FPU_USED == 1U))
    FPU_Enable();
#endif
    Init_MPU();

#if (TCM_ENABLE == 1)   
    if (is_linker_tcm_symbols_define == 1) 
    {
        st_memory_t info_osal = R_UTILS_GetMemoryRegionInfo(OSAL, 0);
        volatile uint32_t *osal_mem = (volatile uint32_t *)info_osal.base_address;
        memcpy((void *)osal_mem, &__tcm_start__, (size_t)&_TCM_SIZE);

        // Configuration for TCM region B.
        ConfigureTCM(RCAR_TCM_B, (uint32_t)&__tcm_start__, RCAR_TCM_SIZE_32KB);

        // Enable TCM region B at EL1.
        ControlTCM(RCAR_TCM_B, RCAR_TCM_EL1, RCAR_TCM_ENABLE);

        memcpy(&__tcm_start__, (void *)osal_mem, (size_t)&_TCM_SIZE);
        memset((void *)osal_mem, 0, (size_t)&_TCM_SIZE);
    }
#endif

#if (CACHE == 1)
    EnableCache(); 
#endif
    EnablePMU();
    __libc_init_array();
    portDISABLE_INTERRUPTS();
    *CNTCR_ADDR = 1;    /* enable system counter */

    /* Init UART */
    (void)R_SERIAL_PortInit(UART_ID);
    if ((uint32_t)&__CONFIG_SILENT_CONSOLE__ == SILENT_CONSOLE_ON)
    {
        R_SERIAL_SetLogState(LOG_OFF);
    }

    Irq_Setup();
    if (R_StateManager_Init() != 0) {
        printf("Error: Failed to init State Manager.\r\n");
        return;
    }

#if (BOARD == MDP_AIACC_HIL || BOARD == MDP_X5H_HIL)
    xTaskCreate(ucie0_setup_task, "ucie0_setup_task", configMINIMAL_STACK_SIZE,
                NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(ucie1_setup_task, "ucie1_setup_task", configMINIMAL_STACK_SIZE,
                NULL, configMAX_PRIORITIES - 1, NULL);
#endif
}

void assert_func(const char *file, int line, const char *func)
{
    printf("ASSERT! File \"%s\", Line \"%d\", Function \"%s\" \n", file, line, func);
    for (;;)
    {
        __BKPT(0);
    }
}
