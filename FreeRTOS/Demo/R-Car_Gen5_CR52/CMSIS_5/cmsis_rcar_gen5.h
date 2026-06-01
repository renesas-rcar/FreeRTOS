/**************************************************************************//**
 * @file     cmsis_rcar_gen5.h
 * @brief    CMSIS for Cortex-R7 on Renesas R-Car Gen3 devices
 ******************************************************************************/
/*
 * Copyright (c) 2019 Renesas Electronics Europe Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * ARM don't currently provide CMSIS for Cortex-R cores.
 * We want the code for CP15 access, instructions and memory barriers so we can
 * setup the caches, etc.
 * We also want to program the MPU, whihc is a bit similar to Cortex-M devices,
 * so we have modified CMSIS code to do this.
 */
#ifndef __CMSIS_RCAR_GEN5_H
#define __CMSIS_RCAR_GEN5_H

#include "board.h"

/* FPU */
#define __FPU_PRESENT           1

/* MPU */
#define __MPU_PRESENT           1

/* GIC */
#define __GIC_PRESENT           1

//#define GIC_DISTRIBUTOR_BASE	0xF0000000U
#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
    #define CR52_GICD_ADDR          ((void *)0xF0000000U)
    #define CR52_GICR_ADDR          ((void *)0xF0100000U)
#elif (BOARD == AI_ACC)
    #define CR52_GICD_ADDR          ((void *)0xC0000000U)
    #define CR52_GICR_ADDR          ((void *)0xC0100000U)
#else
    #define CR52_GICD_ADDR          ((void *)0xC0000000U)
    #define CR52_GICR_ADDR          ((void *)0xC0100000U)
#endif

#define CR52_CPU_ID             0
#define CR52_GIC_BASE_ADDR      ((uint32_t)CR52_GICD_ADDR)

#ifndef __ASSEMBLER__

#define BIT(nr)                   (1UL << (nr))

/* Not going to list all the interrupts */
typedef	unsigned int IRQn_Type;

#include "core_cr52.h"

typedef uintptr_t mem_addr_t;

/*GICR memory map table*/
static const uint32_t gicr_offset_table[4] = {0x100000, 0x120000, 0x140000, 0x160000};

static void barrier_dmem_fence_full() {
    // Todo
}

__STATIC_FORCEINLINE uint8_t sys_read8(mem_addr_t addr)
{
	uint8_t val;

	__asm__ volatile("ldrb %0, [%1]" : "=r" (val) : "r" (addr));

	barrier_dmem_fence_full();
	return val;
}

__STATIC_FORCEINLINE void sys_write8(uint8_t data, mem_addr_t addr)
{
	barrier_dmem_fence_full();
	__asm__ volatile("strb %0, [%1]" : : "r" (data), "r" (addr));
}

__STATIC_FORCEINLINE uint16_t sys_read16(mem_addr_t addr)
{
	uint16_t val;

	__asm__ volatile("ldrh %0, [%1]" : "=r" (val) : "r" (addr));

	barrier_dmem_fence_full();
	return val;
}

__STATIC_FORCEINLINE void sys_write16(uint16_t data, mem_addr_t addr)
{
	barrier_dmem_fence_full();
	__asm__ volatile("strh %0, [%1]" : : "r" (data), "r" (addr));
}

__STATIC_FORCEINLINE uint32_t sys_read32(mem_addr_t addr)
{
	uint32_t val;

	__asm__ volatile("ldr %0, [%1]" : "=r" (val) : "r" (addr));

	barrier_dmem_fence_full();
	return val;
}

__STATIC_FORCEINLINE void sys_write32(uint32_t data, mem_addr_t addr)
{
	barrier_dmem_fence_full();
	__asm__ volatile("str %0, [%1]" : : "r" (data), "r" (addr));
}

__STATIC_FORCEINLINE uint64_t sys_read64(mem_addr_t addr)
{
	uint64_t val;

	__asm__ volatile("ldrd %Q0, %R0, [%1]" : "=r" (val) : "r" (addr));

	barrier_dmem_fence_full();
	return val;
}
#endif /* __ASSEMBLER__ */
#endif /* __CMSIS_RCAR_GEN5_H */

