/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include "r_crc_wrapper.h"
#include "dmac/dmac_common.h"
#include "interrupts.h"
#include "dmac_ctrl_common.h"

#ifndef BIT
#define BIT(x) (1U << (x))
#endif

#define BIT_CONVERT_TO_BYTE 8

#define WCRC_MODULE 0
#define CRC_MODULE  1
#define KCRC_MODULE 2

typedef struct st_wcrc_cfg_dma {
    rDmacCfg_t      cfg;
    rDmacIrqCfg_t   irq;
} wcrc_cfg_dma_t; 

typedef enum e_num_dma_chan
{
    INDEPENDENT_CRC_NO_USE_DMA          = 0,
    E2E_CRC_USE_2_DMA_CHAN              = 2,
    DATA_THROUGH_USE_2_DMA_CHAN         = 2,
    E2E_DATA_THROUGH_USE_3_DMA_CHAN     = 3,
    REGISTER_ACCESS_USE_1_DMA_CHAN      = 1,
    COMPARING_CRC_USE_3_DMA_CHAN        = 2,
} num_dma_chan_t;

typedef enum e_wcrc_mode_fifo_port
{
    E2E_PORT_DATA                       = 0,
    E2E_PORT_RESULT                     = 1,
    DATA_THROUGH_PORT_DATA_INPUT        = 0,
    DATA_THROUGH_PORT_DATA_OUTPUT       = 1,
    E2E_DATA_THROUGH_PORT_DATA_INPUT    = 0,
    E2E_DATA_THROUGH_PORT_DATA_OUTPUT   = 1,
    E2E_DATA_THROUGH_PORT_RESULT        = 2,
    REGISTER_ACCESS_PORT_COMMAND        = 0,
    COMPARING_CRC_PORT_DATA             = 0,
    COMPARING_CRC_PORT_EXPECTED_DATA    = 1,
} wcrc_mode_fifo_port_t;

typedef enum e_wcrc_fifo_ports_use_rtdma
{
    E2E_PORT_DATA_USE_RTDMA_INDEX_0                     = 0,
    E2E_PORT_RESULT_USE_RTDMA_INDEX_1                   = 1,

    DATA_THROUGH_PORT_DATA_IN_USE_RTDMA_INDEX_0         = 0,
    DATA_THROUGH_PORT_DATA_OUT_USE_RTDMA_INDEX_1        = 1,

    E2E_DATA_THROUGH_PORT_DATA_IN_USE_RTDMA_INDEX_0     = 0,
    E2E_DATA_THROUGH_PORT_DATA_OUT_USE_RTDMA_INDEX_1    = 1,
    E2E_DATA_THROUGH_PORT_RESULT_USE_RTDMA_INDEX_2      = 2,

    REGISTER_ACCESS_PORT_COMMAND_USE_RTDMA_INDEX_0      = 0,

    COMPARING_CRC_PORT_DATA_USE_RTDMA_INDEX_0           = 0,
    COMPARING_CRC_PORT_EXPECTED_DATA_USE_RTDMA_INDEX_1  = 1,
} wcrc_fifo_ports_use_rtdma_t;

#define MEM_TO_DEV 1
#define DEV_TO_MEM 2
#define NUM_DATA_ALIGN_AXI_BUS  4

/************************************ WCRC registers ************************************/
/* Register base */
#define WCRC_BASE_ADDR(unit)       (0x19200000 + (unit * 0x10000))
#define WCRC_FIFO_ADDR(unit)       (0x19400000 + (unit * 0x04000))

/* Request ID for WCRC */
#define MID_RID_WCRC_CRC_RES(unit)                  ((unit)*(0x10) + (0x18U))	/* WCRC(unit) CRC  DMARS ID RX */
#define MID_RID_WCRC_CRC_IN(unit)                   ((unit)*(0x10) + (0x19U))	/* WCRC(unit) CRC  DMARS ID TX */
#define MID_RID_WCRC_CRC_OUT(unit)                  ((unit)*(0x10) + (0x1AU))	/* WCRC(unit) CRC  DMARS ID RX */
#define MID_RID_WCRC_CRC_RES_MULTI(unit)            ((unit)*(0x10) + (0x1BU))	/* WCRC(unit) CRC  DMARS ID TX-RX */
#define MID_RID_WCRC_KCRC_RES(unit)                 ((unit)*(0x10) + (0x1CU))	/* WCRC(unit) KCRC DMARS ID RX */
#define MID_RID_WCRC_KCRC_IN(unit)                  ((unit)*(0x10) + (0x1DU))	/* WCRC(unit) KCRC DMARS ID TX */
#define MID_RID_WCRC_KCRC_OUT(unit)                 ((unit)*(0x10) + (0x1EU))	/* WCRC(unit) KCRC DMARS ID RX */
#define MID_RID_WCRC_KCRC_RES_MULTI(unit)           ((unit)*(0x10) + (0x1FU))	/* WCRC(unit) KCRC DMARS ID TX-RX */

#define MID_RID_WCRC_RES(mod, unit) ({                              \
    int _mod = (mod);                                               \
    ((_mod) == (CRC_M))  ? (MID_RID_WCRC_CRC_RES(unit))  :          \
    ((_mod) == (KCRC_M)) ? (MID_RID_WCRC_KCRC_RES(unit)) :          \
    (MID_RID_WCRC_CRC_RES(unit));                                   \
})

#define MID_RID_WCRC_IN(mod, unit) ({                               \
    int _mod = (mod);                                               \
    ((_mod) == (CRC_M))  ? (MID_RID_WCRC_CRC_IN(unit))  :           \
    ((_mod) == (KCRC_M)) ? (MID_RID_WCRC_KCRC_IN(unit)) :           \
    (MID_RID_WCRC_CRC_IN(unit));                                    \
})

#define MID_RID_WCRC_OUT(mod, unit) ({                              \
    int _mod = (mod);                                               \
    ((_mod) == (CRC_M))  ? (MID_RID_WCRC_CRC_OUT(unit))  :          \
    ((_mod) == (KCRC_M)) ? (MID_RID_WCRC_KCRC_OUT(unit)) :          \
    (MID_RID_WCRC_CRC_OUT(unit));                                   \
})

#define MID_RID_WCRC_RES_MULTI(mod, unit) ({                        \
    int _mod = (mod);                                               \
    ((_mod) == (CRC_M))  ? (MID_RID_WCRC_CRC_RES_MULTI(unit))  :    \
    ((_mod) == (KCRC_M)) ? (MID_RID_WCRC_KCRC_RES_MULTI(unit)) :    \
    (MID_RID_WCRC_CRC_RES_MULTI(unit));                             \
})

#define INDEPENDENT_CRC_DATA_SIZE   4

/* Register offset */
#define CRC_M        (CRC_SUB_MODULE)
#define KCRC_M       (KCRC_SUB_MODULE)

/* Address assignment of FIFO */
/* Data */
#define PORT_DATA(mod) ({               \
    int _mod = (mod);                   \
    ((_mod) == (CRC_M))  ? (0x800) :    \
    ((_mod) == (KCRC_M)) ? (0xC00) :    \
    (0x800);                            \
})

/* Command */
#define PORT_CMD(mod) ({                \
    int _mod = (mod);                   \
    ((_mod) == (CRC_M))  ? (0x900) :    \
    ((_mod) == (KCRC_M)) ? (0xD00) :    \
    (0x900);                            \
})

#define PORT_EXPT_DATA(mod) ({          \
    int _mod = (mod);                   \
    ((_mod) == (CRC_M))  ? (0xA00) :    \
    ((_mod) == (KCRC_M)) ? (0xE00) :    \
    (0xA00);                            \
})

/* Result */
#define PORT_RES(mod) ({                \
    int _mod = (mod);                   \
    ((_mod) == (CRC_M))  ? (0xB00) :    \
    ((_mod) == (KCRC_M)) ? (0xF00) :    \
    (0xB00);                            \
})

/* WCRC register (XXXX: CRC_M or KCRC_M) */

/* WCRC_XXXX_EN transfer enable register */
#define WCRC_CRC_EN 0x0800
#define WCRC_KCRC_EN 0x0C00
#define WCRC_XXXX_EN(mod) ({                \
    int _mod = (mod);                       \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_EN)  : \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_EN) : \
    (WCRC_CRC_EN);                          \
})
#define OUT_EN BIT(16)
#define RES_EN BIT(8)
#define TRANS_EN BIT(1)
#define IN_EN BIT(0)

/* WCRC_XXXX_STOP transfer stop register */
#define WCRC_CRC_STOP 0x0820
#define WCRC_KCRC_STOP 0x0C20
#define WCRC_XXXX_STOP(mod) ({                  \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_STOP)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_STOP) :   \
    (WCRC_CRC_STOP);                            \
})
#define STOP BIT(0)

/* WCRC_XXXX_CMDEN transfer command enable register */
#define WCRC_CRC_CMDEN 0x0830
#define WCRC_KCRC_CMDEN 0x0C30
#define WCRC_XXXX_CMDEN(mod) ({                 \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_CMDEN)  :  \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_CMDEN) :  \
    (WCRC_CRC_CMDEN);                           \
})
#define CMD_EN BIT(0)

/* WCRC_XXXX_COMP compare setting register */
#define WCRC_CRC_COMP 0x0840
#define WCRC_KCRC_COMP 0x0C40
#define WCRC_XXXX_COMP(mod) ({                  \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_COMP)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_COMP) :   \
    (WCRC_CRC_COMP);                            \
})
#define COMP_FREQ_16 (0 << 16)
#define COMP_FREQ_32 BIT(16)
#define COMP_FREQ_64 (3 << 16)
#define EXP_REQSEL BIT(1)
#define COMP_EN BIT(0)

/* WCRC_XXXX_COMP_RES compare result register regrister */
#define WCRC_CRC_COMP_RES 0x0850
#define WCRC_KCRC_COMP_RES 0x0C50
#define WCRC_XXXX_COMP_RES(mod) ({                  \
    int _mod = (mod);                               \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_COMP_RES)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_COMP_RES) :   \
    (WCRC_CRC_COMP_RES);                            \
})

/* WCRC_XXXX_CONV conversion setting register */
#define WCRC_CRC_CONV 0x0870
#define WCRC_KCRC_CONV 0x0C70
#define WCRC_XXXX_CONV(mod) ({                  \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_CONV)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_CONV) :   \
    (WCRC_CRC_CONV);                            \
})

/* WCRC_XXXX_WAIT wait register */
#define WCRC_CRC_WAIT 0x0880
#define WCRC_KCRC_WAIT 0x0C80
#define WCRC_XXXX_WAIT(mod) ({                  \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_WAIT)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_WAIT) :   \
    (WCRC_CRC_WAIT);                            \
})
#define WAIT BIT(0)

/* WCRC_XXXX_INIT_CRC initial CRC code register */
#define WCRC_CRC_INIT_CRC 0x0910
#define WCRC_KCRC_INIT_CRC 0x0D10
#define WCRC_XXXX_INIT_CRC(mod) ({                  \
    int _mod = (mod);                               \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_INIT_CRC)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_INIT_CRC) :   \
    (WCRC_CRC_INIT_CRC);                            \
})
#define INIT_CODE 0xFFFFFFFF

/* WCRC_XXXX_STS status register */
#define WCRC_CRC_STS 0x0A00
#define WCRC_KCRC_STS 0x0E00
#define WCRC_XXXX_STS(mod) ({                   \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_STS)  :    \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_STS) :    \
    (WCRC_CRC_STS);                             \
})
#define STOP_DONE BIT(31)
#define CMD_DONE BIT(24)
#define RES_DONE BIT(20)
#define COMP_ERR BIT(13)
#define COMP_DONE BIT(12)
#define TRANS_DONE BIT(0)

/* WCRC_XXXX_INTEN interrupt enable register */
#define WCRC_CRC_INTEN 0x0A40
#define WCRC_KCRC_INTEN 0x0E40
#define WCRC_XXXX_INTEN(mod) ({                 \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_INTEN)  :  \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_INTEN) :  \
    (WCRC_CRC_INTEN);                           \
})
#define STOP_DONE_IE BIT(31)
#define CMD_DONE_IE BIT(24)
#define RES_DONE_IE BIT(20)
#define COMP_ERR_IE BIT(13)
#define COMP_DONE_IE BIT(12)
#define TRANS_DONE_IE BIT(0)

/* WCRC_XXXX_ECMEN ECM output enable register */
#define WCRC_CRC_ECMEN 0x0A80
#define WCRC_KCRC_ECMEN 0x0E80
#define WCRC_XXXX_ECMEN(mod) ({                 \
    int _mod = (mod);                           \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_ECMEN)  :  \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_ECMEN) :  \
    (WCRC_CRC_ECMEN);                           \
})
#define COMP_ERR_OE BIT(13)

/* WCRC_XXXX_BUF_STS_RDEN Buffer state read enable register */
#define WCRC_CRC_BUF_STS_RDEN 0x0AA0
#define WCRC_KCRC_BUF_STS_RDEN 0x0EA0
#define WCRC_XXXX_BUF_STS_RDEN(mod) ({                  \
    int _mod = (mod);                                   \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_BUF_STS_RDEN)  :   \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_BUF_STS_RDEN) :   \
    (WCRC_CRC_BUF_STS_RDEN);                            \
})
#define CODE_VALUE (0xA5A5 << 16)
#define BUF_STS_RDEN BIT(0)

/* WCRC_XXXX_BUF_STS Buffer state read register */
#define WCRC_CRC_BUF_STS 0x0AA4
#define WCRC_KCRC_BUF_STS 0x0EA4
#define WCRC_XXXX_BUF_STS(mod) ({                   \
    int _mod = (mod);                               \
    ((_mod) == (CRC_M))  ? (WCRC_CRC_BUF_STS)  :    \
    ((_mod) == (KCRC_M)) ? (WCRC_KCRC_BUF_STS) :    \
    (WCRC_CRC_BUF_STS);                             \
})
#define RES_COMP_ENDFLAG BIT(18)
#define BUF_EMPTY BIT(8)

/* WCRC common */

/* WCRCm common status register */
#define WCRC_COMMON_STS 0x0F00
#define EDC_ERR BIT(16)

/* WCRCm common interrupt enable register */
#define WCRC_INTEN 0x0F00
#define EDC_ERR_IE BIT(16)

/* WCRCm common ECM output enable register */
#define WCRC_COMMON_ECMEN 0x0F80
#define EDC_ERR_OE BIT(16)

/* WCRCm error injection register */
#define WCRC_ERRINJ 0x0FC0
#define CODE (0xA5A5 << 16)

/************************************ CRC registers ************************************/
/* Register base */
#define CRC_BASE_ADDR(unit)       (0x19000000 + (unit * 0x10000))

/* Register offset */
/* CRC[m] Input register */
#define DCRA_CIN 0x0000

/* CRC[m] Data register */
#define DCRA_COUT 0x0004
#define COUT_DEF 0xFFFFFFFF //default; value for CRC calculation
//initial value of each polynimial: CRC calulation method; polynomial
#define COUT_32_ETHERNET 0xFFFFFFFFU //default; CRC-32-IEEE 802.3; 04C11DB7
#define COUT_16_CCITT_FALSE_CRC16 0xFFFF //CCITT_FALSE_CRC16; 1021
#define COUT_8_SAE_J1850 0xFF //SAE_J1850; 1D
#define COUT_8_0X2F 0xFF // 0x2F polynomial
#define COUT_32_0XF4ACFB13 0xFFFFFFFFU //0xF4ACFB13 polynomial
#define COUT_32_0X1EDC6F41 0xFFFFFFFFU //0x1EDC6F41 polynomial CRC-32 (Castagnoli)
#define COUT_21_0X102899 0x1FFFFF //0x102899 polynomial CRC-21
#define COUT_17_0X1685B 0x1FFFF //0x1685B polynomial CRC-17
#define COUT_15_0X4599 0x7FFF //0x4599 polynomial CRC-15

/* CRC[m] Control register */
#define DCRA_CTL 0x0020
#define ISZ_32 0 //default 32bit width DCRA_CIN_31:0
#define ISZ_16 BIT(4) //16bit width DCRA_CIN_15:0
#define ISZ_8 BIT(5) //8bit width DCRA_CIN_7:0
#define CRC_POL_32_ETHERNET 0 //default CRC-32-IEEE 802.3
#define CRC_POL_16_CCITT_FALSE_CRC16 BIT(0) //CCITT_FALSE_CRC16
#define CRC_POL_8_SAE_J1850 BIT(1) //SAE_J1850
#define CRC_POL_8_0X2F (3 << 0) // 0x2F polynomial
#define CRC_POL_32_0XF4ACFB13 BIT(2) //0xF4ACFB13 polynomial
#define CRC_POL_32_0X1EDC6F41 (5 << 0) //0x1EDC6F41 polynomial CRC-32 (Castagnoli)
#define CRC_POL_21_0X102899 (6 << 0) //0x102899 polynomial CRC-21
#define CRC_POL_17_0X1685B (7 << 0) //0x1685B polynomial CRC-17
#define CRC_POL_15_0X4599 BIT(3) //0x4599 polynomial CRC-15

/* CRC[m] Control register 2 */
#define DCRA_CTL2 0x0040
#define xorvalmode BIT(7) //EXOR ON of output data
#define bitswapmode BIT(6) //bit swap of output data
#define byteswapmode_00 0 //default no byte swap of output data
#define byteswapmode_01 BIT(4)
#define byteswapmode_10 BIT(5)
#define byteswapmode_11 (3 << 3)
#define xorvalinmode BIT(3) //EXOR ON of input data
#define bitswapinmode BIT(2) //bit swap of input data
#define byteswapinmode_00 0 //default no byte swap of input data
#define byteswapinmode_01 BIT(0)
#define byteswapinmode_10 BIT(1)
#define byteswapinmode_11 (3 << 0)

/************************************ KCRC registers ************************************/
/* Register base */
#define KCRC_BASE_ADDR(unit)       (0x19100000 + (unit * 0x10000))
/* Register offsets */
/* KCRC[m] data input register */
#define KCRC_DIN 0x0000

/* KCRC[m] data output register */
#define KCRC_DOUT 0x0080
#define DOUT_DEF 0xFFFFFFFF //initialize value

/* KCRC[m] control register */
#define KCRC_CTL 0x0090
#define PSIZE_32 (31 << 16) //default 32-bit
#define PSIZE_16 (15 << 16) //16-bit
#define PSIZE_8 (7 << 16) //8-bit
#define CMD0 BIT(8) //0: Mode N (Normal), 1: Mode R (output reflect)
#define CMD1 BIT(5) //0: Mode N (Normal), 1: Mode R (input reflect)
#define CMD2 BIT(4) //0: Mode M (MSB shift), 1: Mode R (LSB shift)
#define DW_32   ((0x1F) << 16)  //default 32-bit fix mode
#define DW_16   ((0xF)  << 16)  //16-bit fix mode
#define DW_8    ((0x7)  << 16)  //8-bit fix mode

/* KCRC[m] Polynomial register */
#define KCRC_POLY 0x00A0
#define KCRC_POL_32_ETHERNET 0x04C11DB7 //default 32-bit Ethernet CRC
#define KCRC_POL_16_CCITT 0x1021 //16-bit CCITT CRC
#define KCRC_POL_8_SAE_J1850 0x1D //8-bit SAE J1850 CRC
#define KCRC_POL_8_0X2F 0x2F //8-bit 0x2F CRC
#define KCRC_POL_32_CRC32C 0x1EDC6F41 //32-bit CRC32C (Castagnoli)

/* KCRC[m] XOR mask register */
#define KCRC_XOR 0x00B0
#define DEF_XOR 0xFFFFFFFF //default value

static uint32_t getRegister(uint8_t module, wcrc_unit_t unit, uint32_t offset)
{
    uint32_t base_addr;
    uint32_t reg_addr;

    if (unit < WCRC_00 || unit > WCRC_10)
    {
        printf("WCRC unit %d not exist!\n", unit);
        return 0;
    }

    if (module == WCRC_MODULE)
        base_addr = WCRC_BASE_ADDR(unit);
    if (module == CRC_MODULE)
        base_addr = CRC_BASE_ADDR(unit);
    if (module == KCRC_MODULE)
        base_addr = KCRC_BASE_ADDR(unit);

    reg_addr = base_addr + offset;

    return reg_addr;
}

static uint32_t readl(const uintptr_t address)
{
    return *((volatile unsigned int*)address);
}

static void writel(const uint32_t value, const uintptr_t address)
{
    *((volatile unsigned int*) address)  = value;
}

/************************************ WCRC functions ************************************/
static int wcrcPrepareIndependentCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrcStartIndependentCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrcPrepareE2eCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrcPrepareDataThrough(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrcStartE2eCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrcStartDataThrough(wcrc_instance_ctrl_t * const p_instance_ctrl);

static int wcrc_set_rtdma(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                         void * p_cfg_dma, uint32_t port,
                         uint8_t dma_direction, wcrc_fifo_ports_use_rtdma_t index);

static int crc_setting(wcrc_unit_t unit, crc_module_cfg_t const * const p_cfg);

static int kcrc_setting(wcrc_unit_t unit, kcrc_module_cfg_t const * const p_cfg);

static int crc_start(wcrc_unit_t unit,
                     crc_input_t const * const p_crc_input,
                     crc_output_t * p_crc_result);

static int kcrc_start(wcrc_unit_t unit,
                      crc_input_t const * const p_crc_input,
                      crc_output_t * p_crc_result);

static int wcrc_get_crc_data_size(wcrc_sub_module_t module, wcrc_cfg_t const * const p_cfg,
                                 uint32_t * p_crc_size);

int wcrcSetMode(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * p_cfg = p_instance_ctrl->p_cfg;

    switch (p_cfg->mode) {
        case INDEPENDENT_CRC_MODE:
            ret = wcrcPrepareIndependentCrcMode(p_instance_ctrl);
            break;
        case E2E_CRC_MODE:
            ret = wcrcPrepareE2eCrcMode(p_instance_ctrl);
            break;
        case DATA_THROUGH_MODE:
            ret = wcrcPrepareDataThrough(p_instance_ctrl);
            break;
        case E2E_DATA_THROUGH_MODE:
            break;
        case REGISTER_ACCESS_BY_CMD_MODE:
            break;
        case COMPARING_CRC_RESULT_MODE:
            break;
        default:
            printf("%s: mode not exist!\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

static int wcrcPrepareIndependentCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;
    uint8_t unit;
    crc_module_cfg_t  const * const p_crc_cfg  = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const p_kcrc_cfg = &p_cfg->kcrc_cfg;

    /* Independent CRC mode only returns 1 CRC data size 32 bits(4 bytes):
     *  Allocate memory to store CRC data.
     */
    unit = p_cfg->unit;
    switch (p_cfg->sub_module) {
        case CRC_SUB_MODULE:
            ret  = crc_setting(unit, p_crc_cfg);
            break;
        case KCRC_SUB_MODULE:
            ret  = kcrc_setting(unit, p_kcrc_cfg);
            break;
        case CRC_KCRC_SUB_MODULE:
            ret  = crc_setting(unit, p_crc_cfg);
            ret |= kcrc_setting(unit, p_kcrc_cfg);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

void wcrcRemoveBuffer(void * p_buf)
{
    if (p_buf != NULL)
        vPortFree(p_buf);
}

int wcrcStart(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;

    switch (p_cfg->mode) {
        case INDEPENDENT_CRC_MODE:
            ret = wcrcStartIndependentCrcMode(p_instance_ctrl);
            break;
        case E2E_CRC_MODE:
            ret = wcrcStartE2eCrcMode(p_instance_ctrl);
            break;
        case DATA_THROUGH_MODE:
            ret = wcrcStartDataThrough(p_instance_ctrl);
            break;
        case E2E_DATA_THROUGH_MODE:
            break;
        case REGISTER_ACCESS_BY_CMD_MODE:
            break;
        case COMPARING_CRC_RESULT_MODE:
            break;
        default:
            printf("%s: mode not exist!\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

static int wcrcStartIndependentCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;
    uint8_t unit = p_cfg->unit;
    crc_input_t const * const p_crc_input = &p_cfg->crc_cfg.input_cfg;
    crc_input_t const * const p_kcrc_input = &p_cfg->kcrc_cfg.input_cfg;
    crc_output_t * p_crc_data  = &p_instance_ctrl->crc_data[CRC_SUB_MODULE];
    crc_output_t * p_kcrc_data = &p_instance_ctrl->crc_data[KCRC_SUB_MODULE];

    switch (p_cfg->sub_module) {   
        case CRC_SUB_MODULE:
            ret = crc_start(unit, p_crc_input, p_crc_data);
            break;
        case KCRC_SUB_MODULE:
            ret = kcrc_start(unit, p_kcrc_input, p_kcrc_data);
            break;
        case CRC_KCRC_SUB_MODULE:
            ret  = crc_start(unit, p_crc_input, p_crc_data);
            ret |= kcrc_start(unit, p_kcrc_input, p_kcrc_data);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

int wcrc_set_callback(wcrc_sub_module_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                     void (* p_callback)(void *), void * const p_context)
{
    int ret = 0;

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        ret = -1;
        printf("%s: Invalid module\n", __func__);
    }

    wcrc_cfg_dma_t * p_cfg_dma[E2E_CRC_USE_2_DMA_CHAN];
    rDmacIrqCfg_t * irq_cfg[E2E_CRC_USE_2_DMA_CHAN];
    Context_t * p_usr_context[E2E_CRC_USE_2_DMA_CHAN];
    void * p_usr_temp;

    /* 4. WCRC setups user context */
    p_usr_temp                          = p_context;
    p_instance_ctrl->p_context[module]  = pvPortMalloc(sizeof(Context_t) * E2E_CRC_USE_2_DMA_CHAN);

    if (p_instance_ctrl->p_context[module] == NULL) {
        printf("%s: Allocate p_context FAILED!", __func__);
        return -1;
    }

    p_usr_context[E2E_PORT_DATA]    = p_instance_ctrl->p_context[module];
    p_usr_context[E2E_PORT_RESULT]  = p_usr_context[E2E_PORT_DATA] + 1;

    //printf("%d: >> 0x%x\n", module, p_usr_context[module]);

    p_cfg_dma[E2E_PORT_DATA]    = p_instance_ctrl->p_extend[module];
    p_cfg_dma[E2E_PORT_RESULT]  = p_cfg_dma[E2E_PORT_DATA] + 1;

    irq_cfg[E2E_PORT_DATA]            = &p_cfg_dma[E2E_PORT_DATA]->irq;
    irq_cfg[E2E_PORT_DATA]->p_context = NULL;
    p_usr_context[E2E_PORT_DATA]->ctx = irq_cfg[E2E_PORT_DATA];

    irq_cfg[E2E_PORT_RESULT]            = &p_cfg_dma[E2E_PORT_RESULT]->irq;
    irq_cfg[E2E_PORT_RESULT]->p_context = p_usr_temp;
    p_usr_context[E2E_PORT_RESULT]->ctx = irq_cfg[E2E_PORT_RESULT];

    /* 4.1. DMA TX: E2E_PORT_DATA */
    ret  = R_DMAC_RcarCallBackSet(irq_cfg[E2E_PORT_DATA],
                                 NULL,
                                 p_usr_context[E2E_PORT_DATA]);

    /* 4.2. DMA RX: E2E_PORT_RESULT */
    ret |= R_DMAC_RcarCallBackSet(irq_cfg[E2E_PORT_RESULT],
                                 p_callback,
                                 p_usr_context[E2E_PORT_RESULT]);

    return ret;
}

static int wcrc_start_e2e(wcrc_instance_ctrl_t * const p_instance_ctrl,
                         uint8_t module)
{
    int ret = 0;
    wcrc_cfg_dma_t * p_cfg_dma[E2E_CRC_USE_2_DMA_CHAN];
    uint8_t rtdma_unit[E2E_CRC_USE_2_DMA_CHAN], rtdma_ch[E2E_CRC_USE_2_DMA_CHAN];
    rDmacCfg_t * rtdma_cfg[E2E_CRC_USE_2_DMA_CHAN];

    p_cfg_dma[E2E_PORT_DATA]    = p_instance_ctrl->p_extend[module];
    p_cfg_dma[E2E_PORT_RESULT]  = p_cfg_dma[E2E_PORT_DATA] + 1;

    /* DMA TX direction */
    rtdma_unit[E2E_PORT_DATA]   =  p_cfg_dma[E2E_PORT_DATA]->irq.Unit;
    rtdma_ch[E2E_PORT_DATA]     =  p_cfg_dma[E2E_PORT_DATA]->irq.SubCh;
    rtdma_cfg[E2E_PORT_DATA]    = &p_cfg_dma[E2E_PORT_DATA]->cfg;

    /* DMA RX direction */
    rtdma_unit[E2E_PORT_RESULT] =  p_cfg_dma[E2E_PORT_RESULT]->irq.Unit;
    rtdma_ch[E2E_PORT_RESULT]   =  p_cfg_dma[E2E_PORT_RESULT]->irq.SubCh;
    rtdma_cfg[E2E_PORT_RESULT]  = &p_cfg_dma[E2E_PORT_RESULT]->cfg;

    /* 5. WCRC: start DMA */
    /* 5.1. Start DMA TX: E2E_PORT_DATA */
    ret |= R_DMAC_RcarDmacExec(rtdma_unit[E2E_PORT_DATA],
                              rtdma_ch[E2E_PORT_DATA],
                              rtdma_cfg[E2E_PORT_DATA], NULL);

    /* 5.2. Start DMA RX: E2E_PORT_RESULT */
    ret |= R_DMAC_RcarDmacExec(rtdma_unit[E2E_PORT_RESULT],
                              rtdma_ch[E2E_PORT_RESULT],
                              rtdma_cfg[E2E_PORT_RESULT], NULL);

    return ret;
}

static int wcrcStartE2eCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;

    switch (p_cfg->sub_module) {   
        case CRC_SUB_MODULE:
            ret = wcrc_start_e2e(p_instance_ctrl, CRC_SUB_MODULE);
            break;
        case KCRC_SUB_MODULE:
            ret = wcrc_start_e2e(p_instance_ctrl, KCRC_SUB_MODULE);
            break;
        case CRC_KCRC_SUB_MODULE:
            ret = wcrc_start_e2e(p_instance_ctrl, CRC_SUB_MODULE);

            ret |= wcrc_start_e2e(p_instance_ctrl, KCRC_SUB_MODULE);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

static int wcrc_start_data_through(wcrc_instance_ctrl_t * const p_instance_ctrl,
                                  uint8_t module,
                                  void (* p_callback)(void *),
                                  void * p_context)
{
    int ret = 0;
    uint8_t num_chan            = DATA_THROUGH_USE_2_DMA_CHAN;
    uint8_t port_data_input     = DATA_THROUGH_PORT_DATA_INPUT;
    uint8_t port_data_output    = DATA_THROUGH_PORT_DATA_OUTPUT;
    wcrc_cfg_dma_t * p_cfg_dma[num_chan];
    uint8_t rtdma_unit[num_chan], rtdma_ch[num_chan];
    rDmacIrqCfg_t * irq_cfg[num_chan];
    rDmacCfg_t * rtdma_cfg[num_chan];
    Context_t * p_usr_context[num_chan];
    void * p_usr_temp;

    p_usr_temp                          = p_context;
    p_instance_ctrl->p_context[module]  = pvPortMalloc(sizeof(Context_t) * num_chan);

    if (p_instance_ctrl->p_context[module] == NULL) {
        printf("%s: Allocate p_context FAILED!", __func__);
        return -1;
    }

    p_usr_context[port_data_input]     = (Context_t *)p_instance_ctrl->p_context[module] + 1;
    p_usr_context[port_data_output]    = (Context_t *)p_instance_ctrl->p_context[module];

    //printf("%d: >> 0x%x\n", module, p_usr_context[module]);

    p_cfg_dma[port_data_input]     = p_instance_ctrl->p_extend[module];
    p_cfg_dma[port_data_output]    = p_cfg_dma[port_data_input] + 1;

    /* DMA TX direction */
    rtdma_unit[port_data_input]   =  p_cfg_dma[port_data_input]->irq.Unit;
    rtdma_ch[port_data_input]     =  p_cfg_dma[port_data_input]->irq.SubCh;
    irq_cfg[port_data_input]      = &p_cfg_dma[port_data_input]->irq;
    rtdma_cfg[port_data_input]    = &p_cfg_dma[port_data_input]->cfg;
    // Store user context in pointer irq_cfg[port_data_input].
    irq_cfg[port_data_input]->p_context = p_usr_temp;
    // Store pointer irq_cfg[port_data_input] to context of IRQ.
    p_usr_context[port_data_input]->ctx = irq_cfg[port_data_input];

    /* DMA RX direction */
    rtdma_unit[port_data_output] =  p_cfg_dma[port_data_output]->irq.Unit;
    rtdma_ch[port_data_output]   =  p_cfg_dma[port_data_output]->irq.SubCh;
    irq_cfg[port_data_output]    = &p_cfg_dma[port_data_output]->irq;
    rtdma_cfg[port_data_output]  = &p_cfg_dma[port_data_output]->cfg;
    // Store user context in pointer irq_cfg[port_data_output].
    irq_cfg[port_data_output]->p_context = p_usr_temp;
    // Store pointer irq_cfg[port_data_output] to context of IRQ.
    p_usr_context[port_data_output]->ctx = irq_cfg[port_data_output];

    /* CRC: DMA TX */
    ret  = R_DMAC_RcarCallBackSet(irq_cfg[port_data_input],
                                 NULL,
                                 p_usr_context[port_data_input]);

    ret |= R_DMAC_RcarDmacExec(rtdma_unit[port_data_input],
                              rtdma_ch[port_data_input],
                              rtdma_cfg[port_data_input], NULL);

    /* CRC: DMA RX */
    ret |= R_DMAC_RcarCallBackSet(irq_cfg[port_data_output],
                                 p_callback,
                                 p_usr_context[port_data_output]);

    ret |= R_DMAC_RcarDmacExec(rtdma_unit[port_data_output],
                              rtdma_ch[port_data_output],
                              rtdma_cfg[port_data_output], NULL);

start_err:
    return ret;
}

static int wcrcStartDataThrough(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;
    void (* p_callback)(void *);
    void  * p_context;

    switch (p_cfg->sub_module) {   
        case CRC_SUB_MODULE:
            p_callback  = p_instance_ctrl->p_callback[CRC_SUB_MODULE];
            p_context   = p_instance_ctrl->p_context[CRC_SUB_MODULE];
            ret = wcrc_start_data_through(p_instance_ctrl, CRC_SUB_MODULE,
                                p_callback, p_context);
            break;
        case KCRC_SUB_MODULE:
            p_callback  = p_instance_ctrl->p_callback[KCRC_SUB_MODULE];
            p_context   = p_instance_ctrl->p_context[KCRC_SUB_MODULE];
            ret = wcrc_start_data_through(p_instance_ctrl, KCRC_SUB_MODULE,
                                p_callback, p_context);
            break;
        case CRC_KCRC_SUB_MODULE:
            p_callback  = p_instance_ctrl->p_callback[CRC_SUB_MODULE];
            p_context   = p_instance_ctrl->p_context[CRC_SUB_MODULE];
            ret = wcrc_start_data_through(p_instance_ctrl, CRC_SUB_MODULE,
                                p_callback, p_context);

            p_callback  = p_instance_ctrl->p_callback[KCRC_SUB_MODULE];
            p_context   = p_instance_ctrl->p_context[KCRC_SUB_MODULE];
            ret |= wcrc_start_data_through(p_instance_ctrl, KCRC_SUB_MODULE,
                                 p_callback, p_context);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            break;
    }

    return ret;
}

static int wcrc_check_rtdma_config(uint8_t module, wcrc_cfg_t const * const p_cfg,
                                  uint8_t rtdma_require)
{
    uint32_t * p_rtdma_inst;
    uint8_t num_rtdma_inst;
    int i;

    if (module == CRC_SUB_MODULE) {
        p_rtdma_inst    = p_cfg->crc_cfg.p_rtdma_inst;
        num_rtdma_inst  = p_cfg->crc_cfg.num_rtdma_inst;
    }

    if (module == KCRC_SUB_MODULE) {
        p_rtdma_inst    = p_cfg->kcrc_cfg.p_rtdma_inst;
        num_rtdma_inst  = p_cfg->kcrc_cfg.num_rtdma_inst;
    }

    if (!p_rtdma_inst) {
        printf("%s: p_rtdma_inst is NULL\n", __func__);
        return -1;
    }

    if (num_rtdma_inst < rtdma_require) {
        printf("%s: Require %d RTDMA channels\n", rtdma_require, __func__);
        return -1;
    }

    for (i = 0; i < rtdma_require; i++) {
        if (p_rtdma_inst[i] < RTDMA0_CH0 || p_rtdma_inst[i] > RTDMA3_CH15) {
            printf("%s: p_rtdma_inst[%d] is out of range rtdma_inst_t\n", __func__, i); 
            return -1;
        }
    }

    return 0;
}

static int wcrc_set_e2e_mode(uint8_t module, wcrc_cfg_t const * const p_cfg)
{
    int ret = 0;
    uint8_t reg_type = WCRC_MODULE;
    unsigned int reg_val, reg_addr;
    uint8_t unit = p_cfg->unit;
    crc_module_cfg_t const * const crc_cfg   = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const kcrc_cfg = &p_cfg->kcrc_cfg;

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        return ret;
    }

    ret = wcrc_check_rtdma_config(module, p_cfg, E2E_CRC_USE_2_DMA_CHAN);
    if (ret) {
        printf("%s: Invalid rtdma_config\n", __func__);
        ret = -1;
        return ret;
    }

    //Enable WCRC Stop Interrupt.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_INTEN(module));
    reg_val = STOP_DONE_IE;
    writel(reg_val, reg_addr);

    //1. Set CRC conversion size to once in WCRC_XXXX_CONV register.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_CONV(module));
    reg_val = p_cfg->conv_size[module];
    writel(reg_val, reg_addr);
    //printf("%d: conv=0x%x\n", module, readl(reg_addr));

    //2. Set initial CRC code value in WCRC_XXXX_INIT_CRC register.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_INIT_CRC(module));
    reg_val = 0xFFFFFFFFU;
    writel(reg_val, reg_addr);

    //3. (For CRC)  Set DCRAmCTL, DCRAmCTL2, DCRAmCOUT registers.
    //   (For KCRC) Set KCRCmCTL, KCRCmPOLY, KCRCmXOR, KCRCmDOUT registers.
    if (module == CRC_SUB_MODULE)
        crc_setting(unit, crc_cfg);
    else if (module == KCRC_SUB_MODULE)
        kcrc_setting(unit, kcrc_cfg);

    //4. Set in_en=1, trans_en=1, res_en=1 in WCRC_XXXX_EN register.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_EN(module));
    reg_val = IN_EN | TRANS_EN | RES_EN;
    writel(reg_val, reg_addr);

    //5. Set cmd_en=1 in WCRC_XXXX_CMDEN register
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_CMDEN(module));
    reg_val = CMD_EN;
    writel(reg_val, reg_addr);

    return ret;
}

static int get_width_input(wcrc_sub_module_t module, wcrc_cfg_t const * const p_cfg)
{
    uint32_t each_data_size = 0;
    crc_module_cfg_t  const * const p_crc_cfg  = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const p_kcrc_cfg = &p_cfg->kcrc_cfg;
    crc_input_t const * p_input_cfg;

    if (module == CRC_SUB_MODULE) {
        p_input_cfg = &p_crc_cfg->input_cfg;
    } else if (module == KCRC_SUB_MODULE) {
        p_input_cfg = &p_kcrc_cfg->input_cfg;
    } else {
        printf("%s: Invalid module\n", __func__);
        each_data_size = 0;
        return each_data_size;
    }

    /* Get each data size in byte */
    switch(p_input_cfg->bit_width) {
    case WIDTH_8_BIT:
        each_data_size = 8 / BIT_CONVERT_TO_BYTE;
        break;
    case WIDTH_16_BIT:
        each_data_size = 16 / BIT_CONVERT_TO_BYTE;
        break;
    case WIDTH_32_BIT:
        each_data_size = 32 / BIT_CONVERT_TO_BYTE;
        break;
    default:
        printf("%s: Input bit width INVALID\n", __func__);
        each_data_size = 0;
        return each_data_size;
    }

    return each_data_size;
}

static void wcrc_update_dest_dma_addr(wcrc_cfg_dma_t * p_cfg_dma, void* addr)
{

    p_cfg_dma->cfg.mDestAddr = (uintptr_t)addr;
}

int wcrcSetBufferAddress(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                        uint32_t addr)
{
    int ret = 0;
    crc_output_t *p_crc_data;
    wcrc_cfg_t const * p_cfg = p_instance_ctrl->p_cfg;

    switch(module) {
    case CRC_SUB_MODULE:
        p_crc_data = &p_instance_ctrl->crc_data[CRC_SUB_MODULE];
        break;
    case KCRC_SUB_MODULE:
        p_crc_data = &p_instance_ctrl->crc_data[KCRC_SUB_MODULE];
        break;
    default:
        printf("%s: Module INVALID\n", __func__);
        ret = 1;
        return ret;
    }

    p_crc_data->p_output_buffer = (void *)addr;

    /* Mode uses DMA needs to update destination address from user input buffer address.
     * (Refer Note1)
     */
    if (p_cfg->mode == E2E_CRC_MODE)
    {
        wcrc_cfg_dma_t * p_cfg_dma[E2E_CRC_USE_2_DMA_CHAN];

        p_cfg_dma[E2E_PORT_DATA]    = p_instance_ctrl->p_extend[module];
        p_cfg_dma[E2E_PORT_RESULT]  = p_cfg_dma[E2E_PORT_DATA] + 1;
        wcrc_update_dest_dma_addr(p_cfg_dma[E2E_PORT_RESULT], (void *)addr);
    }
    else if (p_cfg->mode == DATA_THROUGH_MODE)
    {

    }
    else if (p_cfg->mode == E2E_DATA_THROUGH_MODE)
    {

    }
    else if (p_cfg->mode == COMPARING_CRC_RESULT_MODE)
    {

    }

    return ret;
}

int wcrcGetCrcSize(wcrc_sub_module_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                  uint32_t * p_crc_size)
{
    int ret = 0;
    wcrc_cfg_t const * p_cfg   = p_instance_ctrl->p_cfg;

    switch(module) {
    case CRC_SUB_MODULE:
        ret = wcrc_get_crc_data_size(CRC_SUB_MODULE, p_cfg, p_crc_size);
        break;
    case KCRC_SUB_MODULE:
        ret = wcrc_get_crc_data_size(KCRC_SUB_MODULE, p_cfg, p_crc_size);
        break;
    default:
        printf("%s: Module INVALID\n", __func__);
        ret = -1;
    }

    return ret;
}

static int wcrc_get_crc_data_size(wcrc_sub_module_t module, wcrc_cfg_t const * const p_cfg,
                                 uint32_t * p_crc_size)
{
    int ret = 0;
    uint32_t crc_data_size, each_data_size, num_crc_data;
    uint32_t crc_conv_size, data_input_size, num_data_input;
    crc_module_cfg_t  const * const p_crc_cfg  = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const p_kcrc_cfg = &p_cfg->kcrc_cfg;
    crc_input_t const * p_input_cfg;

    if (p_cfg->mode == INDEPENDENT_CRC_MODE) {
        crc_data_size = 4;
        *p_crc_size = crc_data_size;
        return ret;
    }

    /* Get each data size in byte */
    each_data_size = get_width_input(module, p_cfg);

    /* E2E CRC mode returns number of CRC data size:
     *  Allocate memory to store CRC data (unit: byte)
     *      crc_data_size = data_input_size / crc_conv_size;
     */
    if (module == CRC_SUB_MODULE) {
        num_data_input = p_crc_cfg->input_cfg.num_data;
    } else if (module == KCRC_SUB_MODULE) {
        num_data_input = p_kcrc_cfg->input_cfg.num_data;
    } else {
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        crc_data_size = 0;
        *p_crc_size = crc_data_size;
        return ret;
    }

    crc_conv_size   = p_cfg->conv_size[module];
    data_input_size = each_data_size * num_data_input;
    num_crc_data    = data_input_size / crc_conv_size;
    crc_data_size   = num_crc_data * each_data_size;

    *p_crc_size = crc_data_size;
    return ret;
}

static int wcrc_prepare_e2e(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    uint32_t crc_data_size = 0;
    wcrc_cfg_t const * p_cfg   = p_instance_ctrl->p_cfg;
    crc_output_t * p_crc_data;
    wcrc_cfg_dma_t * p_cfg_dma[E2E_CRC_USE_2_DMA_CHAN];
    Context_t * p_usr_context[E2E_CRC_USE_2_DMA_CHAN];

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        printf("%s: Invalid module\n", __func__);
        return -1;
    }

    /* 1. WCRC setups E2E mode */
    ret = wcrc_set_e2e_mode(module, p_cfg);

    /* 2. WCRC allocates buffer for CRC data */
    p_crc_data                  = &p_instance_ctrl->crc_data[module];

    ret                         = wcrc_get_crc_data_size(module, p_cfg, &crc_data_size);
    if(ret) {
        printf("%s: Get CRC size FAIL!\n", __func__);
        return -1;
    }

    p_crc_data->num_data        = crc_data_size / get_width_input(module, p_cfg);

    if ((p_crc_data->num_data % NUM_DATA_ALIGN_AXI_BUS) != 0) {
        printf("%s: Data not align on AXI BUS!\n", __func__);
        return -1;
    }

    /* 3. WCRC setups DMA */
    p_instance_ctrl->p_extend[module]    = pvPortMalloc(sizeof(wcrc_cfg_dma_t) * E2E_CRC_USE_2_DMA_CHAN);

    if (p_instance_ctrl->p_extend[module] == NULL) {
        printf("%s: Allocate p_extend FAILED!", __func__);
        return -1;
    }

    p_cfg_dma[E2E_PORT_DATA]    = p_instance_ctrl->p_extend[module];
    p_cfg_dma[E2E_PORT_RESULT]  = p_cfg_dma[E2E_PORT_DATA] + 1;

    /* 3.1. DMA TX: E2E_PORT_DATA */
    ret |= wcrc_set_rtdma(module, p_instance_ctrl, p_cfg_dma[E2E_PORT_DATA],
                         PORT_DATA(module), MEM_TO_DEV, E2E_PORT_DATA_USE_RTDMA_INDEX_0);
    /* 3.2. DMA RX: E2E_PORT_RESULT */
    ret |= wcrc_set_rtdma(module, p_instance_ctrl, p_cfg_dma[E2E_PORT_RESULT],
                         PORT_RES(module), DEV_TO_MEM, E2E_PORT_RESULT_USE_RTDMA_INDEX_1);
    return ret;
}

static int wcrcPrepareE2eCrcMode(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * p_cfg = p_instance_ctrl->p_cfg;

    switch(p_cfg->sub_module) {
        case CRC_SUB_MODULE:
            ret  = wcrc_prepare_e2e(CRC_SUB_MODULE, p_instance_ctrl);
            break;
        case KCRC_SUB_MODULE:
            ret  = wcrc_prepare_e2e(KCRC_SUB_MODULE, p_instance_ctrl);
            break;
        case CRC_KCRC_SUB_MODULE:
            ret  = wcrc_prepare_e2e(CRC_SUB_MODULE, p_instance_ctrl);
            ret |= wcrc_prepare_e2e(KCRC_SUB_MODULE, p_instance_ctrl);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            return ret;
    }
    return ret;
}

static int wcrc_set_data_through_mode(uint8_t module, wcrc_cfg_t const * const p_cfg)
{
    int ret = 0;
    uint8_t reg_type = WCRC_MODULE;
    unsigned int reg_val, reg_addr;
    uint8_t unit = p_cfg->unit;
    crc_module_cfg_t const * const crc_cfg   = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const kcrc_cfg = &p_cfg->kcrc_cfg;

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        printf("%s: Invalid module\n", __func__);
        ret = -1;
        return ret;
    }

    //Enable WCRC Stop Interrupt.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_INTEN(module));
    reg_val = STOP_DONE_IE;
    writel(reg_val, reg_addr);

    //1. Set in_en=1, out_en=1 in WCRCm_XXXX_EN register.
    reg_addr = getRegister(reg_type, unit, WCRC_XXXX_EN(module));
    reg_val = IN_EN | OUT_EN;
    writel(reg_val, reg_addr);

    return ret;
}

static int wcrc_prepare_data_through(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    uint32_t data_input_readback_size = 0;
    wcrc_cfg_t const  * p_cfg   = p_instance_ctrl->p_cfg;
    crc_input_t const * p_input_cfg;
    crc_output_t * p_crc_data;
    wcrc_cfg_dma_t * p_cfg_dma[DATA_THROUGH_USE_2_DMA_CHAN];
    Context_t * p_usr_context[DATA_THROUGH_USE_2_DMA_CHAN];

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        printf("%s: Invalid module\n", __func__);
        return -1;
    }

    /* 1. WCRC setups Data Through mode */
    ret = wcrc_set_data_through_mode(module, p_cfg);

    /* 2. WCRC allocates buffer for data input readback */
    if (module == CRC_SUB_MODULE) {
        p_input_cfg = &p_cfg->crc_cfg.input_cfg;
    } else if (module == KCRC_SUB_MODULE) {
        p_input_cfg = &p_cfg->kcrc_cfg.input_cfg;
    }

    p_crc_data                  = &p_instance_ctrl->crc_data[module];
    p_crc_data->num_data        = p_input_cfg->num_data;
    data_input_readback_size    = p_crc_data->num_data * get_width_input(module, p_cfg);
    p_crc_data->p_output_buffer = pvPortMalloc(data_input_readback_size);

    if (p_crc_data->p_output_buffer == NULL) {
        printf("%s: Allocate p_output_buffer FAILED!", __func__);
        return -1;
    }

    //printf("%d: B> 0x%x\n", module, p_crc_data->p_output_buffer);

    /* 3. WCRC setups DMA */
    p_instance_ctrl->p_extend[module]    = pvPortMalloc(sizeof(wcrc_cfg_dma_t) * DATA_THROUGH_USE_2_DMA_CHAN);

    if (p_instance_ctrl->p_extend[module] == NULL) {
        printf("%s: Allocate p_extend FAILED!", __func__);
        return -1;
    }

    p_cfg_dma[DATA_THROUGH_PORT_DATA_INPUT]     = p_instance_ctrl->p_extend[module];
    p_cfg_dma[DATA_THROUGH_PORT_DATA_OUTPUT]    = p_cfg_dma[DATA_THROUGH_PORT_DATA_INPUT] + 1;

    /* DMA TX: PORT_DATA */
    ret |= wcrc_set_rtdma(module, p_instance_ctrl, p_cfg_dma[DATA_THROUGH_PORT_DATA_INPUT],
                         PORT_DATA(module), MEM_TO_DEV, DATA_THROUGH_PORT_DATA_IN_USE_RTDMA_INDEX_0);
    /* DMA RX: PORT_DATA */
    ret |= wcrc_set_rtdma(module, p_instance_ctrl, p_cfg_dma[DATA_THROUGH_PORT_DATA_OUTPUT],
                         PORT_DATA(module), DEV_TO_MEM, DATA_THROUGH_PORT_DATA_OUT_USE_RTDMA_INDEX_1);
    return ret;
}

static int wcrcPrepareDataThrough(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    int ret = 0;
    wcrc_cfg_t const * const p_cfg = p_instance_ctrl->p_cfg;

    switch(p_cfg->sub_module) {
        case CRC_SUB_MODULE:
            ret  = wcrc_prepare_data_through(CRC_SUB_MODULE, p_instance_ctrl);
            break;
        case KCRC_SUB_MODULE:
            ret  = wcrc_prepare_data_through(KCRC_SUB_MODULE, p_instance_ctrl);
            break;
        case CRC_KCRC_SUB_MODULE:
            ret  = wcrc_prepare_data_through(CRC_SUB_MODULE, p_instance_ctrl);
            ret |= wcrc_prepare_data_through(KCRC_SUB_MODULE, p_instance_ctrl);
            break;
        default:
            printf("%s: Invalid module\n", __func__);
            ret = -1;
            return ret;
    }

    return ret;
}

static int wcrc_get_dma_fifo(wcrc_sub_module_t module,
                            wcrc_unit_t unit, uint32_t port)
{
    uint32_t fifo_base, fifo_port_addr;

    /* Get FIFO address for DMA */
    fifo_base = WCRC_FIFO_ADDR(unit);
    fifo_port_addr = fifo_base + port;

    return fifo_port_addr;
}

static int wcrc_get_dma_request_id(wcrc_sub_module_t module,
                                  wcrc_unit_t unit, uint32_t port)
{
    uint32_t dma_req_id;

    /* TO DO: Check when to use request ID:
     * MID_RID_WCRC_OUT and MID_RID_WCRC_RES_MULTI
     */
    if (port == PORT_DATA(CRC_M) ||
        port == PORT_DATA(KCRC_M))
        dma_req_id = MID_RID_WCRC_IN(module, unit);

    if (port == PORT_CMD(CRC_M) ||
        port == PORT_CMD(KCRC_M))
        dma_req_id = MID_RID_WCRC_IN(module, unit);

    if (port == PORT_EXPT_DATA(CRC_M) ||
        port == PORT_EXPT_DATA(KCRC_M))
        dma_req_id = MID_RID_WCRC_IN(module, unit);

    if (port == PORT_RES(CRC_M) ||
        port == PORT_RES(KCRC_M))
        dma_req_id = MID_RID_WCRC_RES(module, unit);

    return dma_req_id;
}

static uint32_t get_dma_int_id(uint32_t dma_unit_chan)
{
    uint32_t int_id;

    switch(dma_unit_chan) {
        case 0x00:
        case 0x01:
            int_id = INTID_RTDMA0_CH0;
            break;
        case 0x02:
        case 0x03:
            int_id = INTID_RTDMA0_CH2; 
            break;
        case 0x04:
        case 0x05:
            int_id = INTID_RTDMA0_CH4; 
            break;
        case 0x06:
        case 0x07:
            int_id = INTID_RTDMA0_CH6;
            break;
        case 0x08:
        case 0x09:
            int_id = INTID_RTDMA0_CH8; 
            break;
        case 0x0A:
        case 0x0B:
            int_id = INTID_RTDMA0_CH10; 
            break;
        case 0x0C:
        case 0x0D:
            int_id = INTID_RTDMA0_CH12; 
            break;
        case 0x0E:
        case 0x0F:
            int_id = INTID_RTDMA0_CH14; 
            break;
        case 0x10:
        case 0x11:
            int_id = INTID_RTDMA1_CH0;
            break;
        case 0x12:
        case 0x13:
            int_id = INTID_RTDMA1_CH2; 
            break;
        case 0x14:
        case 0x15:
            int_id = INTID_RTDMA1_CH4; 
            break;
        case 0x16:
        case 0x17:
            int_id = INTID_RTDMA1_CH6;
            break;
        case 0x18:
        case 0x19:
            int_id = INTID_RTDMA1_CH8; 
            break;
        case 0x1A:
        case 0x1B:
            int_id = INTID_RTDMA1_CH10; 
            break;
        case 0x1C:
        case 0x1D:
            int_id = INTID_RTDMA1_CH12; 
            break;
        case 0x1E:
        case 0x1F:
            int_id = INTID_RTDMA1_CH14; 
            break;
        case 0x20:
        case 0x21:
            int_id = INTID_RTDMA2_CH0;
            break;
        case 0x22:
        case 0x23:
            int_id = INTID_RTDMA2_CH2; 
            break;
        case 0x24:
        case 0x25:
            int_id = INTID_RTDMA2_CH4; 
            break;
        case 0x26:
        case 0x27:
            int_id = INTID_RTDMA2_CH6;
            break;
        case 0x28:
        case 0x29:
            int_id = INTID_RTDMA2_CH8; 
            break;
        case 0x2A:
        case 0x2B:
            int_id = INTID_RTDMA2_CH10; 
            break;
        case 0x2C:
        case 0x2D:
            int_id = INTID_RTDMA2_CH12; 
            break;
        case 0x2E:
        case 0x2F:
            int_id = INTID_RTDMA2_CH14; 
            break;
        case 0x30:
        case 0x31:
            int_id = INTID_RTDMA3_CH0;
            break;
        case 0x32:
        case 0x33:
            int_id = INTID_RTDMA3_CH2; 
            break;
        case 0x34:
        case 0x35:
            int_id = INTID_RTDMA3_CH4; 
            break;
        case 0x36:
        case 0x37:
            int_id = INTID_RTDMA3_CH6;
            break;
        case 0x38:
        case 0x39:
            int_id = INTID_RTDMA3_CH8; 
            break;
        case 0x3A:
        case 0x3B:
            int_id = INTID_RTDMA3_CH10; 
            break;
        case 0x3C:
        case 0x3D:
            int_id = INTID_RTDMA3_CH12; 
            break;
        case 0x3E:
        case 0x3F:
            int_id = INTID_RTDMA3_CH14; 
            break;
        default:
            printf("%s: Invalid interrupt id\n", __func__);
            int_id = 0;
            break;
    }

    return int_id;
}

static int wcrc_get_dma_transf_unit_size_config(uint8_t transfer_size)
{
    int config = -1;

    switch (transfer_size) {
    case 4:
        config = DRV_RTDMAC_TRANS_UNIT_4BYTE;
        break;
    case 8:
        config = DRV_RTDMAC_TRANS_UNIT_8BYTE;
        break;
    case 16:
        config = DRV_RTDMAC_TRANS_UNIT_16BYTE;
        break;
    case 32:
        config = DRV_RTDMAC_TRANS_UNIT_32BYTE;
        break;
    case 64:
        config = DRV_RTDMAC_TRANS_UNIT_64BYTE;
        break;
    default:
        printf("%s: Invalid transfer size %d\n", __func__, transfer_size);
        config = -1;
        break;
    }

    return config;
}

static int wcrc_set_rtdma(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                         void * p_cfg_dma, uint32_t port,
                         uint8_t dma_direction, wcrc_fifo_ports_use_rtdma_t index)
{
    wcrc_cfg_t const * p_cfg = p_instance_ctrl->p_cfg;
    wcrc_unit_t unit = p_cfg->unit;
    uint32_t each_data_size = 0;
    uint32_t port_addr   = wcrc_get_dma_fifo(module, unit, port);
    uint32_t port_req_id = wcrc_get_dma_request_id(module, unit, port);

    wcrc_cfg_dma_t * p_wcrc_cfg_dma = (wcrc_cfg_dma_t *)p_cfg_dma;

    crc_module_cfg_t  const * const p_crc_cfg  = &p_cfg->crc_cfg;
    kcrc_module_cfg_t const * const p_kcrc_cfg = &p_cfg->kcrc_cfg;
    crc_input_t const * p_input_cfg;
    crc_output_t * p_data;
    uint8_t dma_tx_unit = 16, dma_rx_unit = 16;
    uint32_t dma_unit_chan;
    uint32_t * p_rtdma_inst;
    uint8_t num_rtdma_inst;

    if (module != CRC_SUB_MODULE &&
        module != KCRC_SUB_MODULE) {
        printf("%s: Invalid module\n", __func__);
        return -1;
    }

    /* Get each data size in byte */
    each_data_size = get_width_input(module, p_cfg);

    if (module == CRC_SUB_MODULE) {
        p_input_cfg     = &p_crc_cfg->input_cfg;
        p_rtdma_inst    = p_crc_cfg->p_rtdma_inst;
        num_rtdma_inst  = p_crc_cfg->num_rtdma_inst;
    } else if (module == KCRC_SUB_MODULE) {
        p_input_cfg  = &p_kcrc_cfg->input_cfg;
        p_rtdma_inst = p_kcrc_cfg->p_rtdma_inst;
        num_rtdma_inst  = p_kcrc_cfg->num_rtdma_inst;
    }

    if (index > num_rtdma_inst) {
        printf("%s: Invalid rtdma_inst_index %d\n", __func__, index);
        return -1;
    }

    p_wcrc_cfg_dma->cfg.mResource       = DRV_RTDMAC_RESOUCE_MAX;

    if (dma_direction == MEM_TO_DEV) {
        p_wcrc_cfg_dma->cfg.mSrcAddr        = (uintptr_t)p_input_cfg->p_input_buffer;
        p_wcrc_cfg_dma->cfg.mDestAddr       = port_addr;
        p_wcrc_cfg_dma->cfg.mTransferCount  = (p_input_cfg->num_data) * each_data_size / dma_tx_unit;
        p_wcrc_cfg_dma->cfg.mDMAMode        = DRV_DMAC_DMA_NO_DESCRIPTOR;
        p_wcrc_cfg_dma->cfg.mSrcAddrMode    = DRV_RTDMAC_ADDR_INCREMENTED;
        p_wcrc_cfg_dma->cfg.mDestAddrMode   = DRV_RTDMAC_ADDR_FIXED;
        p_wcrc_cfg_dma->cfg.mTransferUnit   = wcrc_get_dma_transf_unit_size_config(dma_tx_unit);
        p_wcrc_cfg_dma->cfg.mSourceRequest  = port_req_id;
        p_wcrc_cfg_dma->cfg.mLowSpeed       = DRV_RTDMAC_SPEED_NORMAL;
        p_wcrc_cfg_dma->cfg.mPrioLevel      = 0;
    } else if (dma_direction == DEV_TO_MEM) {
        p_data = &p_instance_ctrl->crc_data[module];
        p_wcrc_cfg_dma->cfg.mSrcAddr        = port_addr;
        /* Note1:
         * Because this func is called at device open but user calls API set buffer after device open.
         * -> Use wcrc_update_dest_dma_addr() at wcrcSetBufferAddress()
         * to update DMA destination address p_wcrc_cfg_dma->cfg.mDestAddr.
         */
        //p_wcrc_cfg_dma->cfg.mDestAddr       = (uintptr_t)p_data->p_output_buffer;
        p_wcrc_cfg_dma->cfg.mTransferCount  = p_data->num_data * each_data_size / dma_rx_unit;
        p_wcrc_cfg_dma->cfg.mDMAMode        = DRV_DMAC_DMA_NO_DESCRIPTOR;
        p_wcrc_cfg_dma->cfg.mSrcAddrMode    = DRV_RTDMAC_ADDR_FIXED;
        p_wcrc_cfg_dma->cfg.mDestAddrMode   = DRV_RTDMAC_ADDR_INCREMENTED;
        p_wcrc_cfg_dma->cfg.mTransferUnit   = wcrc_get_dma_transf_unit_size_config(dma_rx_unit);
        p_wcrc_cfg_dma->cfg.mSourceRequest  = port_req_id;
        p_wcrc_cfg_dma->cfg.mLowSpeed       = DRV_RTDMAC_SPEED_NORMAL;
        p_wcrc_cfg_dma->cfg.mPrioLevel      = 0;
    }

    dma_unit_chan                       = p_rtdma_inst[index] - 1;
    p_wcrc_cfg_dma->irq.Unit            = (0xF0 & dma_unit_chan) >> 4;
    p_wcrc_cfg_dma->irq.SubCh           = (0x0F & dma_unit_chan);
    p_wcrc_cfg_dma->irq.irq_channel     = get_dma_int_id(dma_unit_chan);

    //printf("dma_unit_chan %d\n", dma_unit_chan);
    //printf("%d: Unit  %d\n", module, p_wcrc_cfg_dma->irq.Unit);
    //printf("%d: Chan  %d\n", module, p_wcrc_cfg_dma->irq.SubCh);
    //printf("%d: INTID %d\n", module, p_wcrc_cfg_dma->irq.irq_channel);

    return 0;
}

static int wcrcCloseSubModule(wcrc_instance_ctrl_t * const p_instance_ctrl,
                             wcrc_sub_module_t module)
{
    void * p_buf;

    if (p_instance_ctrl->p_cfg->mode != INDEPENDENT_CRC_MODE)
    {
        /* Release RT-DMA */
        p_buf = p_instance_ctrl->p_extend[module];
        wcrcRemoveBuffer(p_buf);
        p_buf = p_instance_ctrl->p_context[module];
        wcrcRemoveBuffer(p_buf);
        p_instance_ctrl->p_extend[module]  = NULL;
        p_instance_ctrl->p_context[module]  = NULL;
    }
}

int wcrcClose(wcrc_instance_ctrl_t * const p_instance_ctrl)
{
    wcrc_sub_module_t sub_module = p_instance_ctrl->p_cfg->sub_module;

    if (sub_module == CRC_KCRC_SUB_MODULE)
    {
        wcrcCloseSubModule(p_instance_ctrl, CRC_SUB_MODULE);
        wcrcCloseSubModule(p_instance_ctrl, KCRC_SUB_MODULE);
    }
    else if (sub_module == CRC_SUB_MODULE || sub_module == KCRC_SUB_MODULE) {
        wcrcCloseSubModule(p_instance_ctrl, sub_module);
    }
    else
    {
        printf("%s: Invalid module\n", __func__);
        return -1;
    }

    return 0;
}

/************************************ CRC functions ************************************/
static int crc_setting(wcrc_unit_t unit, crc_module_cfg_t const * const p_cfg)
{
    unsigned int bit_width_input;
    unsigned int poly_set;
    unsigned int initial_set;
    unsigned int crc_features;
    uint8_t reg_type;

    /* Checking the Polynomial mode */
    switch (p_cfg->poly) {
    case POLY_32_ETHERNET:
        poly_set    = CRC_POL_32_ETHERNET;
        initial_set = COUT_32_ETHERNET;
        break;
    case POLY_16_CCITT_FALSE_CRC16:
        poly_set    = CRC_POL_16_CCITT_FALSE_CRC16;
        initial_set = COUT_16_CCITT_FALSE_CRC16;
        break;
    case POLY_8_SAE_J1850:
        poly_set    = CRC_POL_8_SAE_J1850;
        initial_set = COUT_8_SAE_J1850;
        break;
    case POLY_8_0X2F:
        poly_set    = CRC_POL_8_0X2F;
        initial_set = COUT_8_0X2F;
        break;
    case POLY_32_0XF4ACFB13:
        poly_set    = CRC_POL_32_0XF4ACFB13;
        initial_set = COUT_32_0XF4ACFB13;
        break;
    case POLY_32_0X1EDC6F41:
        poly_set    = CRC_POL_32_0X1EDC6F41;
        initial_set = COUT_32_0X1EDC6F41;
        break;
    case POLY_21_0X102899:
        poly_set    = CRC_POL_21_0X102899;
        initial_set = COUT_21_0X102899;
        break;
    case POLY_17_0X1685B:
        poly_set    = CRC_POL_17_0X1685B;
        initial_set = COUT_17_0X1685B;
        break;
    case POLY_15_0X4599:
        poly_set    = CRC_POL_15_0X4599;
        initial_set = COUT_15_0X4599;
        break;
    default:
        printf("%s: Polynomial mode INVALID\n", __func__);
        return -1;
    }

    /* Checking DCRA_CIN data size setting */
    switch (p_cfg->input_cfg.bit_width) {
    case WIDTH_8_BIT:
        bit_width_input = ISZ_8;
        break;
    case WIDTH_16_BIT:
        bit_width_input = ISZ_16;
        break;
    case WIDTH_32_BIT:
        bit_width_input = ISZ_32;
        break;
    default:
        printf("%s: Input bit width INVALID\n", __func__);
        return -1;
    }

    /* Checking DCRA_CTL2 setting */
    crc_features =  (p_cfg->is_out_exor     ? xorvalmode    : 0) |
                    (p_cfg->is_out_bitswap  ? bitswapmode   : 0) |
                    (p_cfg->is_in_exor      ? xorvalinmode  : 0) |
                    (p_cfg->is_in_bitswap   ? bitswapinmode : 0);

    switch (p_cfg->out_byteswap) {
    case BYTE_SWAP_00:
        crc_features |= byteswapmode_00;
        break;
    case BYTE_SWAP_01:
        crc_features |= byteswapmode_01;
        break;
    case BYTE_SWAP_10:
        crc_features |= byteswapmode_10;
        break;
    case BYTE_SWAP_11:
        crc_features |= byteswapmode_11;
        break;
    default:
        printf("%s: Out ByteSwap INVALID\n", __func__);
        return -1;
    }

    switch (p_cfg->in_byteswap) {
    case BYTE_SWAP_00:
        crc_features |= byteswapinmode_00;
        break;
    case BYTE_SWAP_01:
        crc_features |= byteswapinmode_01;
        break;
    case BYTE_SWAP_10:
        crc_features |= byteswapinmode_10;
        break;
    case BYTE_SWAP_11:
        crc_features |= byteswapinmode_11;
        break;
    default:
        printf("%s: In ByteSwap INVALID\n", __func__);
        return -1;
    }

    /* Set DCRA_CTL registers. */
    reg_type = CRC_MODULE;
    writel((bit_width_input | poly_set), getRegister(reg_type, unit, DCRA_CTL));

    /* Set DCRA_CTL2 registers. */
    writel(crc_features, getRegister(reg_type, unit, DCRA_CTL2));

    /* Set polynomial initial value to DCRA_COUT register. */
    writel(p_cfg->input_cfg.crc_seed, getRegister(reg_type, unit, DCRA_COUT));

    return 0;
}

static int crc_start(wcrc_unit_t unit,
                     crc_input_t const * const p_crc_input,
                     crc_output_t * p_crc_data)
{
    int ret, index;
    uint8_t reg_type;
    uint32_t * p_output_buffer;

    uint8_t  * p_8bit_input;
    uint16_t * p_16bit_input;
    uint32_t * p_32bit_input;

    /* Mark CRC operation is in-progress */
    p_crc_data->is_done = false;

    /* Independent mode return CRC data size 4-byte.
     * Get user buffer from application.
     * Only check if user buffer is valid or not.
     */
    if (p_crc_data->p_output_buffer == NULL) {
        printf("%s: Allocate FAILED!", __func__);
        return -1;
    }

    reg_type = CRC_MODULE;

    /* Set CRC seed value */
    writel(p_crc_input->crc_seed, getRegister(reg_type, unit, DCRA_COUT));

    switch (p_crc_input->bit_width) {
    case WIDTH_8_BIT:
        p_8bit_input = (uint8_t *)p_crc_input->p_input_buffer;

        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((uint32_t)(*p_8bit_input), getRegister(reg_type, unit, DCRA_CIN));
            p_8bit_input++;
        }
        break;
    case WIDTH_16_BIT:
        p_16bit_input = (uint16_t *)p_crc_input->p_input_buffer;

        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((uint32_t)(*p_16bit_input), getRegister(reg_type, unit, DCRA_CIN));
            p_16bit_input++;
        }
        break;
    case WIDTH_32_BIT:
        p_32bit_input = (uint32_t *)p_crc_input->p_input_buffer;
        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((*p_32bit_input), getRegister(reg_type, unit, DCRA_CIN));
            p_32bit_input++;
        }
        break;
    default:
        printf("%s: Width Input INVALID\n", __func__);
        return -1;
    }

    /* Store generate CRC data */
    p_crc_data->num_data = 1;
    p_output_buffer = (uint32_t *)p_crc_data->p_output_buffer;
    *p_output_buffer = readl(getRegister(reg_type, unit, DCRA_COUT));

    /* Mark CRC operation is done */
    p_crc_data->is_done = true;

    return 0;
}

/************************************ KCRC functions ************************************/
static int kcrc_setting(wcrc_unit_t unit, kcrc_module_cfg_t const * const p_cfg)
{
    unsigned int poly_set;
    unsigned int p_size;
    unsigned int kcrc_cmd;
    unsigned int input_dw;
    uint32_t reg_val;
    uint8_t reg_type;

    /* Checking the Polynomial mode */
    switch (p_cfg->poly) {
    case POLY_32_ETHERNET:
        p_size   = PSIZE_32;
        poly_set = KCRC_POL_32_ETHERNET;
        break;
    case POLY_16_CCITT_FALSE_CRC16:
        p_size   = PSIZE_16;
        poly_set = KCRC_POL_16_CCITT;
        break;
    case POLY_8_SAE_J1850:
        p_size   = PSIZE_8;
        poly_set = KCRC_POL_8_SAE_J1850;
        break;
    case POLY_8_0X2F:
        p_size   = PSIZE_8;
        poly_set = KCRC_POL_8_0X2F;
        break;
    case POLY_32_0X1EDC6F41:
        p_size   = PSIZE_32;
        poly_set = KCRC_POL_32_CRC32C;
        break;
    default:
        printf("%s: Polominal mode INVALID\n", __func__);
        return -1;
    }

    /* Checking KCRC Calculate Mode 0/1/2 */
    kcrc_cmd = (p_cfg->is_out_reflect   ? CMD0 : 0) |
               (p_cfg->is_in_reflect    ? CMD1 : 0);

    switch (p_cfg->shift_mode) {
    case MSB_SHIFT:
        kcrc_cmd &= ~CMD2;
        break;
    case LSB_SHIFT:
        kcrc_cmd |= CMD2;
        break;
    default:
        printf("%s: CMD2 mode INVALID\n", __func__);
        return -1;
    }

    switch (p_cfg->input_cfg.bit_width) {
    case WIDTH_8_BIT:
        input_dw = DW_8;
        break;
    case WIDTH_16_BIT:
        input_dw = DW_16;
        break;
    case WIDTH_32_BIT:
        input_dw = DW_32;
        break;
    default:
        printf("%s: Bit width input INVALID\n", __func__);
        return -1;
    }

    /* Set KCRC_CTL registers. */
    reg_type = KCRC_MODULE;
    writel((input_dw | kcrc_cmd), getRegister(reg_type, unit, KCRC_CTL));

    /* Set KCRC_POLY registers. */
    writel(poly_set, getRegister(reg_type, unit, KCRC_POLY));

    /* Set KCRC_XOR register. */
    writel(p_cfg->xor_mask_out, getRegister(reg_type, unit, KCRC_XOR));

    /* Set initial value to KCRC_DOUT register. */
    writel(p_cfg->input_cfg.crc_seed, getRegister(reg_type, unit, KCRC_DOUT));

    return 0;
}

static int kcrc_start(wcrc_unit_t unit,
                     crc_input_t const * const p_crc_input,
                     crc_output_t * p_kcrc_data)
{
    int ret, index;
    uint8_t reg_type;
    uint32_t * p_output_buffer;

    uint8_t  * p_8bit_input;
    uint16_t * p_16bit_input;
    uint32_t * p_32bit_input;

    /* Mark CRC operation is in-progress */
    p_kcrc_data->is_done = false;

    /* Independent mode return CRC data size 4-byte.
     * Get user buffer from application.
     * Only check if user buffer is valid or not.
     */
    if (p_kcrc_data->p_output_buffer == NULL) {
        printf("%s: Allocate FAILED!", __func__);
        return -1;
    }

    reg_type = KCRC_MODULE;

    /* Set CRC seed value */
    writel(p_crc_input->crc_seed, getRegister(reg_type, unit, KCRC_DOUT));

    switch (p_crc_input->bit_width) {
    case WIDTH_8_BIT:
        p_8bit_input = (uint8_t *)p_crc_input->p_input_buffer;

        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((uint32_t)(*p_8bit_input), getRegister(reg_type, unit, KCRC_DIN));
            p_8bit_input++;
        }
        break;
    case WIDTH_16_BIT:
        p_16bit_input = (uint16_t *)p_crc_input->p_input_buffer;

        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((uint32_t)(*p_16bit_input), getRegister(reg_type, unit, KCRC_DIN));
            p_16bit_input++;
        }
        break;
    case WIDTH_32_BIT:
        p_32bit_input = (uint32_t *)p_crc_input->p_input_buffer;

        /* Transfer data input to WCRC */
        for (index = 0; index < p_crc_input->num_data; index++) {
            writel((*p_32bit_input), getRegister(reg_type, unit, KCRC_DIN));
            p_32bit_input++;
        }
        break;
    default:
        printf("%s: Width Input INVALID\n", __func__);
        return -1;
    }

    /* Store generate KCRC data */
    p_kcrc_data->num_data = 1;
    p_output_buffer = (uint32_t *)p_kcrc_data->p_output_buffer;
    *p_output_buffer= readl(getRegister(reg_type, unit, KCRC_DOUT));

    /* Mark KCRC operation is done */
    p_kcrc_data->is_done = true;

    return 0;
}
