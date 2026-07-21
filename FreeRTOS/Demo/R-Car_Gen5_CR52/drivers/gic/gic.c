/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
#include "gic.h"
#include "cmsis_rcar_gen5.h"

GICD_Type*       gic_dist;
GICR_Type*      gic_rdist;

static uint8_t gic_max_rd = 4;

void R_GIC_SetICC_SRE(unsigned int value)
{
__set_CP(15, 0, value, 12, 12, 5); // Write value to ICC_SRE

__ISB();
}

uint32_t R_GIC_GetICC_SRE(void)
{
uint32_t value;

__get_CP(15, 0, value, 12, 12, 5); // Read ICC_SRE into value

return value;
}

void R_GIC_EnableGroup0Ints(void) 
{
    uint32_t ICC_IGRPEN0 = 1;
   
   __set_CP(15, 0, ICC_IGRPEN0, 12, 12, 6); // Write value to ICC_IGRPEN0

   __ISB();
}

void R_GIC_DisableGroup0Ints(void)
{
    uint32_t value = 0;

    __set_CP(15, 0, value, 12, 12, 6); // Write value to ICC_IGRPEN0

    __ISB();
}

void R_GIC_EnableGroup1Ints(void)
{
    uint32_t value = 1;

    __set_CP(15, 0, value, 12, 12, 7); // Write value to ICC_IGRPEN1

    __ISB();
}

void R_GIC_DisableGroup1Ints(void)
{
    uint32_t ICC_IGRPEN1 = 0;

    __set_CP(15, 0, ICC_IGRPEN1, 12, 12, 7); // Write value to ICC_IGRPEN1

    __ISB();
}

unsigned int R_GIC_GetICC_CTLR(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 12, 4); // Read ICC_CTLR into value

    return value;
}

void R_GIC_SetICC_MCTLR(unsigned int value)
{
    __set_CP(15, 0, value, 12, 12, 4); // Write value to ICC_CTLR

    __ISB();
}

unsigned int R_GIC_ReadIntAck(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 8, 0); // Read ICC_IAR0 into value

    return value;
}

void R_GIC_WriteEOI(unsigned int ID)
{
    __set_CP(15, 0, ID, 12, 8, 1); // Write value to ICC_EOIR0

    __ISB();
}

void R_GIC_WriteDIR(unsigned int ID)
{
    __set_CP(15, 0, ID, 12, 11, 1); // Write value to ICC_DIR

    __ISB();
}

unsigned int R_GIC_ReadAliasedIntAck(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 12, 0); // Read ICC_IAR1 into value

    return value;
}

void R_GIC_WriteAliasedEOI(unsigned int ID)
{
    __set_CP(15, 0, ID, 12, 12, 1); // Write value to ICC_EOIR1

    __DSB();
}

unsigned int R_GIC_GetPriorityMask()
{
    uint32_t priority_mask;

    __get_CP(15, 0, priority_mask, 4, 6, 0); // Read ICC_PMR into priority_mask

    return priority_mask ;
}

void R_GIC_SetPriorityMask(unsigned int ID)
{
    __set_CP(15, 0, ID, 4, 6, 0); // Write ID to ICC_PMR
}

unsigned int R_GIC_GetBinaryPoint(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 8, 3); // Read ICC_BPR0 into value

    return value;
}

void R_GIC_SetBinaryPoint(unsigned int ID)
{
    __set_CP(15, 0, ID, 12, 8, 3); // Write ID to ICC_BPR0

    __ISB();
}

unsigned int R_GIC_GetAliasedBinaryPoint(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 12, 3); // Read ICC_BPR1 into value

    return value;
}

void R_GIC_SetAliasedBinaryPoint(unsigned int ID)
{
    __set_CP(15, 0, ID, 12, 12, 3); // Write ID to ICC_BPR1

    __ISB();
}

uint32_t R_GIC_GetRunningPriority(void)
{
    uint32_t value;

    __get_CP(15, 0, value, 12, 11, 3); // Read ICC_RPR into value

    return value;
}

void R_GIC_SendGroup0SGI(unsigned int ID, unsigned int mode, unsigned target_list)
{
    uint64_t value;

    value = (ID | target_list) | (uint64_t)mode << 32;

    __set_CP64(15, 2, value, 12); // Write value_L to ICC_SGI0R[31:0] and value_H to ICC_SGI0R[63:32]
}

void R_GIC_SendGroup1SGI(unsigned int ID, unsigned int mode, unsigned target_list)
{
    uint64_t value;

    value = (ID | target_list) | (uint64_t)mode << 32;

    __set_CP64(15, 0, value, 12); // Write value_L to ICC_SGI1R[31:0] and value_H to ICC_SGI1R[63:32]
}

void R_GIC_sendOtherGroup1SGI(unsigned int ID, unsigned int mode, unsigned target_list)
{
    uint64_t value;

    value = (ID | target_list) | (uint64_t)mode << 32;

    __set_CP64(15, 1, value, 12); // Write value_L to ICC_ASGI1R[31:0] and value_H to ICC_ASGI1R[63:32]
}

int R_GIC_SetAddr(void* dist, void* rdist) {
    uint32_t index = 0;

    gic_dist = (GICD_Type *)dist;
    gic_rdist = (GICR_Type *)rdist;

    if ((gic_dist == NULL) || (gic_rdist == NULL)) {
      return -1;
    }

    while((gic_rdist[index].target_ctrl.GICR_TYPER[0] & (1U<<4)) == 0)
    {
      index++;
    }

    gic_max_rd = index;
    return 0;
}

uint32_t R_GIC_Enable(void) {
    uint32_t result = 1;  // Success indicator

    if (gic_dist == NULL) {
        return 1;
    }

    gic_dist->GICD_CTLR = 0x13; // Enable group 0, group 1 and affinity.

    while ((gic_dist->GICD_CTLR & 0x80000000) != 0x0) {} // Wait for RWP clear.

    return result;
}

/* ------------------------------------------------------------
 * Redistributor Functions
 * ------------------------------------------------------------ */
uint32_t R_GIC_GetRedistID(uint32_t affinity) {
    uint32_t index = 0;

    if (gic_rdist == 0) {
      return 0xFFFFFFFF;
    }

    do
    {
        if ((gic_rdist[index].target_ctrl.GICR_TYPER[1] & 0x0F) == (affinity & 0xFF)) {
         return index;
        }

      index++;
    }
    while(index <= gic_max_rd);

    return 0xFFFFFFFF;
}

uint32_t R_GIC_WakeUpRedist(uint32_t rd) {
    if (gic_rdist == 0) {
        return 1;
    }

    gic_rdist[rd].target_ctrl.GICR_WAKER &= 0xFFFFFFFD;

    while ((gic_rdist[rd].target_ctrl.GICR_WAKER&0x4) == 0x4) {}

    return 0;
}

uint32_t R_GIC_EnableInt(uint32_t ID, uint32_t rd) {
    uint32_t result = 1;

    if (gic_rdist==0) {
      return 1;
    }

    if (ID < 31) {
       gic_rdist[rd].sgi_ppi.GICR_ISENABLER[0] = ((uint32_t)1 << ID);
    } else if (ID < 1020) {
      GIC_EnableIRQ(gic_dist, ID);
    }

    return result;
}

uint32_t R_GIC_DisableInt(uint32_t ID, uint32_t rd) {
    if (ID < 31)
    {
      gic_rdist[rd].sgi_ppi.GICR_ICENABLER[0] = ((uint32_t)1 << ID);
    }
    else if (ID < 1020)
    {
      GIC_DisableIRQ(gic_dist, ID);
    }

    return 0;
}

uint32_t R_GIC_SetIntPriority(uint32_t ID, uint32_t rd, uint8_t priority) {

    if (gic_rdist==0)
    {
      return 1;
    }

    if (ID < 32)
    {
        if (rd > gic_max_rd) {
         return 1;
        }

      gic_rdist[rd].sgi_ppi.GICR_IPRIORITYR[ID] = priority;
    }
    else if (ID < 1020)
    {
      gic_dist->GICD_IPRIORITYR[ID] = priority;
    }

    return 0;
}

uint32_t R_GIC_SetIntType(uint32_t ID, uint32_t rd, uint32_t type) {
    if (gic_rdist==0) {
        return 1;
    }

    if (ID < 31)
    {
      return 1;
    }
    else if (ID < 1020)
    {
      GIC_SetConfiguration(gic_dist, ID, type);
    }
    else
    {
      return 1;
    }
    
    return 0;
}

uint32_t R_GIC_SetIntGroup(uint32_t ID, uint32_t rd, uint32_t security) {
    // Just support group 1 non secure
    if (gic_rdist==0) {
        return 1;
    }

    if (ID < 31)
    {
      gic_rdist[rd].sgi_ppi.GICR_IGROUPR[0] |= ((uint32_t)1 << (ID % 32U));
      gic_rdist[rd].sgi_ppi.GICR_IGRPMODR[0] &= ((uint32_t)1 << (ID % 32U));
    }
    else if (ID < 1020)
    {
      gic_dist->GICD_IGROUPR[ID / 32U] |= ((uint32_t)1 << (ID % 32U));
      gic_dist->GICD_IGRPMODR[ID / 2U] &= ((uint32_t)1 << (ID % 32U));
    }

  return 0;
}

uint32_t R_GIC_SetIntRoute(uint32_t ID, uint32_t mode, uint32_t affinity) {
    if (gic_rdist==0) {
        return 0xFFFFFFFF;
    }

    if (ID >= 32)
    {
        uint64_t value  = (uint64_t)(affinity & 0x00FFFFFF) | (uint64_t) mode;
        gic_dist->GICD_IROUTER[ID] = value;
    }

  return 0;
}

uint32_t R_GIC_SetIntPending(uint32_t ID, uint32_t rd) {
    uint32_t bank;

    if (gic_rdist==0) {
      return 0xFFFFFFFF;
    }

    if (ID < 31) {
      gic_rdist[rd].sgi_ppi.GICR_ISPENDR[0] = ((uint32_t)1 << (ID % 32U));
    }
    else if (ID < 1020)
    {
      GIC_SetPendingIRQ(gic_dist, ID);
      GIC_ClearPendingIRQ(gic_dist, ID);
    }
    else {
      return 1;
    }

    return 0;

}

uint32_t R_GIC_ClearIntPending(uint32_t ID, uint32_t rd) {
    if (gic_rdist==0) {
      return 0xFFFFFFFF;
    }

    if (ID < 31)
    {
      gic_rdist[rd].sgi_ppi.GICR_ICPENDR[0] = ((uint32_t)1 << (ID % 32U));
    }
    else if (ID < 1020)
    {
      GIC_SetPendingIRQ(gic_dist, ID);
    }
    else {
      return 1;
    }

    return 0;
}
