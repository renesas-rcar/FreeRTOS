/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/

#ifndef R_TAUD_REG_H
#define R_TAUD_REG_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Definition of TAUD base address */
#define DRV_REG_BASE_ADDR_TAUD0             (0xC1392000U)      /* TAUD0 */
#define DRV_REG_BASE_ADDR_TAUD1             (0xC1393000U)      /* TAUD1 */
#define DRV_REG_BASE_ADDR_TAUD(unit)        (((uint32_t)(unit) == 1U) ? DRV_REG_BASE_ADDR_TAUD1 : DRV_REG_BASE_ADDR_TAUD0)      /* TAUD1 */


#define DRV_REG_OFFSET_CH_TAUD              (0x00000004u)

/* Definition of TAUD reg offset */
#define DRV_REG_OFFSET_TAUD_TPS             (0x00000240u)
#define DRV_REG_OFFSET_TAUD_BRS             (0x00000244U)
#define DRV_REG_OFFSET_TAUD_CDR             (0x00000000u)
#define DRV_REG_OFFSET_TAUD_CNT             (0x00000080u)
#define DRV_REG_OFFSET_TAUD_CMOR            (0x00000200u)
#define DRV_REG_OFFSET_TAUD_CMUR            (0x000000C0u)
#define DRV_REG_OFFSET_TAUD_CSR             (0x00000140u)
#define DRV_REG_OFFSET_TAUD_CSC             (0x00000180u)
#define DRV_REG_OFFSET_TAUD_TS              (0x000001C4u)
#define DRV_REG_OFFSET_TAUD_TE              (0x000001C0u)
#define DRV_REG_OFFSET_TAUD_TT              (0x000001C8u)
#define DRV_REG_OFFSET_TAUD_RDE             (0x00000260u)
#define DRV_REG_OFFSET_TAUD_RDS             (0x00000268u)
#define DRV_REG_OFFSET_TAUD_RDM             (0x00000264u)
#define DRV_REG_OFFSET_TAUD_RDC             (0x0000026Cu)
#define DRV_REG_OFFSET_TAUD_RDT             (0x00000044u)
#define DRV_REG_OFFSET_TAUD_RSF             (0x00000048u)
#define DRV_REG_OFFSET_TAUD_TOE             (0x0000005Cu)
#define DRV_REG_OFFSET_TAUD_TO              (0x00000058u)
#define DRV_REG_OFFSET_TAUD_TOM             (0x00000248u)
#define DRV_REG_OFFSET_TAUD_TOC             (0x0000024Cu)
#define DRV_REG_OFFSET_TAUD_TOL             (0x00000040u)
#define DRV_REG_OFFSET_TAUD_TDE             (0x00000250u)
#define DRV_REG_OFFSET_TAUD_TDM             (0x00000254u)
#define DRV_REG_OFFSET_TAUD_TDL             (0x00000054u)
#define DRV_REG_OFFSET_TAUD_TRO             (0x00000004u)
#define DRV_REG_OFFSET_TAUD_TRE             (0x00000258u)
#define DRV_REG_OFFSET_TAUD_TRC             (0x0000025Cu)
#define DRV_REG_OFFSET_TAUD_TME             (0x00000050u)

/* Definition of TAUD reg address */
#define DRV_REG_ADDR_TAUD_TPS(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TPS)
#define DRV_REG_ADDR_TAUD_BRS(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_BRS)
#define DRV_REG_ADDR_TAUD_CDR(unit, ch)     (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CDR  + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_CNT(unit, ch)     (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CNT  + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_CMOR(unit, ch)    (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CMOR + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_CMUR(unit, ch)    (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CMUR + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_CSR(unit, ch)     (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CSR  + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_CSC(unit, ch)     (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_CSC  + (DRV_REG_OFFSET_CH_TAUD * (uint32_t)(ch)))
#define DRV_REG_ADDR_TAUD_TS(unit)          (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TS)
#define DRV_REG_ADDR_TAUD_TE(unit)          (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TE)
#define DRV_REG_ADDR_TAUD_TT(unit)          (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TT)
#define DRV_REG_ADDR_TAUD_TOE(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TOE)
#define DRV_REG_ADDR_TAUD_TO(unit)          (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TO)
#define DRV_REG_ADDR_TAUD_TOM(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TOM)
#define DRV_REG_ADDR_TAUD_TOC(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TOC)
#define DRV_REG_ADDR_TAUD_TOL(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TOL)
#define DRV_REG_ADDR_TAUD_TDE(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TDE)
#define DRV_REG_ADDR_TAUD_TDM(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TDM)
#define DRV_REG_ADDR_TAUD_TDL(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TDL)
#define DRV_REG_ADDR_TAUD_TRO(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TRO)
#define DRV_REG_ADDR_TAUD_TRE(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TRE)
#define DRV_REG_ADDR_TAUD_TRC(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TRC)
#define DRV_REG_ADDR_TAUD_TME(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_TME)
#define DRV_REG_ADDR_TAUD_RDE(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RDE)
#define DRV_REG_ADDR_TAUD_RDM(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RDM)
#define DRV_REG_ADDR_TAUD_RDS(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RDS)
#define DRV_REG_ADDR_TAUD_RDC(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RDC)
#define DRV_REG_ADDR_TAUD_RDT(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RDT)
#define DRV_REG_ADDR_TAUD_RSF(unit)         (uintptr_t)(DRV_REG_BASE_ADDR_TAUD(unit) + DRV_REG_OFFSET_TAUD_RSF)

#define TAUD_CMOR_STS_TRIGGER_COUNTER_USING_SOFTWARE                0B000
#define TAUD_CMOR_STS_TRIGGER_COUNTER_USING_MASTERCHANNEL           0B100

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef union u_reg16_TPS 
{
    struct 
    {
        uint16_t PRS0   : 4;
        uint16_t PRS1   : 4;
        uint16_t PRS2   : 4;
        uint16_t PRS3   : 4;
    } BIT;

    uint16_t INT;
} reg16_TPS_t;

typedef struct st_reg8_BRS
{
    uint8_t INT; 
} reg8_BRS_t;

typedef struct st_reg16_CDR
{
    uint16_t INT;
} reg16_CDR_t;

typedef struct st_reg16_CNT
{
    uint16_t INT;
} reg16_CNT_t;

typedef union u_reg16_CMOR
{
    struct 
    {
        uint16_t MD0    : 1;
        uint16_t MD     : 4;
        uint16_t        : 1;
        uint16_t COS    : 2;
        uint16_t STS    : 3;
        uint16_t MAS    : 1;
        uint16_t CCS    : 2;
        uint16_t CKS    : 2;   
    } BIT;
    
    uint16_t INT;
} reg16_CMOR_t;

typedef union u_reg8_CMUR
{
    struct 
    {
        uint8_t TIS     :2;
        uint8_t         :6;
    } BIT;
    
    uint8_t INT;
} reg8_CMUR_t;

typedef union u_reg8_CSR
{
    struct 
    {
        uint8_t OVF     : 1;
        uint8_t CSF     : 1;
        uint8_t         : 6;
    } BIT;
    
    uint8_t INT;
} reg8_CSR_t;

typedef union u_reg8_CSC
{
    struct 
    {
        uint8_t CLOV    : 1;
        uint8_t         : 7;
    } BIT;
    
    uint8_t INT;
} reg8_CSC_t;

typedef union u_reg16_TS
{
    struct 
    {
        uint16_t TS00   : 1;
        uint16_t TS01   : 1;
        uint16_t TS02   : 1;
        uint16_t TS03   : 1;
        uint16_t TS04   : 1;
        uint16_t TS05   : 1;
        uint16_t TS06   : 1;
        uint16_t TS07   : 1;
        uint16_t TS08   : 1;
        uint16_t TS09   : 1;
        uint16_t TS10   : 1;
        uint16_t TS11   : 1;
        uint16_t TS12   : 1;
        uint16_t TS13   : 1;
        uint16_t TS14   : 1;
        uint16_t TS15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TS_t;

typedef union u_reg16_TE
{
    struct 
    {
        uint16_t TE00   : 1;
        uint16_t TE01   : 1;
        uint16_t TE02   : 1;
        uint16_t TE03   : 1;
        uint16_t TE04   : 1;
        uint16_t TE05   : 1;
        uint16_t TE06   : 1;
        uint16_t TE07   : 1;
        uint16_t TE08   : 1;
        uint16_t TE09   : 1;
        uint16_t TE10   : 1;
        uint16_t TE11   : 1;
        uint16_t TE12   : 1;
        uint16_t TE13   : 1;
        uint16_t TE14   : 1;
        uint16_t TE15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TE_t;

typedef union u_reg16_TT
{
    struct 
    {
        uint16_t TT00   : 1;
        uint16_t TT01   : 1;
        uint16_t TT02   : 1;
        uint16_t TT03   : 1;
        uint16_t TT04   : 1;
        uint16_t TT05   : 1;
        uint16_t TT06   : 1;
        uint16_t TT07   : 1;
        uint16_t TT08   : 1;
        uint16_t TT09   : 1;
        uint16_t TT10   : 1;
        uint16_t TT11   : 1;
        uint16_t TT12   : 1;
        uint16_t TT13   : 1;
        uint16_t TT14   : 1;
        uint16_t TT15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TT_t;

typedef union u_reg16_RDE
{
    struct 
    {
        uint16_t RDE00   : 1;
        uint16_t RDE01   : 1;
        uint16_t RDE02   : 1;
        uint16_t RDE03   : 1;
        uint16_t RDE04   : 1;
        uint16_t RDE05   : 1;
        uint16_t RDE06   : 1;
        uint16_t RDE07   : 1;
        uint16_t RDE08   : 1;
        uint16_t RDE09   : 1;
        uint16_t RDE10   : 1;
        uint16_t RDE11   : 1;
        uint16_t RDE12   : 1;
        uint16_t RDE13   : 1;
        uint16_t RDE14   : 1;
        uint16_t RDE15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RDE_t;

typedef union u_reg16_RDS
{
    struct 
    {
        uint16_t RDS00   : 1;
        uint16_t RDS01   : 1;
        uint16_t RDS02   : 1;
        uint16_t RDS03   : 1;
        uint16_t RDS04   : 1;
        uint16_t RDS05   : 1;
        uint16_t RDS06   : 1;
        uint16_t RDS07   : 1;
        uint16_t RDS08   : 1;
        uint16_t RDS09   : 1;
        uint16_t RDS10   : 1;
        uint16_t RDS11   : 1;
        uint16_t RDS12   : 1;
        uint16_t RDS13   : 1;
        uint16_t RDS14   : 1;
        uint16_t RDS15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RDS_t;

typedef union u_reg16_RDM
{
    struct 
    {
        uint16_t RDM00   : 1;
        uint16_t RDM01   : 1;
        uint16_t RDM02   : 1;
        uint16_t RDM03   : 1;
        uint16_t RDM04   : 1;
        uint16_t RDM05   : 1;
        uint16_t RDM06   : 1;
        uint16_t RDM07   : 1;
        uint16_t RDM08   : 1;
        uint16_t RDM09   : 1;
        uint16_t RDM10   : 1;
        uint16_t RDM11   : 1;
        uint16_t RDM12   : 1;
        uint16_t RDM13   : 1;
        uint16_t RDM14   : 1;
        uint16_t RDM15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RDM_t;

typedef union u_reg16_RDC
{
    struct 
    {
        uint16_t RDC00   : 1;
        uint16_t RDC01   : 1;
        uint16_t RDC02   : 1;
        uint16_t RDC03   : 1;
        uint16_t RDC04   : 1;
        uint16_t RDC05   : 1;
        uint16_t RDC06   : 1;
        uint16_t RDC07   : 1;
        uint16_t RDC08   : 1;
        uint16_t RDC09   : 1;
        uint16_t RDC10   : 1;
        uint16_t RDC11   : 1;
        uint16_t RDC12   : 1;
        uint16_t RDC13   : 1;
        uint16_t RDC14   : 1;
        uint16_t RDC15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RDC_t;

typedef union u_reg16_RDT
{
    struct 
    {
        uint16_t RDT00   : 1;
        uint16_t RDT01   : 1;
        uint16_t RDT02   : 1;
        uint16_t RDT03   : 1;
        uint16_t RDT04   : 1;
        uint16_t RDT05   : 1;
        uint16_t RDT06   : 1;
        uint16_t RDT07   : 1;
        uint16_t RDT08   : 1;
        uint16_t RDT09   : 1;
        uint16_t RDT10   : 1;
        uint16_t RDT11   : 1;
        uint16_t RDT12   : 1;
        uint16_t RDT13   : 1;
        uint16_t RDT14   : 1;
        uint16_t RDT15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RDT_t;

typedef union u_reg16_RSF
{
    struct 
    {
        uint16_t RSF00   : 1;
        uint16_t RSF01   : 1;
        uint16_t RSF02   : 1;
        uint16_t RSF03   : 1;
        uint16_t RSF04   : 1;
        uint16_t RSF05   : 1;
        uint16_t RSF06   : 1;
        uint16_t RSF07   : 1;
        uint16_t RSF08   : 1;
        uint16_t RSF09   : 1;
        uint16_t RSF10   : 1;
        uint16_t RSF11   : 1;
        uint16_t RSF12   : 1;
        uint16_t RSF13   : 1;
        uint16_t RSF14   : 1;
        uint16_t RSF15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_RSF_t;

typedef union u_reg16_TOE
{
    struct 
    {
        uint16_t TOE00   : 1;
        uint16_t TOE01   : 1;
        uint16_t TOE02   : 1;
        uint16_t TOE03   : 1;
        uint16_t TOE04   : 1;
        uint16_t TOE05   : 1;
        uint16_t TOE06   : 1;
        uint16_t TOE07   : 1;
        uint16_t TOE08   : 1;
        uint16_t TOE09   : 1;
        uint16_t TOE10   : 1;
        uint16_t TOE11   : 1;
        uint16_t TOE12   : 1;
        uint16_t TOE13   : 1;
        uint16_t TOE14   : 1;
        uint16_t TOE15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TOE_t;

typedef union u_reg16_TO
{
    struct 
    {
        uint16_t TO00   : 1;
        uint16_t TO01   : 1;
        uint16_t TO02   : 1;
        uint16_t TO03   : 1;
        uint16_t TO04   : 1;
        uint16_t TO05   : 1;
        uint16_t TO06   : 1;
        uint16_t TO07   : 1;
        uint16_t TO08   : 1;
        uint16_t TO09   : 1;
        uint16_t TO10   : 1;
        uint16_t TO11   : 1;
        uint16_t TO12   : 1;
        uint16_t TO13   : 1;
        uint16_t TO14   : 1;
        uint16_t TO15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TO_t;

typedef union u_reg16_TOM
{
    struct 
    {
        uint16_t TOM00   : 1;
        uint16_t TOM01   : 1;
        uint16_t TOM02   : 1;
        uint16_t TOM03   : 1;
        uint16_t TOM04   : 1;
        uint16_t TOM05   : 1;
        uint16_t TOM06   : 1;
        uint16_t TOM07   : 1;
        uint16_t TOM08   : 1;
        uint16_t TOM09   : 1;
        uint16_t TOM10   : 1;
        uint16_t TOM11   : 1;
        uint16_t TOM12   : 1;
        uint16_t TOM13   : 1;
        uint16_t TOM14   : 1;
        uint16_t TOM15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TOM_t;

typedef union u_reg16_TOC
{
    struct 
    {
        uint16_t TOC00   : 1;
        uint16_t TOC01   : 1;
        uint16_t TOC02   : 1;
        uint16_t TOC03   : 1;
        uint16_t TOC04   : 1;
        uint16_t TOC05   : 1;
        uint16_t TOC06   : 1;
        uint16_t TOC07   : 1;
        uint16_t TOC08   : 1;
        uint16_t TOC09   : 1;
        uint16_t TOC10   : 1;
        uint16_t TOC11   : 1;
        uint16_t TOC12   : 1;
        uint16_t TOC13   : 1;
        uint16_t TOC14   : 1;
        uint16_t TOC15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TOC_t;

typedef union u_reg16_TOL
{
    struct 
    {
        uint16_t TOL00   : 1;
        uint16_t TOL01   : 1;
        uint16_t TOL02   : 1;
        uint16_t TOL03   : 1;
        uint16_t TOL04   : 1;
        uint16_t TOL05   : 1;
        uint16_t TOL06   : 1;
        uint16_t TOL07   : 1;
        uint16_t TOL08   : 1;
        uint16_t TOL09   : 1;
        uint16_t TOL10   : 1;
        uint16_t TOL11   : 1;
        uint16_t TOL12   : 1;
        uint16_t TOL13   : 1;
        uint16_t TOL14   : 1;
        uint16_t TOL15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TOL_t;

typedef union u_reg16_TDE
{
    struct 
    {
        uint16_t TDE00   : 1;
        uint16_t TDE01   : 1;
        uint16_t TDE02   : 1;
        uint16_t TDE03   : 1;
        uint16_t TDE04   : 1;
        uint16_t TDE05   : 1;
        uint16_t TDE06   : 1;
        uint16_t TDE07   : 1;
        uint16_t TDE08   : 1;
        uint16_t TDE09   : 1;
        uint16_t TDE10   : 1;
        uint16_t TDE11   : 1;
        uint16_t TDE12   : 1;
        uint16_t TDE13   : 1;
        uint16_t TDE14   : 1;
        uint16_t TDE15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TDE_t;

typedef union u_reg16_TDM
{
    struct 
    {
        uint16_t TDM00   : 1;
        uint16_t TDM01   : 1;
        uint16_t TDM02   : 1;
        uint16_t TDM03   : 1;
        uint16_t TDM04   : 1;
        uint16_t TDM05   : 1;
        uint16_t TDM06   : 1;
        uint16_t TDM07   : 1;
        uint16_t TDM08   : 1;
        uint16_t TDM09   : 1;
        uint16_t TDM10   : 1;
        uint16_t TDM11   : 1;
        uint16_t TDM12   : 1;
        uint16_t TDM13   : 1;
        uint16_t TDM14   : 1;
        uint16_t TDM15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TDM_t;

typedef union u_reg16_TDL
{
    struct 
    {
        uint16_t TDL00   : 1;
        uint16_t TDL01   : 1;
        uint16_t TDL02   : 1;
        uint16_t TDL03   : 1;
        uint16_t TDL04   : 1;
        uint16_t TDL05   : 1;
        uint16_t TDL06   : 1;
        uint16_t TDL07   : 1;
        uint16_t TDL08   : 1;
        uint16_t TDL09   : 1;
        uint16_t TDL10   : 1;
        uint16_t TDL11   : 1;
        uint16_t TDL12   : 1;
        uint16_t TDL13   : 1;
        uint16_t TDL14   : 1;
        uint16_t TDL15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TDL_t;

typedef union u_reg16_TRE
{
    struct 
    {
        uint16_t TRE00   : 1;
        uint16_t TRE01   : 1;
        uint16_t TRE02   : 1;
        uint16_t TRE03   : 1;
        uint16_t TRE04   : 1;
        uint16_t TRE05   : 1;
        uint16_t TRE06   : 1;
        uint16_t TRE07   : 1;
        uint16_t TRE08   : 1;
        uint16_t TRE09   : 1;
        uint16_t TRE10   : 1;
        uint16_t TRE11   : 1;
        uint16_t TRE12   : 1;
        uint16_t TRE13   : 1;
        uint16_t TRE14   : 1;
        uint16_t TRE15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TRE_t;

typedef union u_reg16_TRC
{
    struct 
    {
        uint16_t TRC00   : 1;
        uint16_t TRC01   : 1;
        uint16_t TRC02   : 1;
        uint16_t TRC03   : 1;
        uint16_t TRC04   : 1;
        uint16_t TRC05   : 1;
        uint16_t TRC06   : 1;
        uint16_t TRC07   : 1;
        uint16_t TRC08   : 1;
        uint16_t TRC09   : 1;
        uint16_t TRC10   : 1;
        uint16_t TRC11   : 1;
        uint16_t TRC12   : 1;
        uint16_t TRC13   : 1;
        uint16_t TRC14   : 1;
        uint16_t TRC15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TRC_t;

typedef union u_reg16_TRO
{
    struct 
    {
        uint16_t TRO00   : 1;
        uint16_t TRO01   : 1;
        uint16_t TRO02   : 1;
        uint16_t TRO03   : 1;
        uint16_t TRO04   : 1;
        uint16_t TRO05   : 1;
        uint16_t TRO06   : 1;
        uint16_t TRO07   : 1;
        uint16_t TRO08   : 1;
        uint16_t TRO09   : 1;
        uint16_t TRO10   : 1;
        uint16_t TRO11   : 1;
        uint16_t TRO12   : 1;
        uint16_t TRO13   : 1;
        uint16_t TRO14   : 1;
        uint16_t TRO15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TRO_t;

typedef union u_reg16_TME
{
    struct 
    {
        uint16_t TME00   : 1;
        uint16_t TME01   : 1;
        uint16_t TME02   : 1;
        uint16_t TME03   : 1;
        uint16_t TME04   : 1;
        uint16_t TME05   : 1;
        uint16_t TME06   : 1;
        uint16_t TME07   : 1;
        uint16_t TME08   : 1;
        uint16_t TME09   : 1;
        uint16_t TME10   : 1;
        uint16_t TME11   : 1;
        uint16_t TME12   : 1;
        uint16_t TME13   : 1;
        uint16_t TME14   : 1;
        uint16_t TME15   : 1;
    } BIT;
    
    uint16_t INT;
} reg16_TME_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
uint16_t R_TAUD_RegWrite16(uintptr_t Addr, uint16_t Data);

uint16_t  R_TAUD_RegRead16(uintptr_t Addr);

uint8_t  R_TAUD_RegWrite8(uintptr_t Addr, uint8_t Data);

uint8_t  R_TAUD_RegRead8(uintptr_t Addr);

uint16_t R_TAUD_CH_Set(uintptr_t reg_addr, uint8_t ch);

uint16_t R_TAUD_CH_Clear(uintptr_t reg_addr, uint8_t ch);

#endif /* R_TAUD_REG_H */