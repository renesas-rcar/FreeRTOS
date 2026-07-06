/**************************************************************************//**
 * @file     core_cr52.h
 * @brief    CMSIS Cortex-R52 Core Peripheral Access Layer Header File
 * @date     31. August 2021
 ******************************************************************************/
/*
 * Copyright [2020-2025] Renesas Electronics Corporation and/or its affiliates. All Rights Reserved.
 *
 * This file is based on the "\CMSIS\Core\Include\core_armv8mml.h"
 *
 * Changes:
 * Renesas Electronics Corporation on 2021-08-31
 *    - Changed to be related to Cortex-R52 by
 * Renesas Electronics Corporation on 2025-01-17
 *    - Disabled unused functions and added some functions for Cortex-R52
 *    - Refer to "\CMSIS\Core\Include\core_ca.h" for GIC functions.
 */
/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
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

#if defined ( __ICCARM__ )
 #pragma system_include  /* treat file as system include file for MISRA check */
#endif

#ifndef __CORE_CR52_H_GENERIC
#define __CORE_CR52_H_GENERIC

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/**
  \page CMSIS_MISRA_Exceptions  MISRA-C:2004 Compliance Exceptions
  CMSIS violates the following MISRA-C:2004 rules:

   \li Required Rule 8.5, object/function definition in header file.<br>
     Function definitions in header files are used to allow 'inlining'.

   \li Required Rule 18.4, declaration of union type or object of union type: '{...}'.<br>
     Unions are used for effective representation of core registers.

   \li Advisory Rule 19.7, Function-like macro defined.<br>
     Function-like macros are used to allow more efficient code.
 */


/*******************************************************************************
 *                 CMSIS definitions
 ******************************************************************************/
/**
  \ingroup Cortex_R52
  @{
 */

#if defined ( __GNUC__ )
  #if defined (__VFP_FP__) && !defined(__SOFTFP__)
    #if defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)
      #define __FPU_USED       1U
      #define __FPU_D32        1U
    #else
      #error "Compiler generates FPU instructions for a device without an FPU (check __FPU_PRESENT)"
      #define __FPU_USED       0U
    #endif
  #else
    #define __FPU_USED         0U
  #endif

#elif defined ( __ICCARM__ )
  #if defined __ARMVFP__
    #if defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)
      #define __FPU_USED       1U
      #ifndef __ARMVFP_D16__
        #define __FPU_D32      1U
      #endif
    #else
      #error "Compiler generates FPU instructions for a device without an FPU (check __FPU_PRESENT)"
      #define __FPU_USED       0U
    #endif
  #else
    #define __FPU_USED         0U
  #endif
#endif

#include "cmsis_version.h"

/* CMSIS CR52 definitions */
#define __CR52_CMSIS_VERSION_MAIN  (__CM_CMSIS_VERSION_MAIN)                  /*!< \deprecated [31:16] CMSIS HAL main version */
#define __CR52_CMSIS_VERSION_SUB   ( __CM_CMSIS_VERSION_SUB)                  /*!< \deprecated [15:0]  CMSIS HAL sub version */
#define __CR52_CMSIS_VERSION       ((__CR52_CMSIS_VERSION_MAIN << 16U) | \
                                    __CR52_CMSIS_VERSION_SUB           )      /*!< \deprecated CMSIS HAL version number */

#define __CORTEX_R                 (52U)                                      /*!< Cortex-R Core                    */

#include "cmsis_compiler.h"               /* CMSIS compiler specific defines */


#ifdef __cplusplus
}
#endif

#endif /* __CORE_CR52_H_GENERIC */

#ifndef __CMSIS_GENERIC

#ifndef __CORE_CR52_H_DEPENDANT
#define __CORE_CR52_H_DEPENDANT

#ifdef __cplusplus
 extern "C" {
#endif


/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

/* CPSR Register Definitions */
#define CPSR_N_Pos                       31U                                    /* CPSR: N Position */
#define CPSR_N_Msk                       (1UL << CPSR_N_Pos)                    /* CPSR: N Mask */

#define CPSR_Z_Pos                       30U                                    /* CPSR: Z Position */
#define CPSR_Z_Msk                       (1UL << CPSR_Z_Pos)                    /* CPSR: Z Mask */

#define CPSR_C_Pos                       29U                                    /* CPSR: C Position */
#define CPSR_C_Msk                       (1UL << CPSR_C_Pos)                    /* CPSR: C Mask */

#define CPSR_V_Pos                       28U                                    /* CPSR: V Position */
#define CPSR_V_Msk                       (1UL << CPSR_V_Pos)                    /* CPSR: V Mask */

#define CPSR_Q_Pos                       27U                                    /* CPSR: Q Position */
#define CPSR_Q_Msk                       (1UL << CPSR_Q_Pos)                    /* CPSR: Q Mask */

#define CPSR_IT0_Pos                     25U                                    /* CPSR: IT0 Position */
#define CPSR_IT0_Msk                     (3UL << CPSR_IT0_Pos)                  /* CPSR: IT0 Mask */

#define CPSR_J_Pos                       24U                                    /* CPSR: J Position */
#define CPSR_J_Msk                       (1UL << CPSR_J_Pos)                    /* CPSR: J Mask */

#define CPSR_GE_Pos                      16U                                    /* CPSR: GE Position */
#define CPSR_GE_Msk                      (0xFUL << CPSR_GE_Pos)                 /* CPSR: GE Mask */

#define CPSR_IT1_Pos                     10U                                    /* CPSR: IT1 Position */
#define CPSR_IT1_Msk                     (0x3FUL << CPSR_IT1_Pos)               /* CPSR: IT1 Mask */

#define CPSR_E_Pos                       9U                                     /* CPSR: E Position */
#define CPSR_E_Msk                       (1UL << CPSR_E_Pos)                    /* CPSR: E Mask */

#define CPSR_A_Pos                       8U                                     /* CPSR: A Position */
#define CPSR_A_Msk                       (1UL << CPSR_A_Pos)                    /* CPSR: A Mask */

#define CPSR_I_Pos                       7U                                     /* CPSR: I Position */
#define CPSR_I_Msk                       (1UL << CPSR_I_Pos)                    /* CPSR: I Mask */

#define CPSR_F_Pos                       6U                                     /* CPSR: F Position */
#define CPSR_F_Msk                       (1UL << CPSR_F_Pos)                    /* CPSR: F Mask */

#define CPSR_T_Pos                       5U                                     /* CPSR: T Position */
#define CPSR_T_Msk                       (1UL << CPSR_T_Pos)                    /* CPSR: T Mask */

#define CPSR_M_Pos                       0U                                     /* CPSR: M Position */
#define CPSR_M_Msk                       (0x1FUL << CPSR_M_Pos)                 /* CPSR: M Mask */

#define CPSR_M_USR                       0x10U                                  /* CPSR: M User mode (PL0) */
#define CPSR_M_FIQ                       0x11U                                  /* CPSR: M Fast Interrupt mode (PL1) */
#define CPSR_M_IRQ                       0x12U                                  /* CPSR: M Interrupt mode (PL1) */
#define CPSR_M_SVC                       0x13U                                  /* CPSR: M Supervisor mode (PL1) */
#define CPSR_M_MON                       0x16U                                  /* CPSR: M Monitor mode (PL1) */
#define CPSR_M_ABT                       0x17U                                  /* CPSR: M Abort mode (PL1) */
#define CPSR_M_HYP                       0x1AU                                  /* CPSR: M Hypervisor mode (PL2) */
#define CPSR_M_UND                       0x1BU                                  /* CPSR: M Undefined mode (PL1) */
#define CPSR_M_SYS                       0x1FU                                  /* CPSR: M System mode (PL1) */

/* SCTLR Register Definitions */
#define SCTLR_IE_Pos                     31U                                    /* SCTLR: IE Position */
#define SCTLR_IE_Msk                     (1UL << SCTLR_IE_Pos)                  /* SCTLR: IE Mask */

#define SCTLR_TE_Pos                     30U                                    /* SCTLR: TE Position */
#define SCTLR_TE_Msk                     (1UL << SCTLR_TE_Pos)                  /* SCTLR: TE Mask */

#define SCTLR_AFE_Pos                    29U                                    /* SCTLR: AFE Position */
#define SCTLR_AFE_Msk                    (1UL << SCTLR_AFE_Pos)                 /* SCTLR: AFE Mask */

#define SCTLR_TRE_Pos                    28U                                    /* SCTLR: TRE Position */
#define SCTLR_TRE_Msk                    (1UL << SCTLR_TRE_Pos)                 /* SCTLR: TRE Mask */

#define SCTLR_NMFI_Pos                   27U                                    /* SCTLR: NMFI Position */
#define SCTLR_NMFI_Msk                   (1UL << SCTLR_NMFI_Pos)                /* SCTLR: NMFI Mask */

#define SCTLR_EE_Pos                     25U                                    /* SCTLR: EE Position */
#define SCTLR_EE_Msk                     (1UL << SCTLR_EE_Pos)                  /* SCTLR: EE Mask */

#define SCTLR_VE_Pos                     24U                                    /* SCTLR: VE Position */
#define SCTLR_VE_Msk                     (1UL << SCTLR_VE_Pos)                  /* SCTLR: VE Mask */

#define SCTLR_FI_Pos                     21U                                    /* SCTLR: FI Position */
#define SCTLR_FI_Msk                     (1UL << SCTLR_FI_Pos)                  /* SCTLR: FI Mask */

#define SCTLR_UWXN_Pos                   20U                                    /* SCTLR: UWXN Position */
#define SCTLR_UWXN_Msk                   (1UL << SCTLR_UWXN_Pos)                /* SCTLR: UWXN Mask */

#define SCTLR_DZ_Pos                     19U                                    /* SCTLR: DZ Position */
#define SCTLR_DZ_Msk                     (1UL << SCTLR_DZ_Pos)                  /* SCTLR: DZ Mask */

#define SCTLR_WXN_Pos                    19U                                    /* SCTLR: WXN Position */
#define SCTLR_WXN_Msk                    (1UL << SCTLR_WXN_Pos)                 /* SCTLR: WXN Mask */

#define SCTLR_NTWE_Pos                   18U                                    /* SCTLR: NTWE Position */
#define SCTLR_NTWE_Msk                   (1UL << SCTLR_NTWE_Pos)                /* SCTLR: NTWE Mask */

#define SCTLR_BR_Pos                     17U                                    /* SCTLR: BR Position */
#define SCTLR_BR_Msk                     (1UL << SCTLR_BR_Pos)                  /* SCTLR: BR Mask */

#define SCTLR_NTWI_Pos                   16U                                    /* SCTLR: NTWI Position */
#define SCTLR_NTWI_Msk                   (1UL << SCTLR_NTWI_Pos)                /* SCTLR: NTWI Mask */

#define SCTLR_RR_Pos                     14U                                    /* SCTLR: RR Position */
#define SCTLR_RR_Msk                     (1UL << SCTLR_RR_Pos)                  /* SCTLR: RR Mask */

#define SCTLR_V_Pos                      13U                                    /* SCTLR: V Position */
#define SCTLR_V_Msk                      (1UL << SCTLR_V_Pos)                   /* SCTLR: V Mask */

#define SCTLR_I_Pos                      12U                                    /* SCTLR: I Position */
#define SCTLR_I_Msk                      (1UL << SCTLR_I_Pos)                   /* SCTLR: I Mask */

#define SCTLR_Z_Pos                      11U                                    /* SCTLR: Z Position */
#define SCTLR_Z_Msk                      (1UL << SCTLR_Z_Pos)                   /* SCTLR: Z Mask */

#define SCTLR_SW_Pos                     10U                                    /* SCTLR: SW Position */
#define SCTLR_SW_Msk                     (1UL << SCTLR_SW_Pos)                  /* SCTLR: SW Mask */

#define SCTLR_SED_Pos                    8U                                     /* SCTLR: SED Position */
#define SCTLR_SED_Msk                    (1UL << SCTLR_SED_Pos)                 /* SCTLR: SED Mask */

#define SCTLR_ITD_Pos                    7U                                     /* SCTLR: ITD Position */
#define SCTLR_ITD_Msk                    (1UL << SCTLR_ITD_Pos)                 /* SCTLR: ITD Mask */

#define SCTLR_CP15BEN_Pos                5U                                     /* SCTLR: CP15BEN Position */
#define SCTLR_CP15BEN_Msk                (1UL << SCTLR_CP15BEN_Pos)             /* SCTLR: CP15BEN Mask */

#define SCTLR_C_Pos                      2U                                     /* SCTLR: C Position */
#define SCTLR_C_Msk                      (1UL << SCTLR_C_Pos)                   /* SCTLR: C Mask */

#define SCTLR_A_Pos                      1U                                     /* SCTLR: A Position */
#define SCTLR_A_Msk                      (1UL << SCTLR_A_Pos)                   /* SCTLR: A Mask */

#define SCTLR_M_Pos                      0U                                     /* SCTLR: M Position */
#define SCTLR_M_Msk                      (1UL << SCTLR_M_Pos)                   /* SCTLR: M Mask */

/* ACTLR Register Definition */
#define ACTLR_DICDI_Pos                  31U                                     /* ACTLR: DICDI Position */
#define ACTLR_DICDI_Msk                  (1UL << ACTLR_DICDI_Pos)                /* ACTLR: DICDI Mask */

#define ACTLR_DIB2DI_Pos                 30U                                     /* ACTLR: DIB2DI Position */
#define ACTLR_DIB2DI_Msk                 (1UL << ACTLR_DIB2DI_Pos)               /* ACTLR: DIB2DI Mask */

#define ACTLR_DIB1DI_Pos                 29U                                     /* ACTLR: DIB1DI Position */
#define ACTLR_DIB1DI_Msk                 (1UL << ACTLR_DIB1DI_Pos)               /* ACTLR: DIB1DI Mask */

#define ACTLR_DIADI_Pos                  28U                                     /* ACTLR: DIADI Position */
#define ACTLR_DIADI_Msk                  (1UL << ACTLR_DIADI_Pos)                /* ACTLR: DIADI Mask */

#define ACTLR_B1TCMPCEN_Pos              27U                                     /* ACTLR: B1TCMPCEN Position */
#define ACTLR_B1TCMPCEN_Msk              (1UL << ACTLR_B1TCMPCEN_Pos)            /* ACTLR: B1TCMPCEN Mask */

#define ACTLR_B0TCMPCEN_Pos              26U                                     /* ACTLR: B0TCMPCEN Position */
#define ACTLR_B0TCMPCEN_Msk              (1UL << ACTLR_B0TCMPCEN_Pos)            /* ACTLR: B0TCMPCEN Mask */

#define ACTLR_ATCMPCEN_Pos               25U                                     /* ACTLR: ATCMPCEN Position */
#define ACTLR_ATCMPCEN_Msk               (1UL << ACTLR_ATCMPCEN_Pos)             /* ACTLR: ATCMPCEN Mask */

#define ACTLR_AXISCEN_Pos                24U                                     /* ACTLR: AXISCEN Position */
#define ACTLR_AXISCEN_Msk                (1UL << ACTLR_AXISCEN_Pos)              /* ACTLR: AXISCEN Mask */

#define ACTLR_AXISCUEN_Pos               23U                                     /* ACTLR: AXISCUEN Position */
#define ACTLR_AXISCUEN_Msk               (1UL << ACTLR_AXISCUEN_Pos)             /* ACTLR: AXISCUEN Mask */

#define ACTLR_DILSM_Pos                  22U                                     /* ACTLR: DILSM Position */
#define ACTLR_DILSM_Msk                  (1UL << ACTLR_DILSM_Pos)                /* ACTLR: DILSM Mask */

#define ACTLR_DEOLP_Pos                  21U                                     /* ACTLR: DEOLP Position */
#define ACTLR_DEOLP_Msk                  (1UL << ACTLR_DEOLP_Pos)                /* ACTLR: DEOLP Mask */

#define ACTLR_DBHE_Pos                   20U                                     /* ACTLR: DBHE Position */
#define ACTLR_DBHE_Msk                   (1UL << ACTLR_DBHE_Pos)                 /* ACTLR: DBHE Mask */

#define ACTLR_FRCDIS_Pos                 19U                                     /* ACTLR: FRCDIS Position */
#define ACTLR_FRCDIS_Msk                 (1UL << ACTLR_FRCDIS_Pos)               /* ACTLR: FRCDIS Mask */

#define ACTLR_RSDIS_Pos                  17U                                     /* ACTLR: RSDIS Position */
#define ACTLR_RSDIS_Msk                  (1UL << ACTLR_RSDIS_Pos)                /* ACTLR: RSDIS Mask */

#define ACTLR_BP_Pos                     15U                                     /* ACTLR: BP Position */
#define ACTLR_BP_Msk                     (3UL << ACTLR_BP_Pos)                   /* ACTLR: BP Mask */

#define ACTLR_DBWR_Pos                   14U                                     /* ACTLR: DBWR Position */
#define ACTLR_DBWR_Msk                   (1UL << ACTLR_DBWR_Pos)                 /* ACTLR: DBWR Mask */

#define ACTLR_DLFO_Pos                   13U                                     /* ACTLR: DLFO Position */
#define ACTLR_DLFO_Msk                   (1UL << ACTLR_DLFO_Pos)                 /* ACTLR: DLFO Mask */

#define ACTLR_ERPEG_Pos                  12U                                     /* ACTLR: ERPEG Position */
#define ACTLR_ERPEG_Msk                  (1UL << ACTLR_ERPEG_Pos)                /* ACTLR: ERPEG Mask */

#define ACTLR_DNCH_Pos                   11U                                     /* ACTLR: DNCH Position */
#define ACTLR_DNCH_Msk                   (1UL << ACTLR_DNCH_Pos)                 /* ACTLR: DNCH Mask */

#define ACTLR_QOSEN_Pos                  11U                                     /* ACTLR: QOSEN Position */
#define ACTLR_QOSEN_Msk                  (1UL << ACTLR_QOSEN_Pos)                /* ACTLR: QOSEN Mask */

#define ACTLR_FORA_Pos                   10U                                     /* ACTLR: FORA Position */
#define ACTLR_FORA_Msk                   (1UL << ACTLR_FORA_Pos)                 /* ACTLR: FORA Mask */

#define ACTLR_ITCMECEN_Pos               10U                                     /* ACTLR: ITCMECEN Position */
#define ACTLR_ITCMECEN_Msk               (1UL << ACTLR_ITCMECEN_Pos)             /* ACTLR: ITCMECEN Mask */

#define ACTLR_FWT_Pos                    9U                                      /* ACTLR: FWT Position */
#define ACTLR_FWT_Msk                    (1UL << ACTLR_FWT_Pos)                  /* ACTLR: FWT Mask */

#define ACTLR_DTCMECEN_Pos               9U                                      /* ACTLR: DTCMECEN Position */
#define ACTLR_DTCMECEN_Msk               (1UL << ACTLR_DTCMECEN_Pos)             /* ACTLR: DTCMECEN Mask */

#define ACTLR_FDSNS_Pos                  8U                                      /* ACTLR: FDSNS Position */
#define ACTLR_FDSNS_Msk                  (1UL << ACTLR_FDSNS_Pos)                /* ACTLR: FDSNS Mask */

#define ACTLR_AOW_Pos                    8U                                      /* ACTLR: AOW Position */
#define ACTLR_AOW_Msk                    (1UL << ACTLR_AOW_Pos)                  /* ACTLR: AOW Mask */

#define ACTLR_SMOV_Pos                   7U                                      /* ACTLR: SMOV Position */
#define ACTLR_SMOV_Msk                   (1UL << ACTLR_SMOV_Pos)                 /* ACTLR: SMOV Mask */

#define ACTLR_DILS_Pos                   6U                                      /* ACTLR: DILS Position */
#define ACTLR_DILS_Msk                   (1UL << ACTLR_DILS_Pos)                 /* ACTLR: DILS Mask */

#define ACTLR_SMP_Pos                    6U                                      /* ACTLR: SMP Position */
#define ACTLR_SMP_Msk                    (1UL << ACTLR_SMP_Pos)                  /* ACTLR: SMP Mask */

#define ACTLR_CEC_Pos                    3U                                      /* ACTLR: CEC Position */
#define ACTLR_CEC_Msk                    (7UL << ACTLR_CEC_Pos)                  /* ACTLR: CEC Mask */

#define ACTLR_MRPEN_Pos                  3U                                      /* ACTLR: MRPEN Position */
#define ACTLR_MRPEN_Msk                  (1UL << ACTLR_MRPEN_Pos)                /* ACTLR: MRPEN Mask */

#define ACTLR_B1TCMECEN_Pos              2U                                      /* ACTLR: B1TCMECEN Position */
#define ACTLR_B1TCMECEN_Msk              (1UL << ACTLR_B1TCMECEN_Pos)            /* ACTLR: B1TCMECEN Mask */

#define ACTLR_B0TCMECEN_Pos              1U                                      /* ACTLR: B0TCMECEN Position */
#define ACTLR_B0TCMECEN_Msk              (1UL << ACTLR_B0TCMECEN_Pos)            /* ACTLR: B0TCMECEN Mask */

#define ACTLR_ATCMECEN_Pos               0U                                      /* ACTLR: ATCMECEN Position */
#define ACTLR_ATCMECEN_Msk               (1UL << ACTLR_ATCMECEN_Pos)             /* ACTLR: ATCMECEN Mask */

#define ACTLR_FW_Pos                     0U                                      /* ACTLR: FW Position */
#define ACTLR_FW_Msk                     (1UL << ACTLR_FW_Pos)                   /* ACTLR: FW Mask */

/* DFSR Register Definition */
#define DFSR_FnV_Pos                     16U                                    /* DFSR: FnV Position */
#define DFSR_FnV_Msk                     (1UL << DFSR_FnV_Pos)                  /* DFSR: FnV Mask */

#define DFSR_CM_Pos                      13U                                    /* DFSR: CM Position */
#define DFSR_CM_Msk                      (1UL << DFSR_CM_Pos)                   /* DFSR: CM Mask */

#define DFSR_Ext_Pos                     12U                                    /* DFSR: Ext Position */
#define DFSR_Ext_Msk                     (1UL << DFSR_Ext_Pos)                  /* DFSR: Ext Mask */

#define DFSR_WnR_Pos                     11U                                    /* DFSR: WnR Position */
#define DFSR_WnR_Msk                     (1UL << DFSR_WnR_Pos)                  /* DFSR: WnR Mask */

#define DFSR_FS1_Pos                     10U                                    /* DFSR: FS1 Position */
#define DFSR_FS1_Msk                     (1UL << DFSR_FS1_Pos)                  /* DFSR: FS1 Mask */

#define DFSR_LPAE_Pos                    9U                                     /* DFSR: LPAE Position */
#define DFSR_LPAE_Msk                    (1UL << DFSR_LPAE_Pos)                 /* DFSR: LPAE Mask */

#define DFSR_Domain_Pos                  4U                                     /* DFSR: Domain Position */
#define DFSR_Domain_Msk                  (0xFUL << DFSR_Domain_Pos)             /* DFSR: Domain Mask */

#define DFSR_FS0_Pos                     0U                                     /* DFSR: FS0 Position */
#define DFSR_FS0_Msk                     (0xFUL << DFSR_FS0_Pos)                /* DFSR: FS0 Mask */

#define DFSR_STATUS_Pos                  0U                                     /* DFSR: STATUS Position */
#define DFSR_STATUS_Msk                  (0x3FUL << DFSR_STATUS_Pos)            /* DFSR: STATUS Mask */

/* IFSR Register Definition */
#define IFSR_FnV_Pos                     16U                                    /* IFSR: FnV Position */
#define IFSR_FnV_Msk                     (1UL << IFSR_FnV_Pos)                  /* IFSR: FnV Mask */

#define IFSR_ExT_Pos                     12U                                    /* IFSR: ExT Position */
#define IFSR_ExT_Msk                     (1UL << IFSR_ExT_Pos)                  /* IFSR: ExT Mask */

#define IFSR_FS1_Pos                     10U                                    /* IFSR: FS1 Position */
#define IFSR_FS1_Msk                     (1UL << IFSR_FS1_Pos)                  /* IFSR: FS1 Mask */

#define IFSR_LPAE_Pos                    9U                                     /* IFSR: LPAE Position */
#define IFSR_LPAE_Msk                    (0x1UL << IFSR_LPAE_Pos)               /* IFSR: LPAE Mask */

#define IFSR_FS0_Pos                     0U                                     /* IFSR: FS0 Position */
#define IFSR_FS0_Msk                     (0xFUL << IFSR_FS0_Pos)                /* IFSR: FS0 Mask */

#define IFSR_STATUS_Pos                  0U                                     /* IFSR: STATUS Position */
#define IFSR_STATUS_Msk                  (0x3FUL << IFSR_STATUS_Pos)            /* IFSR: STATUS Mask */

/* ISR Register Definition */
#define ISR_A_Pos                        13U                                    /* ISR: A Position */
#define ISR_A_Msk                        (1UL << ISR_A_Pos)                     /* ISR: A Mask */

#define ISR_I_Pos                        12U                                    /* ISR: I Position */
#define ISR_I_Msk                        (1UL << ISR_I_Pos)                     /* ISR: I Mask */

#define ISR_F_Pos                        11U                                    /* ISR: F Position */
#define ISR_F_Msk                        (1UL << ISR_F_Pos)                     /* ISR: F Mask */

/* DACR Register Defnition */
#define DACR_D_Pos_(n)                   (2U*(n))                                 /* DACR: Dn Position */
#define DACR_D_Msk_(n)                   (3UL << DACR_D_Pos_(n))                /* DACR: Dn Mask */
#define DACR_Dn_NOACCESS                 0U                                     /* DACR Dn field: No access */
#define DACR_Dn_CLIENT                   1U                                     /* DACR Dn field: Client */
#define DACR_Dn_MANAGER                  3U                                     /* DACR Dn field: Manager */

/* CPACR Register Defnition */
#define CPACR_CP_Pos                     (20U)                                  /* CPACR: CP Position */
#define CPACR_CP_Msk                     (15UL << CPACR_CP_Pos)                 /* CPACR: CP Mask */

/* PRSELR Register Defnition */
#define PRSELR_REGION_Pos                (0U)                                   /* PRSELR: REGION Position */
#define PRSELR_REGION_16_Msk             (15UL << PRSELR_REGION_Pos)            /* PRSELR: REGION Mask (16 regions) */
#define PRSELR_REGION_24_Msk             (31UL << PRSELR_REGION_Pos)            /* PRSELR: REGION Mask (24 regions) */

/* PRBAR Register Defnition */
#define PRBAR_XN_Pos                     (0U)                                   /* PRBAR: XN Position */
#define PRBAR_XN_Msk                     (1UL << PRBAR_XN_Pos)                  /* PRBAR: XN Mask */
#define PRBAR_AP_Pos                     (1U)                                   /* PRBAR: AP Position */
#define PRBAR_AP_Msk                     (3UL << PRBAR_AP_Pos)                  /* PRBAR: AP Mask */
#define PRBAR_SH_Pos                     (3U)                                   /* PRBAR: SH Position */
#define PRBAR_SH_Msk                     (3UL << PRBAR_SH_Pos)                  /* PRBAR: SH Mask */
#define PRBAR_BASE_Pos                   (6U)                                   /* PRBAR: BASE Position */
#define PRBAR_BASE_Msk                   (0x3FFFFFFUL << PRBAR_BASE_Pos)        /* PRBAR: BASE Mask */

/* PRLAR Register Defnition */
#define PRLAR_EN_Pos                     (0U)                                   /* PRLAR: EN Position */
#define PRLAR_EN_Msk                     (1UL << PRLAR_EN_Pos)                  /* PRLAR: EN Mask */
#define PRLAR_ATTR_IDX_Pos               (1U)                                   /* PRLAR: AttrIndx Position */
#define PRLAR_ATTR_IDX_Msk               (7UL << PRLAR_ATTR_IDX_Pos)            /* PRLAR: AttrIndx Mask */
#define PRLAR_LIMIT_Pos                  (6U)                                   /* PRLAR: LIMIT Position */
#define PRLAR_LIMIT_Msk                  (0x3FFFFFFUL << PRLAR_LIMIT_Pos)       /* PRLAR: LIMIT Mask */

/* MAIR Register Defnition */
#define MAIR_ATTR_Pos(n)                 (8U*(n))                                 /* MAIR: Attr Position */
#define MAIR_ATTR_Msk(n)                 (255UL << MAIR_ATTR_Pos(n))            /* MAIR: Attr Mask */

/*******************************************************************************
**                      Cache                                                 **
*******************************************************************************/

/* Enable L1 I-Cache */
__STATIC_INLINE void L1C_EnableICache(void)
{
  __set_SCTLR( __get_SCTLR() | SCTLR_I_Msk);
  __ISB();
}

/* Disable L1 I-Cache */
__STATIC_INLINE void L1C_DisableICache(void)
{
  __set_SCTLR( __get_SCTLR() & (~SCTLR_I_Msk));
  __ISB();
}

/* Enable L1 D-Cache */
__STATIC_INLINE void L1C_EnableDCache(void)
{
  __set_SCTLR( __get_SCTLR() | SCTLR_C_Msk);
  __ISB();
}

/* Disable L1 D-Cache */
__STATIC_INLINE void L1C_DisableDCache(void)
{
  __set_SCTLR( __get_SCTLR() & (~SCTLR_C_Msk));
  __ISB();
}

/* Enable Branch Prediction */
__STATIC_INLINE void L1C_EnableBP(void) {
  __set_SCTLR( __get_SCTLR() | SCTLR_Z_Msk);
  __ISB();
}

/* Disable Branch Prediction */
__STATIC_INLINE void L1C_DisableBP(void) {
  __set_SCTLR( __get_SCTLR() & (~SCTLR_Z_Msk));
  __ISB();
}

/* Invalidate entire branch predictor array */
__STATIC_INLINE void L1C_InvalidateBP(void) {
  __set_BPIALL(0);
  __DSB();     //ensure completion of the invalidation
  __ISB();     //ensure instruction fetch path sees new state
}

/* Invalidate the whole instruction cache */
__STATIC_INLINE void L1C_InvalidateICacheAll(void) {
  __set_ICIALLU(0);
  __DSB();     //ensure completion of the invalidation
  __ISB();     //ensure instruction fetch path sees new I cache state
}

/* Calculate logarit 2 and round up */
__STATIC_INLINE uint32_t __log_2_n_roundup(uint32_t val)
{
  uint32_t ret = 0, additional = 0;

  if ((val & (val - 1U)) != 0U)
  {
    additional = 1;
  }
  while (val > 1)
  {
    ret += 1;
    val = val >> 1;
  }
  return (ret + additional);
}

/* Maintain data cache by set/way */
__STATIC_INLINE void __L1C_MaintainDCacheSetWay(uint32_t level, uint32_t maint)
{
  uint32_t Dummy;
  uint32_t ccsidr;
  uint32_t num_sets;
  uint32_t num_ways;
  uint32_t shift_way;
  uint32_t log2_linesize;
  uint32_t log2_num_ways;
  uint32_t way, set;

  Dummy = level << 1U;
  /* set csselr, select ccsidr register */
  __set_CSSELR(Dummy);
  /* get current ccsidr register */
  ccsidr = __get_CCSIDR();
  num_sets = ((ccsidr & 0x0FFFE000U) >> 13U) + 1U;
  num_ways = ((ccsidr & 0x00001FF8U) >> 3U) + 1U;
  /* Calculate L (based on line length) */
  /* From B6.1.7 CCSIDR, Cache Size ID Registers, PMSA and
          B6.2.1 Cache and branch predictor maintenance operations, PMSA
      - L = log2(LINELEN) with LINELEN is bytes
      - LINELEN = line_size x 4 with line_size is got from CCSIDR
      - line_size is calculated as below :
        + CSSIDR_LineSize = (log2(line_size))-2
            with CSSIDR_LineSize : value from CCSIDR
          => line_size = 2^(CSSIDR_LineSize + 2)
      => LINELEN = (2^(CSSIDR_LineSize + 2)) x 4 = 16 x 2^(CSSIDR_LineSize)
      => L = log2(16 x 2^(CSSIDR_LineSize)) = 4 + CSSIDR_LineSize
  */
  log2_linesize = (ccsidr & 0x00000007U) + 2U + 2U;
  /* Calculate A (based on num_ways) */
  log2_num_ways = __log_2_n_roundup(num_ways);
  shift_way = 32U - (uint32_t)log2_num_ways;
  for(way = 0; way < num_ways; way++)
  {
    for(set = 0; set < num_sets; set++)
    {
      Dummy = (level << 1U) | (((uint32_t)set) << log2_linesize) | (((uint32_t)way) << shift_way);
      switch (maint)
      {
        case 0U: __set_DCISW(Dummy);  break;
        case 1U: __set_DCCSW(Dummy);  break;
        default: __set_DCCISW(Dummy); break;
      }
    }
  }
  __DMB();
}

/* Clean and Invalidate the entire data or unified cache */
__STATIC_INLINE void L1C_MaintainDCacheSetWay(uint32_t op) {
  uint32_t clidr;
  uint32_t cache_type;
  uint32_t i = 0U;
  clidr =  __get_CLIDR();
  for(i = 0U; i < 7U; i++)
  {
    cache_type = (clidr >> i*3U) & 0x7UL;
    if ((cache_type >= 2U) && (cache_type <= 4U))
    {
      __L1C_MaintainDCacheSetWay(i, op);
    }
  }
}

/* Maintain data cache by address */
__STATIC_INLINE void __L1C_MaintainDCacheAddress(uint32_t level, uint32_t maint, \
                                                      uint32_t addr, uint32_t size)
{
  uint32_t Dummy, ccsidr;
  uint32_t line_size = 0, pow = 1, num_lines = 0;

  Dummy = level << 1U;
  /* set csselr, select ccsidr register */
  __set_CSSELR(Dummy);
  /* get current ccsidr register */
  ccsidr = __get_CCSIDR();
  line_size = (ccsidr & 0x00000007U);
  /* Convert to number of bytes */
  while (line_size-- > 0)
  {
    pow *= 2;
  }
  line_size = 16 * pow;
  /* Calculate num lines which need to be maintained */
  if ((size % line_size) != 0U)
  {
    num_lines = (size / line_size) + 1;
  }
  else
  {
    num_lines = (size / line_size);
  }

  while (num_lines-- > 0)
  {
    switch (maint)
    {
      case 0U: __set_DCIMVAC(addr);  break;
      case 1U: __set_DCCMVAC(addr);  break;
      default: __set_DCCIMVAC(addr); break;
    }
    addr += line_size;
  }

  __DMB();
}

/* Clean data cache line by address */
__STATIC_INLINE void L1C_CleanDCacheAddress(uint32_t addr, uint32_t size) {
  __L1C_MaintainDCacheAddress(1, 1, addr, size);
  __DMB();     //ensure the ordering of data cache maintenance operations and their effects
}

/* Invalidate data cache line by address */
__STATIC_INLINE void L1C_InvalidateDCacheAddress(uint32_t addr, uint32_t size) {
  __L1C_MaintainDCacheAddress(1, 0, addr, size);
  __DMB();     //ensure the ordering of data cache maintenance operations and their effects
}

/* Clean and Invalidate data cache by address */
__STATIC_INLINE void L1C_CleanInvalidateDCacheAddress(uint32_t addr, uint32_t size) {
  __L1C_MaintainDCacheAddress(1, 2, addr, size);
  __DMB();     //ensure the ordering of data cache maintenance operations and their effects
}

/* Invalidate the whole data cache */
__STATIC_INLINE void L1C_InvalidateDCacheAll(void) {
  L1C_MaintainDCacheSetWay(0);
}

/* Clean the whole data cache */
__STATIC_INLINE void L1C_CleanDCacheAll(void) {
  L1C_MaintainDCacheSetWay(1);
}

/* Clean and invalidate the whole data cache */
__STATIC_INLINE void L1C_CleanInvalidateDCacheAll(void) {
  L1C_MaintainDCacheSetWay(2);
}

/*@} end of group Cortex_R52 */


#if 1 // define GIC interface in drivers
/*******************************************************************************
 *                 Register Abstraction
  Core Register contain:
  - Core Register
 ******************************************************************************/
/**
  \defgroup CMSIS_core_register Defines and Type Definitions
  \brief Type definitions and defines for Cortex-M processor based devices.
*/

/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_CORE  Status and Control Registers
  \brief      Core Register type definitions.
  @{
 */

/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_GIC  Generic Interrupt Controller (GIC)
  \brief      Type definitions for the GIC Registers
  @{
*/

 /**
  \brief  Structure type to access the Generic Interrupt Controller (GIC) for GICD.
 */
typedef struct
{ 
  __IOM uint32_t GICD_CTLR;                  /*!< Offset: 0x0000 (R/W)  Distributor Control Register */
  __IM  uint32_t GICD_TYPER;                 /*!< Offset: 0x0004 (R/ )  Interrupt Controller Type Register */
  __IM  uint32_t GICD_IIDR;                  /*!< Offset: 0x0008 (R/ )  Distributor Implementer Identification Register */
        uint32_t RESERVED0[29U];
  __IOM uint32_t GICD_IGROUPR[32U];          /*!< Offset: 0x0080 (R/W)  Interrupt Group Registers 0 - 31 */
  __IOM uint32_t GICD_ISENABLER[32U];        /*!< Offset: 0x0100 (R/W)  Interrupt Set-Enable Registers 0 - 31 */
  __IOM uint32_t GICD_ICENABLER[32U];        /*!< Offset: 0x0180 (R/W)  Interrupt Clear-Enable Registers 0 - 31 */
  __IOM uint32_t GICD_ISPENDR[32U];          /*!< Offset: 0x0200 (R/W)  Interrupt Set-Pending Registers 0 - 31 */
  __IOM uint32_t GICD_ICPENDR[32U];          /*!< Offset: 0x0280 (R/W)  Interrupt Clear-Pending Registers 0 - 31 */
  __IOM uint32_t GICD_ISACTIVER[32U];        /*!< Offset: 0x0300 (R/W)  Interrupt Set-Active Registers 0 - 31 */
  __IOM uint32_t GICD_ICACTIVER[32U];        /*!< Offset: 0x0380 (R/W)  Interrupt Clear-Active Registers 0 - 31 */
  __IOM uint8_t  GICD_IPRIORITYR[1020U];     /*!< Offset: 0x0400 (R/W)  Interrupt Priority Registes 0 - 254 */
        uint32_t RESERVED1;
  __IOM uint32_t GICD_ITARGETSR[255U];       /*!< Offset: 0x0800 (R/W)  Interrupt Processor Targets Registes 0 - 254 */
        uint32_t RESERVED2;
  __IOM uint32_t GICD_ICFGR[64U];            /*!< Offset: 0x0C00 (R/W)  Interrupt Configuration Registers 0 - 63 */
  __IOM uint32_t GICD_IGRPMODR[32U];         /*!< Offset: 0x0D00 (R/W)  Interrupt Group Modifier Registers 0 - 31 */
        uint32_t RESERVED3[32];
  __IOM uint32_t GICD_NSACR[64];	     /*!< Offset: 0x0E00 (R/W)  Interrupt Group Modifier Registers 0 - 31 */
  __OM  uint32_t GICD_SGIR;                  /*!< Offset: 0x0F00 (R/W)  Interrupt Software Generated Registers*/
        uint32_t RESERVED4[3];
  __IOM uint32_t GICD_CPENDSGIR[4];          /*!< Offset: 0x0F10 (R/W)  Interrupt SGI Clear Pending Registers*/
  __IOM uint32_t GICD_SPENDSGIR[4];          /*!< Offset: 0x0F20 (R/W)  Interrupt SGI Set Pending Registers*/
        uint32_t RESERVED5[5172];
  __IOM uint64_t GICD_IROUTER[960U];         /*!< Offset: 0x6000 (R/ )  Interrupt Routing Registers 32 - 991 */
        uint32_t RESERVED6[2035U];           /* Reserved space from 0x7EFC to 0xFFCF */
  __IM  uint32_t GICD_PIDR[8U];              /*!< Offset: 0xFFD0 (R/ )  Identification Registers 4 - 7, Offset: 0xFFE0 (R/ )  Identification Registers 0 - 3 */
  __IM  uint32_t GICD_CIDR[4U];              /*!< Offset: 0xFFF0 (R/ )  Identification Registers 0 - 3 */
}  GICD_Type;

 /**
  \brief  Structure type to access the Generic Interrupt Controller (GIC) for GICR for Control target.
 */
typedef struct
{
  __IOM  uint32_t GICR_CTLR;                  /*!< Offset: 0x0000 (R/W )  Redistributor Control Register */
  __IM  uint32_t GICR_IIDR;                  /*!< Offset: 0x0004 (R/ )  Redistributor Implementer Identification Register */
  __IM  uint32_t GICR_TYPER[2];              /*!< Offset: 0x0008 (R/ )  Redistributor Type Register */
        uint32_t RESERVED0;
  __IOM uint32_t GICR_WAKER;                 /*!< Offset: 0x0014 (R/W)  Redistributor Wake Register */
        uint32_t RESERVED1[16370];           /* Reserved space from 0x0018 to 0xFFDF */
}  GICR_CONTROL_TARGET_Type;

 /**
  \brief  Structure type to access the Generic Interrupt Controller (GIC) for GICR for SGI and PPI.
 */
typedef struct
{
        uint32_t RESERVED0[32];
  __IOM uint32_t GICR_IGROUPR[32];             /*!< Offset: 0x0080 (R/W)  Interrupt Group Register 0 */
  __IOM uint32_t GICR_ISENABLER[3];            /*!< Offset: 0x0100 (R/W)  Interrupt Set-Enable Register 0 */
        uint32_t RESERVED2[29];
  __IOM uint32_t GICR_ICENABLER[3];            /*!< Offset: 0x0180 (R/W)  Interrupt Clear-Enable Register 0 */
        uint32_t RESERVED3[29];
  __IOM uint32_t GICR_ISPENDR[3];              /*!< Offset: 0x0200 (R/W)  Interrupt Set-Pending Register 0 */
        uint32_t RESERVED4[29];
  __IOM uint32_t GICR_ICPENDR[3];              /*!< Offset: 0x0280 (R/W)  Interrupt Clear-Pending Register 0 */
        uint32_t RESERVED5[29];
  __IOM uint32_t GICR_ISACTIVER0;            /*!< Offset: 0x0300 (R/W)  Interrupt Set-Active Register 0 */
        uint32_t RESERVED6[31];
  __IOM uint32_t GICR_ICACTIVER0;            /*!< Offset: 0x0380 (R/W)  Interrupt Clear-Active Register 0 */
        uint32_t RESERVED7[31];
  __IOM uint8_t GICR_IPRIORITYR[32];         /*!< Offset: 0x0400 (R/W)  Interrupt Priority Registers 0 - 7 */
        uint32_t RESERVED8[504];
  __IM  uint32_t GICR_ICFGR0;                /*!< Offset: 0x0C00 (R/ )  Interrupt Configuration Register 0 */
  __IOM uint32_t GICR_ICFGR1;                /*!< Offset: 0x0C04 (R/W)  Interrupt Configuration Register 1 */
        uint32_t RESERVED9[62];
        uint32_t GICR_IGRPMODR[3];           /*!< Offset: 0x0D00 (R/W)  Interrupt Group Modifier Registers */
}  GICR_SGI_PPI_Type;


typedef struct
{
  GICR_CONTROL_TARGET_Type   target_ctrl  __attribute__((aligned (0x10000)));
  GICR_SGI_PPI_Type          sgi_ppi  __attribute__((aligned (0x10000)));
} GICR_Type;

/*@} end of group CMSIS_GIC */


/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_core_base     Core Definitions
  \brief      Definitions for base addresses, unions, and structures.
  @{
 */

/* Memory mapping of Core Hardware */
#define GIC0_BASE                     (0xf0000000UL)              /*!< GIC0 Base Address */
#define GIC1_BASE                     (0x9C000000UL)              /*!< GIC1 Base Address */
#define GICR_TARGET0_BASE             (0x00100000UL)              /*!< GICR Base Address (for Control target 0) */
#define GICR_TARGET0_SGI_PPI_BASE     (0x00110000UL)              /*!< GICR Base Address (for SGI and PPI target 0) */

#define GICD0                         ((GICD_Type *) GIC0_BASE )   /*!< GICD configuration struct */
#define GICD1                         ((GICD_Type *) GIC1_BASE )   /*!< GICD configuration struct */
#define GICR0_TARGET0_IFREG           ((GICR_CONTROL_TARGET_Type *) (GIC0_BASE + GICR_TARGET0_BASE) )    /*!< GICR configuration struct for Control target 0 */
#define GICR1_TARGET0_IFREG           ((GICR_CONTROL_TARGET_Type *) (GIC1_BASE + GICR_TARGET0_BASE) )    /*!< GICR configuration struct for Control target 0 */
#define GICR0_TARGET0_INTREG          ((GICR_SGI_PPI_Type *) (GIC0_BASE + GICR_TARGET0_SGI_PPI_BASE) )   /*!< GICR configuration struct for SGI and PPI target 0 */
#define GICR1_TARGET0_INTREG          ((GICR_SGI_PPI_Type *) (GIC1_BASE + GICR_TARGET0_SGI_PPI_BASE) )   /*!< GICR configuration struct for SGI and PPI target 0 */

typedef	unsigned int IRQn_Type;

/* ##########################  GIC functions  ###################################### */

/** \brief  Enable the interrupt distributor using the GIC's CTLR register.
*/
__STATIC_INLINE void GIC_EnableDistributor(GICD_Type* GICDistributor)
{
  GICDistributor->GICD_CTLR |= 1U;
}

/** \brief Disable the interrupt distributor using the GIC's CTLR register.
*/
__STATIC_INLINE void GIC_DisableDistributor(GICD_Type* GICDistributor)
{
  GICDistributor->GICD_CTLR &=~1U;
}

/** \brief Read the GIC's TYPER register.
* \return GICDistributor_Type::TYPER
*/
__STATIC_INLINE uint32_t GIC_DistributorInfo(GICD_Type* GICDistributor)
{
  return (GICDistributor->GICD_TYPER);
}

/** \brief Reads the GIC's IIDR register.
* \return GICDistributor_Type::IIDR
*/
__STATIC_INLINE uint32_t GIC_DistributorImplementer(GICD_Type* GICDistributor)
{
  return (GICDistributor->GICD_IIDR);
}

/** \brief Sets the GIC's ITARGETSR register for the given interrupt.
* \param [in] IRQn Interrupt to be configured.
* \param [in] cpu_target CPU interfaces to assign this interrupt to.
*/
__STATIC_INLINE void GIC_SetTarget(GICD_Type* GICDistributor, IRQn_Type IRQn, uint32_t cpu_target)
{
  uint32_t mask = GICDistributor->GICD_ITARGETSR[IRQn / 4U] & ~(0xFFU << ((IRQn % 4U) * 8U));
  GICDistributor->GICD_ITARGETSR[IRQn / 4U] = mask | ((cpu_target & 0xFFU) << ((IRQn % 4U) * 8U));
}

/** \brief Read the GIC's ITARGETSR register.
* \param [in] IRQn Interrupt to acquire the configuration for.
* \return GICDistributor_Type::ITARGETSR
*/
__STATIC_INLINE uint32_t GIC_GetTarget(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  return (GICDistributor->GICD_ITARGETSR[IRQn / 4U] >> ((IRQn % 4U) * 8U)) & 0xFFUL;
}

/** \brief Enable the CPU's interrupt interface.
*/
__STATIC_INLINE void GIC_EnableInterface(GICD_Type* GICDistributor)
{
  GICDistributor->GICD_CTLR |= 1U; //enable interface
}

/** \brief Disable the CPU's interrupt interface.
*/
__STATIC_INLINE void GIC_DisableInterface(GICD_Type* GICDistributor)
{
  GICDistributor->GICD_CTLR &= ~1U; //disable distributor
}

/** \brief Enables the given interrupt using GIC's ISENABLER register.
* \param [in] IRQn The interrupt to be enabled.
*/
__STATIC_INLINE void GIC_EnableIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  GICDistributor->GICD_ISENABLER[IRQn / 32U] = 1U << (IRQn % 32U);
}

/** \brief Get interrupt enable status using GIC's ISENABLER register.
* \param [in] IRQn The interrupt to be queried.
* \return 0 - interrupt is not enabled, 1 - interrupt is enabled.
*/
__STATIC_INLINE uint32_t GIC_GetEnableIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  return (GICDistributor->GICD_ISENABLER[IRQn / 32U] >> (IRQn % 32U)) & 1UL;
}

/** \brief Disables the given interrupt using GIC's ICENABLER register.
* \param [in] IRQn The interrupt to be disabled.
*/
__STATIC_INLINE void GIC_DisableIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  GICDistributor->GICD_ICENABLER[IRQn / 32U] = 1U << (IRQn % 32U);
}

/** \brief Get interrupt pending status from GIC's ISPENDR register.
* \param [in] IRQn The interrupt to be queried.
* \return 0 - interrupt is not pending, 1 - interrupt is pendig.
*/
__STATIC_INLINE uint32_t GIC_GetPendingIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  uint32_t pend;

  if (IRQn >= 16U) {
    pend = (GICDistributor->GICD_ISPENDR[IRQn / 32U] >> (IRQn % 32U)) & 1UL;
  } else {
    // INTID 0-15 Software Generated Interrupt
    pend = (GICDistributor->GICD_SPENDSGIR[IRQn / 4U] >> ((IRQn % 4U) * 8U)) & 0xFFUL;
    // No CPU identification offered
    if (pend != 0U) {
      pend = 1U;
    } else {
      pend = 0U;
    }
  }

  return (pend);
}

/** \brief Sets the given interrupt as pending using GIC's ISPENDR register.
* \param [in] IRQn The interrupt to be enabled.
*/
__STATIC_INLINE void GIC_SetPendingIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
    GICDistributor->GICD_ISPENDR[IRQn / 32U] = 1U << (IRQn % 32U);
}

/** \brief Clears the given interrupt from being pending using GIC's ICPENDR register.
* \param [in] IRQn The interrupt to be enabled.
*/
__STATIC_INLINE void GIC_ClearPendingIRQ(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
    GICDistributor->GICD_ICPENDR[IRQn / 32U] = 1U << (IRQn % 32U);

}

/** \brief Sets the interrupt configuration using GIC's ICFGR register.
* \param [in] IRQn The interrupt to be configured.
* \param [in] int_config Int_config field value. Bit 0: Reserved (0 - N-N model, 1 - 1-N model for some GIC before v1)
*                                           Bit 1: 0 - level sensitive, 1 - edge triggered
*/
__STATIC_INLINE void GIC_SetConfiguration(GICD_Type* GICDistributor, IRQn_Type IRQn, uint32_t int_config)
{
  uint32_t icfgr = GICDistributor->GICD_ICFGR[IRQn / 16U];  /* read current register content */
  uint32_t shift = (IRQn % 16U) << 1U;                 /* calculate shift value */

  int_config &= 3U;                                    /* only 2 bits are valid */
  icfgr &= (~(3U         << shift));                   /* clear bits to change */
  icfgr |= (  int_config << shift);                    /* set new configuration */

  GICDistributor->GICD_ICFGR[IRQn / 16U] = icfgr;           /* write new register content */
}

/** \brief Get the interrupt configuration from the GIC's ICFGR register.
* \param [in] IRQn Interrupt to acquire the configuration for.
* \return Int_config field value. Bit 0: Reserved (0 - N-N model, 1 - 1-N model for some GIC before v1)
*                                 Bit 1: 0 - level sensitive, 1 - edge triggered
*/
__STATIC_INLINE uint32_t GIC_GetConfiguration(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  return (GICDistributor->GICD_ICFGR[IRQn / 16U] >> ((IRQn % 16U) >> 1U));
}

/** \brief Set the priority for the given interrupt in the GIC's IPRIORITYR register.
* \param [in] IRQn The interrupt to be configured.
* \param [in] priority The priority for the interrupt, lower values denote higher priorities.
*/
__STATIC_INLINE void GIC_SetPriority(GICD_Type* GICDistributor, IRQn_Type IRQn, uint32_t priority)
{
  uint32_t mask = GICDistributor->GICD_IPRIORITYR[IRQn / 4U] & ~(0xFFU << ((IRQn % 4U) * 8U));
  GICDistributor->GICD_IPRIORITYR[IRQn / 4U] = mask | ((priority & 0xFFU) << ((IRQn % 4U) * 8U));
}

/** \brief Read the current interrupt priority from GIC's IPRIORITYR register.
* \param [in] IRQn The interrupt to be queried.
*/
__STATIC_INLINE uint32_t GIC_GetPriority(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  return (GICDistributor->GICD_IPRIORITYR[IRQn / 4U] >> ((IRQn % 4U) * 8U)) & 0xFFUL;
}

/** \brief Get the status for a given interrupt.
* \param [in] IRQn The interrupt to get status for.
* \return 0 - not pending/active, 1 - pending, 2 - active, 3 - pending and active
*/
__STATIC_INLINE uint32_t GIC_GetIRQStatus(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  uint32_t pending, active;

  active = ((GICDistributor->GICD_ISACTIVER[IRQn / 32U])  >> (IRQn % 32U)) & 1UL;
  pending = ((GICDistributor->GICD_ISPENDR[IRQn / 32U]) >> (IRQn % 32U)) & 1UL;

  return ((active<<1U) | pending);
}

/** \brief Generate a software interrupt using GIC's SGIR register.
* \param [in] IRQn Software interrupt to be generated.
* \param [in] target_list List of CPUs the software interrupt should be forwarded to.
* \param [in] filter_list Filter to be applied to determine interrupt receivers.
*/
__STATIC_INLINE void GIC_SendSGI(GICD_Type* GICDistributor, IRQn_Type IRQn, uint32_t target_list, uint32_t filter_list)
{
  GICDistributor->GICD_SGIR = ((filter_list & 3U) << 24U) | ((target_list & 0xFFUL) << 16U) | (IRQn & 0x0FUL);
}

/** \brief Set the interrupt group from the GIC's IGROUPR register.
* \param [in] IRQn The interrupt to be queried.
* \param [in] group Interrupt group number: 0 - Group 0, 1 - Group 1
*/
__STATIC_INLINE void GIC_SetGroup(GICD_Type* GICDistributor, IRQn_Type IRQn, uint32_t group)
{
  uint32_t igroupr = GICDistributor->GICD_IGROUPR[IRQn / 32U];
  uint32_t shift   = (IRQn % 32U);

  igroupr &= (~(1U          << shift));
  igroupr |= ( (group & 1U) << shift);

  GICDistributor->GICD_IGROUPR[IRQn / 32U] = igroupr;
}
#define GIC_SetSecurity         GIC_SetGroup

/** \brief Get the interrupt group from the GIC's IGROUPR register.
* \param [in] IRQn The interrupt to be queried.
* \return 0 - Group 0, 1 - Group 1
*/
__STATIC_INLINE uint32_t GIC_GetGroup(GICD_Type* GICDistributor, IRQn_Type IRQn)
{
  return (GICDistributor->GICD_IGROUPR[IRQn / 32U] >> (IRQn % 32U)) & 1UL;
}

/*@} */


/* ###########################  Core Function Access  ########################### */
/** \ingroup  CMSIS_Core_FunctionInterface
    \defgroup CMSIS_Core_RegAccFunctions CMSIS Core Register Access Functions
  @{
 */


#if   defined ( __CC_ARM ) /*------------------RealView Compiler -----------------*/
/* ARM armcc specific functions */

#if (__ARMCC_VERSION < 400677)
  #error "Please use ARM Compiler Toolchain V4.0.677 or later!"
#endif


/** \brief  Get CPSR Register

    This function returns the content of the CPSR Register.

    \return               CPSR Register value
 */
__STATIC_INLINE uint32_t __get_CPSR(void)
{
  register uint32_t __regCPSR          __ASM("cpsr");
  return(__regCPSR);
}


#elif (defined (__ICCARM__)) /*---------------- ICC Compiler ---------------------*/


#include <intrinsics.h>


#endif


#ifdef __cplusplus
}
#endif

#endif

#endif /* __CORE_CR52_H_DEPENDANT */

#endif /* __CMSIS_GENERIC */
