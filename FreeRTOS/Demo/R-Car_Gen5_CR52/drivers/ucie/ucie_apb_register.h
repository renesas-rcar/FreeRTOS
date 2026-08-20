/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_UCIE_APB_REGISTER_H_
#define R_UCIE_APB_REGISTER_H_

#include "ucie_common.h"

/* --- Common / PCIe control group (E00xxx) --- */
#define UCIE_APB_UCIEMSR0_OFFSET            0x00E00000U  /* PCIe Mode Setting Register 0            */
#define UCIE_APB_UCIERSTCTRL1_OFFSET        0x00E00014U  /* PCIe Reset Control Register 1           */
#define UCIE_APB_UCIERSTCTRL2_OFFSET        0x00E00018U  /* PCIe Reset Control Register 2           */
#define UCIE_APB_UCIEMSICTRL0_OFFSET        0x00E00024U  /* MSI Control Register 0                  */
#define UCIE_APB_UCIEMSICTRL1_OFFSET        0x00E00028U  /* MSI Control Register 1                  */
#define UCIE_APB_UCIEMSICTRL2_OFFSET        0x00E0002CU  /* MSI Control Register 2                  */
#define UCIE_APB_UCIEMSICTRL3_OFFSET        0x00E00030U  /* MSI Control Register 3                  */
#define UCIE_APB_UCIEMSICTRL4_OFFSET        0x00E00034U  /* MSI Control Register 4  (R)             */
#define UCIE_APB_UCIEMSGTX_OFFSET           0x00E0003CU  /* PCIe Message Transmission               */
#define UCIE_APB_UCIEMSGRX2_OFFSET          0x00E00048U  /* PCIe Message Reception  (R)             */
#define UCIE_APB_UCIEPWRMNGCTRL_OFFSET      0x00E00070U  /* PCIe Power Management Control           */
#define UCIE_APB_UCIEINTXSTS_OFFSET         0x00E00094U  /* PCIe INTx Interrupt Status (R)          */
#define UCIE_APB_UCIEINTXSTSCLR_OFFSET      0x00E002E0U  /* PCIe INTx Interrupt Status Clear        */
#define UCIE_APB_UCIEFMIS_OFFSET            0x00E00308U  /* PCIe Flit Mode I/F / Segment Signals    */
#define UCIE_APB_UCIEFDMS_OFFSET            0x00E00320U  /* PCIe FRS/DRS Messaging Signals          */
#define UCIE_APB_UCIEDWEHSS_OFFSET          0x00E0032CU  /* PCIe DMA Write Engine Hard Stop Signals */
#define UCIE_APB_UCIECNFGINFO2_OFFSET       0x00E004D8U  /* config 02 : BDF Signals                 */
#define UCIE_APB_UCIESYSINT_OFFSET          0x00E00548U  /* PCIe Interrupt Signals                  */
#define UCIE_APB_UCIEIACTIFSEL_OFFSET       0x00E00564U  /* UCie Active Interface Selection         */
#define UCIE_APB_UCIECFGMSIADDR0_OFFSET     0x00E00584U  /* cfg_msi_addr [31:0]   (R)               */
#define UCIE_APB_UCIECFGMSIADDR1_OFFSET     0x00E00588U  /* cfg_msi_addr [63:32]  (R)               */
#define UCIE_APB_UCIECFGMSIADDR2_OFFSET     0x00E0058CU  /* cfg_msi_addr [95:64]  (R)               */
#define UCIE_APB_UCIECFGMSIADDR3_OFFSET     0x00E00590U  /* cfg_msi_addr [127:96] (R)               */
#define UCIE_APB_UCIECFGMSIDATA0_OFFSET     0x00E00594U  /* cfg_msi_data [31:0]   (R)               */
#define UCIE_APB_UCIECFGMSIDATA1_OFFSET     0x00E00598U  /* cfg_msi_data [63:32]  (R)               */
#define UCIE_APB_UCIECFGMSIMASK0_OFFSET     0x00E0059CU  /* cfg_msi_mask [31:0]   (R)               */
#define UCIE_APB_UCIECFGMSIMASK1_OFFSET     0x00E005A0U  /* cfg_msi_mask [63:32]  (R)               */
#define UCIE_APB_UCIEDBIADR_OFFSET          0x00E005E8U  /* dbi_axaddr_31to24                        */
#define UCIE_APB_UCIEGICIF_OFFSET           0x00E005ECU  /* PCIe IDE AXINTC_gic                     */
#define UCIE_APB_UCIEFGICADR0_OFFSET        0x00E005F0U  /* PCIe IDE AXINTC_gic_adr0                */
#define UCIE_APB_UCIEFGICADR1_OFFSET        0x00E005F4U  /* PCIe IDE AXINTC_gic_adr1                */

/* --- Interrupt Control group (E10xxx) --- */
#define UCIE_APB_UCIEICR00a_OFFSET          0x00E10000U  /* Interrupt Control Reg00 (Initial: 0x00100000) */
#define UCIE_APB_UCIEICR00b_OFFSET          0x00E10004U  /* Interrupt Control Reg01 (Initial: 0x00000040) */
#define UCIE_APB_UCIEICR11_OFFSET           0x00E10034U  /* Interrupt Control Reg11                 */
#define UCIE_APB_UCIEICR12_OFFSET           0x00E10038U  /* Interrupt Control Reg12                 */
#define UCIE_APB_UCIEICR13_OFFSET           0x00E1003CU  /* Interrupt Control Reg13                 */
#define UCIE_APB_UCIEICR14a_OFFSET          0x00E10040U  /* Interrupt Control Reg14a (Enable A)     */
#define UCIE_APB_UCIEICR14b_OFFSET          0x00E10044U  /* Interrupt Control Reg14b (Enable B)     */
#define UCIE_APB_UCIEICR15a_OFFSET          0x00E10048U  /* Interrupt Control Reg15a (DMA EN [31:0])  */
#define UCIE_APB_UCIEICR15b_OFFSET          0x00E1004CU  /* Interrupt Control Reg15b (DMA EN [63:32]) */
#define UCIE_APB_UCIEICR25_OFFSET           0x00E10074U  /* Interrupt Control Reg25                 */
#define UCIE_APB_UCIEICR27_OFFSET           0x00E1007CU  /* Interrupt Control Reg27                 */
#define UCIE_APB_UCIEICR28a_OFFSET          0x00E10080U  /* Interrupt Control Reg28a (Clear A)      */
#define UCIE_APB_UCIEICR28b_OFFSET          0x00E10084U  /* Interrupt Control Reg28b (Clear B)      */
#define UCIE_APB_UCIEICR39_OFFSET           0x00E100B4U  /* Interrupt Control Reg39                 */
#define UCIE_APB_UCIEICR40_OFFSET           0x00E100B8U  /* Interrupt Control Reg40                 */
#define UCIE_APB_UCIEICR41_OFFSET           0x00E100BCU  /* Interrupt Control Reg41                 */

/* --- CXL Selector / PHY group (E20xxx / E21xxx) --- */
#define UCIE_APB_UCIECSR00_OFFSET           0x00E20000U  /* CXL Selector Reg00                      */
#define UCIE_APB_UCIEPCR00_OFFSET           0x00E21000U  /* PHY Control Reg00 (App ctrl / Dev Sel)  */
#define UCIE_APB_UCIEPTR00_OFFSET           0x00E21004U  /* PHY Top Reg00 (Global Signals)          */
#define UCIE_APB_UCIEPTR01_OFFSET           0x00E21008U  /* PHY Top Reg01 (Global Signals, R)       */

/* --- AXI Slave (E80xxx) --- */
#define UCIE_APB_UCIESLVADR_OFFSET          0x00E80008U  /* slv_axaddr_with_axuser                  */

#define UCIEPCR00_ROOT_PORT_ENABLE_MASK             BIT_MASK(0)
#define UCIECSR00_CXL_MODE_MASK                     BIT_MASK(0)
#define UCIEPCR00_ENDPOINT_ENABLE_MASK              BIT_MASK(1)
#define UCIEICR27_INTERRUPT_OUTPUT_ENABLE_MASK      BIT_MASK(1)

#endif // R_UCIE_APB_REGISTER_H_
