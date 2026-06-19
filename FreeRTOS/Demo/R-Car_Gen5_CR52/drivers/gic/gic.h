/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
 *************************************************************************************************************/

#ifndef R_GICV3_H_
#define R_GIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

#define GICV3_GROUP0                     (0)
#define GICV3_GROUP1_SECURE              (1)
#define GICV3_GROUP1_NON_SECURE          (2)

void R_GIC_SetICC_SRE(unsigned int value);

uint32_t R_GIC_GetICC_SRE(void);

void R_GIC_SetICC_HSRE(unsigned int value);

uint32_t R_GIC_GetICC_HSRE(void);

void R_GIC_SetICC_MSRE(unsigned int value);

uint32_t R_GIC_GetICC_MSRE(void);

/**
 * @brief Enables group 0 interrupts
 */
void R_GIC_EnableGroup0Ints(void);

/**
 * @brief Disables group 0 interrupts
 */
void R_GIC_DisableGroup0Ints(void);

/**
 * @brief Enables group 1 interrupts for current security state
 */
void R_GIC_EnableGroup1Ints(void);

/**
 * @brief Disables group 1 interrupts for current security state
 */
void R_GIC_DisableGroup1Ints(void);

void R_GIC_EnableNSGroup1Ints(void);

void R_GIC_DisableNSGroup1Ints(void);

unsigned int R_GIC_GetICC_MCTLR(void);

void R_GIC_SetICC_MCTLR(unsigned int value);

unsigned int R_GIC_GetICC_CTLR(void);

void R_GIC_SetICC_MCTLR(unsigned int value);

unsigned int R_GIC_ReadIntAck(void);

void R_GIC_WriteEOI(unsigned int ID);

void R_GIC_WriteDIR(unsigned int ID);

unsigned int R_GIC_ReadAliasedIntAck(void);

void R_GIC_WriteAliasedEOI(unsigned int ID);

/**
 * Set interrupt priority mask. 
 * Only interrupts with higher priority than the value in this register are signaled to the core 
 * Lower values have higher priority.
 *  
 * @param[in] priority          - priority mask value [0:31]
 */
#define ICC_PMR_PRIORITY_BIT_MASK 0x1F
#define ICC_PMR_PRIORITY_BIT_POSITION 3
void R_GIC_SetPriorityMask(unsigned int priority);

/**
 * @brief Get interrupt priority mask.
 *
 * @return Priority mask value.  
 */
unsigned int R_GIC_GetPriorityMask();

unsigned int R_GIC_GetBinaryPoint(void);

void R_GIC_SetBinaryPoint(unsigned int ID);

unsigned int R_GIC_GetAliasedBinaryPoint(void);

void R_GIC_SetAliasedBinaryPoint(unsigned int ID);

uint32_t R_GIC_GetRunningPriority(void);

void R_GIC_SendGroup0SGI(unsigned int ID, unsigned int mode, unsigned target_list);

void R_GIC_SendGroup1SGI(unsigned int ID, unsigned int mode, unsigned target_list);

void R_GIC_sendOtherGroup1SGI(unsigned int ID, unsigned int mode, unsigned target_list);

void R_GIC_SetICH_HCR(unsigned int value);

unsigned int R_GIC_GetICH_HCR(void);

void R_GIC_SetICH_AP0R0(unsigned int value);

unsigned int R_GIC_GetICH_AP0R0(void);

void R_GIC_SetICH_AP1R0(unsigned int value);

unsigned int R_GIC_GetICH_AP1R0(void);

void R_GIC_SetICH_LR0(unsigned int value);

unsigned int R_GIC_GetICH_LR0(void);

void R_GIC_SetICH_LRC0(unsigned int value);

unsigned int R_GIC_GetICH_LRC0(void);

void R_GIC_SetICH_LR1(unsigned int value);

unsigned int R_GIC_GetICH_LR1(void);

void R_GIC_SetICH_LRC1(unsigned int value);

unsigned int R_GIC_GetICH_LRC1(void);

void R_GIC_SetICH_LR2(unsigned int value);

unsigned int R_GIC_GetICH_LR2(void);

void R_GIC_SetICH_LRC2(unsigned int value);

unsigned int R_GIC_GetICH_LRC2(void);

void R_GIC_SetICH_LR3(unsigned int value);

unsigned int R_GIC_GetICH_LR3(void);

void R_GIC_SetICH_LRC3(unsigned int value);

unsigned int R_GIC_GetICH_LRC3(void);

/* C source code */
/* Distributor Interface Functions */
/**
 * @brief Sets the base addresses for the GIC Distributor and Redistributor.
 * @param dist Pointer to the Distributor base address.
 * @param rdist Pointer to the Redistributor base address.
 */
int R_GIC_SetAddr(void* dist, void* rdist);

/**
 * @brief Enables and configures the GIC Distributor Interface.
 * @retval Status of the operation.
 */
uint32_t R_GIC_Enable(void);

/* ------------------------------------------------------------
 * Redistributor Functions
 * ------------------------------------------------------------ */

/**
 * @brief Retrieves the Redistributor ID based on the affinity value.
 * @param affinity Affinity value.
 * @retval Redistributor ID.
 */
uint32_t R_GIC_GetRedistID(uint32_t affinity);

/**
 * @brief Wakes up a specific Redistributor.
 * @param rd Redistributor ID.
 * @retval Status of the operation.
 */
uint32_t R_GIC_WakeUpRedist(uint32_t rd);

/**
 * @brief Enables a specific interrupt at the Redistributor level.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @retval Status of the operation.
 */
uint32_t R_GIC_EnableInt(uint32_t ID, uint32_t rd);

/**
 * @brief Disables a specific interrupt at the Redistributor level.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @retval Status of the operation.
 */
uint32_t R_GIC_DisableInt(uint32_t ID, uint32_t rd);

/**
 * @brief Sets the priority of a specific interrupt.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @param priority Priority level.
 * @retval Status of the operation.
 */
uint32_t R_GIC_SetIntPriority(uint32_t ID, uint32_t rd, uint8_t priority);

/**
 * @brief Sets the type of a specific interrupt.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @param type Interrupt type.
 * @retval Status of the operation.
 */
uint32_t R_GIC_SetIntType(uint32_t ID, uint32_t rd, uint32_t type);

/**
 * @brief Sets the security group of a specific interrupt.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @param security Security group (Group 0 or Group 1).
 * @retval Status of the operation.
 */
uint32_t R_GIC_SetIntGroup(uint32_t ID, uint32_t rd, uint32_t security);

/**
 * @brief Sets the routing mode and target processor affinity for an interrupt.
 * @param ID Interrupt ID.
 * @param mode Routing mode.
 * @param affinity Target processor affinity.
 * @retval Status of the operation.
 */
uint32_t R_GIC_SetIntRoute(uint32_t ID, uint32_t mode, uint32_t affinity);

/**
 * @brief Sets an interrupt as pending.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @retval Status of the operation.
 */
uint32_t R_GIC_SetIntPending(uint32_t ID, uint32_t rd);

/**
 * @brief Clears the pending state of an interrupt.
 * @param ID Interrupt ID.
 * @param rd Redistributor ID.
 * @retval Status of the operation.
 */
uint32_t R_GIC_ClearIntPending(uint32_t ID, uint32_t rd);

#ifdef __cplusplus
}
#endif 

#endif /* R_GIC_H_ */
