/*
 * Copyright (c) 2019-2020 Renesas Electronics Europe Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stddef.h>
#include "cmsis_rcar_gen5.h"
#include "interrupts.h"
#include "core_cr52.h"
#include "CMSIS_5/irq_ctrl.h"
#include "irq_ctrl.h"
#include "drivers/gic/gic.h"
#include "cmsis_cp15.h"
#include "merge_interrupt_private.h"

/* Logging Function include. */
#include "logging_stack.h"

#define MAX_IRQ_NUMBER       1019
#define DEFAULT_ISR_PRIORITY IPRIORITY(1)

typedef void (*IrqHandlerFn)(void *data);

typedef struct
{
	IrqHandlerFn Handler;
	Context_t *Context;
} IvtEntry;

static IvtEntry HandlerTable[MAX_IRQ_NUMBER];

/**
 * @brief Reads a value from the specified register address.
 *
 * @param addr The address of the register.
 *
 * @return The value read from the register.
 */
static uint32_t Irq_RegRead(uint32_t addr);

/**
 * @brief Writes a value to the specified register address.
 *
 * @param addr The address of the register.
 * @param val The value to be written to the register.
 */
static void Irq_RegWrite(uint32_t addr, uint32_t val);

/**
 * @brief Get CPU ID that program is currently running.
 *
 * @param None.
 *
 * @return CPU ID: Affinity level 0 - read and parse from MPIDR.
 */
static uint8_t Irq_GetCpuId(void);

/**
 * @brief Get GICR Address based on CPU is currently running.
 *
 * @param None.
 *
 * @return GICR Address.
 */
static uint32_t Irq_GetGICRAddr(void);

/**
 * @brief Get Affinity that indicate CPU ID and Cluster ID.
 *
 * @param None.
 *
 * @return Affinity (level 0 and level 1) read from MPIDR.
 */
static uint32_t Irq_GetAffinity(void);

void Irq_Setup(void)
{
	uint32_t rd, affinity, gicr_addr;
	affinity = Irq_GetAffinity();
	gicr_addr = Irq_GetGICRAddr();
	
	//
	// Configure the interrupt controller
	//
	// Set location of GIC
	R_GIC_SetAddr(CR52_GICD_ADDR, (void*)gicr_addr);
	
	// Enable GIC
	R_GIC_Enable();
	
	// Get the ID of the Redistributor connected to this PE
	rd = R_GIC_GetRedistID(affinity);
	// Mark this core as being active
	R_GIC_WakeUpRedist(rd);
	
	// Configure the CPU interface
	// This assumes that the SRE bits are already set
	R_GIC_SetPriorityMask(0xFF);
	R_GIC_EnableGroup0Ints();
	R_GIC_EnableGroup1Ints();
}

/* Set up a CR7 or INTC-RT GIC entry */
void Irq_SetupEntry(unsigned int id, IrqHandlerFn Handler, Context_t *Context)
{
	int ret;
	/* Just in case... */
    if (id > MAX_IRQ_NUMBER) {
        while (1) {}
    }

	ret = Irq_MergeSetup(id);

	HandlerTable[id].Handler = Handler;
	HandlerTable[id].Context = Context;

	Irq_SetPriority(id, DEFAULT_ISR_PRIORITY);
}

static void StubHandler(void *data)
{
    (void)data;
    while (1) {}
}

/* Remove a CR7 or INTC-RT GIC entry */
void Irq_RemoveEntry(unsigned int id)
{
	/* Just in case... */
    if (id > MAX_IRQ_NUMBER) {
        while (1) {}
    }

	Irq_Disable(id);

	HandlerTable[id].Handler = &StubHandler;
	HandlerTable[id].Context = NULL;
}

void Irq_Enable(unsigned int id)
{
    uint32_t rd, affinity;

    affinity = Irq_GetAffinity();
    R_GIC_SetIntRoute(id, 0, affinity);
    rd = R_GIC_GetRedistID(affinity);
    R_GIC_SetIntGroup(id, rd, GICV3_GROUP1_NON_SECURE);
    R_GIC_EnableInt(id, rd);
}
/* Legacy: IRQ_Enable should not be used */
int32_t IRQ_Enable (IRQn_ID_t irqn)
{
    Irq_Enable(irqn);
    return 0;
}

void Irq_Disable(unsigned int id)
{
	uint32_t cpu_id = (uint32_t)Irq_GetCpuId();
	R_GIC_DisableInt(id, cpu_id);
}
/* Legacy: IRQ_Disable should not be used */
int32_t IRQ_Disable (IRQn_ID_t irqn)
{
	Irq_Disable(irqn);
	return 0;
}

void Irq_SetPriority(unsigned int id, uint8_t priority)
{
    uint32_t cpu_id = (uint32_t)Irq_GetCpuId();
    R_GIC_SetIntPriority(id, cpu_id, priority);
}


/* CR52 GIC interrupt handler that also checks the INTC-RT GIC */
void vApplicationIRQHandler(uint32_t ulICCIAR)
{
	/*
	 * Interrupts cannot be re-enabled until the source of the interrupt is
	 * cleared. The ID of the interrupt is obtained by bitwise ANDing the ICCIAR
	 * value with 0x3FF
	 */
	uint32_t id = ulICCIAR & 0x3FFU;
	IvtEntry *pEntry;
	UBaseType_t UxSavedInterruptStatus;
	if (id > MAX_IRQ_NUMBER) {
        return;
	}

	pEntry = &HandlerTable[id];

	if (!pEntry->Handler) {
		/* No interrupt handler! */
		while (1) {}
	}

    int channel_info = Irq_GetMergeStatReg(id);
	if (pEntry->Context && channel_info >= 0) {
        pEntry->Context->channel_info = channel_info;
	}
	UxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
	pEntry->Handler(pEntry->Context);
	taskEXIT_CRITICAL_FROM_ISR(UxSavedInterruptStatus);
}

int Irq_GetTableId(unsigned int id)
{
	for (unsigned int i = 0 ; i < sizeof(merged_irq_table) / sizeof(merged_irq_table[0]); i++)
	{
		if (id == merged_irq_table[i].irq) {
			return i;
        }
	}

	/* The irq id is not a merged interrupt */
	return -1;
}

static uint32_t Irq_RegRead(uint32_t addr)
{
	return *((volatile uint32_t *)addr);
}

static void Irq_RegWrite(uint32_t addr, uint32_t val)
{
	*((volatile uint32_t *)addr) = val;
}

int Irq_MergeSetup(unsigned int id)
{
	int t_id;
	uint32_t val, current_val = 0;
    uint32_t timeout = 1000;

	t_id = Irq_GetTableId(id);
    if (t_id < 0) {
		return t_id;
    }

	val = Irq_RegRead(merged_irq_table[t_id].mask_reg);
	val &= ~merged_irq_table[t_id].mask_val;
	Irq_RegWrite(merged_irq_table[t_id].mask_reg, val);

    // Check whether the register is reflected setting value.
    while (timeout-- && (current_val != val)) {
        current_val = Irq_RegRead(merged_irq_table[t_id].mask_reg);
    }

    if (timeout == 0) {
        LogDebug(("Merge interrupt: Setup fail"));
    }

	return 0;
}

int Irq_GetMergeStatReg(unsigned int id)
{
	int t_id;

	t_id = Irq_GetTableId(id);
    if (t_id < 0) {
		return t_id;
    }

	return Irq_RegRead(merged_irq_table[t_id].status_reg);
}

int Irq_SetIntType(unsigned int id, r_irq_type type)
{
    return R_GIC_SetIntType(id, 0, type);
}

static uint8_t Irq_GetCpuId(void)
{
        uint32_t mpidr = __get_MPIDR();
        uint8_t cpu_id = (uint8_t)(mpidr & 0xFFU);
        return cpu_id;
}

static uint32_t Irq_GetGICRAddr(void)
{
	uint8_t cpu_id = Irq_GetCpuId();
	uint32_t gicr_addr = CR52_GIC_BASE_ADDR + gicr_offset_table[cpu_id];
	return gicr_addr;

}

static uint32_t Irq_GetAffinity(void)
{
        uint32_t mpidr = __get_MPIDR();
        uint32_t affinity = mpidr & 0xFFFF; // Get Affinity level 0 & 1
        return affinity;
}
