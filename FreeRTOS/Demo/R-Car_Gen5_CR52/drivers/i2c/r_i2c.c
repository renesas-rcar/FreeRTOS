/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "FreeRTOS.h"
#include "interrupts.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "state-manager/r_clock_domain_id.h"
#include "state-manager/r_state_manager.h"
#include "dmac/dmac_common.h"
#include "i2c/r_i2c.h"
#include "r_i2c_regs.h"
#include "r_i2c_private.h"
#include "board.h"

/* ==================== DEFINES ==================== */
#define I2C_OPEN                                (0x00000001ULL)
#define DMA_BUFFER_START_OFFSET                 1  ///< DMA buffer start offset (after PIO byte)
#define MAX_I2C_UNITS                           7
#define MAX_DMAC_UNITS                          4
#define ICMAR_MASK_READ                         ((uint32_t)0xFF)
#define ICMAR_MASK_WRITE                        ((uint32_t)0xFE)
#define ICMSR_MASK                              ((uint32_t)0x7F)
#define ICMCR_CLEAR                             ((uint32_t)0x80)

/* ==================== STATIC VARIABLES ==================== */
static int clock_id;

/* ==================== STATIC FUNCTION PROTOTYPES ==================== */
/* Helper functions */
static int32_t  loc_WaitMsrEvent(r_i2c_Unit_t Unit, uint32_t EventMask);
static uint32_t loc_ReadCommon(r_i2c_Unit_t Unit, uint32_t SlaveAddr, uint8_t *Bytes, uint32_t NumBytes);
static uint32_t rcar_dma_request_id(r_i2c_Unit_t Unit, bool is_read);

/* DMA functions */
static void rcar_i2c_dma_callback(void *p_context);
static int  rcar_i2c_dma_init(i2c_instance_ctrl_t *p_instance_ctrl);

/* Interrupt functions */
static void r_i2c_isr_handler(void * const p_context);
static void rcar_i2c_irq_recv(i2c_instance_ctrl_t *p_instance_ctrl, uint32_t msr);
static void rcar_i2c_irq_send(i2c_instance_ctrl_t *p_instance_ctrl, uint32_t msr);
static int  RCar_I2C_DisableGICInterrupt(r_i2c_Unit_t Unit);
static int  R_I2C_SetInterruptCallback(r_i2c_Unit_t Unit, IrqHandlerFn handler, void *ctx);
static int  R_I2C_Irq_handler(i2c_instance_ctrl_t *p_instance_ctrl);

/* Core I2C functions */
static int      RCar_I2C_Init(i2c_instance_ctrl_t *p_instance_ctrl);
static int      RCar_I2C_Close(i2c_instance_ctrl_t *p_instance_ctrl);
static uint32_t RCar_I2C_Write(i2c_instance_ctrl_t *p_instance_ctrl, uint8_t *Bytes, uint32_t NumBytes);
static uint32_t RCar_I2C_Read(r_i2c_Unit_t Unit, uint32_t SlaveAddr, uint8_t *Bytes, uint32_t NumBytes);
static uint32_t RCar_I2C_ReadRegMap(i2c_instance_ctrl_t *p_instance_ctrl, uint32_t SlaveReg, uint8_t *Bytes, uint32_t NumBytes);

/* Utility functions */
static int i2c_abort_seq_master(i2c_instance_ctrl_t * const p_instance_ctrl);

int R_I2C_Open(i2c_master_ctrl_t * const p_ctrl, i2c_master_cfg_t const * const p_cfg) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;

    /* Record the configuration on the device for use later */
    p_instance_ctrl->p_cfg             = p_cfg;
    p_instance_ctrl->rate              = p_cfg->rate;
    p_instance_ctrl->slave             = p_cfg->slave;
    p_instance_ctrl->addr_mode         = p_cfg->addr_mode;
    p_instance_ctrl->dma_single        = p_cfg->dma_single;
    p_instance_ctrl->dma_cont          = p_cfg->dma_cont;
    p_instance_ctrl->p_callback        = p_cfg->p_callback;
    p_instance_ctrl->p_context         = p_cfg->p_context;
    p_instance_ctrl->p_callback_memory = NULL;

    p_instance_ctrl->p_buff    = NULL;
    p_instance_ctrl->total     = 0U;
    p_instance_ctrl->remain    = 0U;
    p_instance_ctrl->loaded    = 0U;
    p_instance_ctrl->restart   = false;
    p_instance_ctrl->err       = false;
    p_instance_ctrl->restarted = false;
    p_instance_ctrl->open      = I2C_OPEN;
    /* For DMA only */
    p_instance_ctrl->p_dmac_handle_irq = NULL;

    RCar_I2C_Init(p_instance_ctrl);

    return 0;
}

int R_I2C_Close(i2c_master_ctrl_t * const p_ctrl) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;

    /* Abort an in-progress transfer with this device only */
    i2c_abort_seq_master(p_instance_ctrl);

    /* The device is now considered closed */
    p_instance_ctrl->open = 0U;

    if (p_instance_ctrl->p_cfg != NULL) 
    {
        RCar_I2C_Close(p_instance_ctrl);
    }

    return 0;
}

int R_I2C_Read(i2c_master_ctrl_t * const p_ctrl,
                         uint8_t * const           p_dest,
                         uint32_t const            bytes,
                         bool const                restart) 
{
    int result = -1;
    (void) restart; // unused parameter.
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    return RCar_I2C_Read(p_instance_ctrl->p_cfg->channel, p_instance_ctrl->p_cfg->slave, p_dest, bytes);
}

int R_I2C_ReadRegMap(i2c_master_ctrl_t * const p_ctrl,
                    uint32_t const            slave_reg,
                    uint8_t * const           p_dest,
                    uint32_t const            bytes) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    if (p_instance_ctrl->p_cfg->dma_single == true)
    {
        p_instance_ctrl->read = true;
        p_instance_ctrl->dma_read_done = false;
        p_instance_ctrl->dma_final_phase_read = false;
    }

    return RCar_I2C_ReadRegMap(p_instance_ctrl, slave_reg, p_dest, bytes);
}

int R_I2C_Write(i2c_master_ctrl_t * const p_ctrl,
                uint8_t * const           p_src,
                uint32_t const            bytes,
                bool const                restart) 
{
    (void) restart;
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    if (p_instance_ctrl->p_cfg->dma_single == true)
    {
        p_instance_ctrl->read = false;
        p_instance_ctrl->dma_write_done = false;
    }
    
    return RCar_I2C_Write(p_instance_ctrl, p_src, bytes);
}

int R_I2C_Abort(i2c_master_ctrl_t * const p_ctrl) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    return i2c_abort_seq_master(p_instance_ctrl);
}

int R_I2C_SlaveAddressSet(i2c_master_ctrl_t * const    p_ctrl,
                        uint32_t const               slave,
                        i2c_master_addr_mode_t const addr_mode) 
{

    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;

    /* Sets the address of the slave device */
    p_instance_ctrl->slave = slave;

    /* Sets the mode of addressing */
    p_instance_ctrl->addr_mode = addr_mode;

    return 0;
}

int R_I2C_CallbackSet(i2c_master_ctrl_t * const          p_ctrl,
                    void (* p_callback) (i2c_master_callback_args_t *),
                    void const * const          p_context,
                    i2c_master_callback_args_t * const p_callback_memory) 
{

    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    p_instance_ctrl->p_callback        = p_callback;
    p_instance_ctrl->p_context         = p_context;
    p_instance_ctrl->p_callback_memory = p_callback_memory;

    /* Only enable interrupt if using DMA */
    if (p_instance_ctrl->p_cfg->dma_single == true) 
    {
        R_I2C_SetInterruptCallback(p_instance_ctrl->p_cfg->channel, r_i2c_isr_handler, (void *)p_context);
    }
    return 0;
}

int R_I2C_StatusGet(i2c_master_ctrl_t * const p_ctrl, i2c_master_status_t * p_status) 
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_ctrl;
    p_status->open = (I2C_OPEN == p_instance_ctrl->open);
    return 0;
}

static int32_t loc_WaitMsrEvent(r_i2c_Unit_t Unit, uint32_t EventMask)
{
    uint32_t val;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);

    /* Ignore reserved bits */
    EventMask &= 0x7f;

    do {
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7F;

        /* Check if the master has received a NACK response */
        uint32_t a = (uint32_t)val & ((uint32_t)R_I2C_MNR_BIT);
	 if ((val & R_I2C_MNR_BIT) != (uint32_t)0) {
            break;
        }
    } while (!(val & EventMask));

    return (val & (uint32_t)R_I2C_MNR_BIT) ? -1 : 0;
}

static int RCar_I2C_Init(i2c_instance_ctrl_t * p_instance_ctrl)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint8_t ret;

    switch (Unit) {
#if (BOARD == X5H_IRONHIDE) || (BOARD == MDP_X5H_HIL)
        case R_I2C_IF0:
            clock_id = X5H_CLOCK_ID_MDLC_I2C0;
            break;
        case R_I2C_IF1:
            clock_id = X5H_CLOCK_ID_MDLC_I2C1;
            break;
        case R_I2C_IF2:
            clock_id = X5H_CLOCK_ID_MDLC_I2C2;
            break;
        case R_I2C_IF3:
            clock_id = X5H_CLOCK_ID_MDLC_I2C3;
            break;
        case R_I2C_IF4:
            clock_id = X5H_CLOCK_ID_MDLC_I2C4;
            break;
        case R_I2C_IF5:
            clock_id = X5H_CLOCK_ID_MDLC_I2C5;
            break;
        case R_I2C_IF6:
            clock_id = X5H_CLOCK_ID_MDLC_I2C6;
            break;
        case R_I2C_IF7:
            clock_id = X5H_CLOCK_ID_MDLC_I2C7;
            break;
        case R_I2C_IF8:
            clock_id = X5H_CLOCK_ID_MDLC_I2C8;
            break;
#endif
        default:
            printf("[R_I2C_PRV_GetClockId] : Wrong I2C Unit %d\r\n", Unit);
            break;
    }

    ret = R_StateManager_ClockOn(clock_id);
    if (ret)
    {
        printf("Error: Failed to set clock id %d ON.\r\n", clock_id);
    }

    uint32_t I2C_ClockRate = p_instance_ctrl->p_cfg->rate;
    switch (I2C_ClockRate)
    {
        case 100000:
            /* Set Clock Control register */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICCCR, 0x1e);
            break;

        case 400000:
	    /* Set Clock Control register */
	    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICCCR, 0xae);
            break;

        case 1000000:
	    /* Set SCL Mask Control regiters (Variable Duty ratio only) */
	    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMPR, 0x13);

	    /* Set SCL High Control regiters (Variable Duty ratio only) */
	    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICHPR, 0x15);

	    /* Set SCL Low Control regiters (Variable Duty ratio only) */
	    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICLPR, 0x15);

	    /* Set Clock Control register 2 */
	    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICCCR2, 0x87);
            break;

        default:
            printf("Invalid I2C ClockRate\n");
            break;
    }

    /* Set First Bit Setup Cycle register (1st bit setup cycle = 17*Tcyc) */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICFBSCR, 0x07);

    if (p_instance_ctrl->p_cfg->dma_single == true)
    {
        if (p_instance_ctrl->p_dmac_handle_irq == NULL) 
        {            
            p_instance_ctrl->p_dmac_handle_irq = (rDmacIrqCfg_t *)pvPortMalloc(sizeof(rDmacIrqCfg_t));
            if (p_instance_ctrl->p_dmac_handle_irq == NULL) 
            {
                printf("ERROR: Malloc failed for I2C DMAC handle\n");
                return -1;
            }       
            memset(p_instance_ctrl->p_dmac_handle_irq, 0, sizeof(rDmacIrqCfg_t));
        }

        if (p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg == NULL) 
        {
            p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg = (rDmacCfg_t *)pvPortMalloc(sizeof(rDmacCfg_t));
            if (p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg == NULL) 
            {
                printf("ERROR: Malloc failed for I2C DMAC config\n");
                return -1;
            }
            memset(p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg, 0, sizeof(rDmacCfg_t));
        }

        ret = RCAR_DMAC_CTRL_INIT(p_instance_ctrl->p_cfg->dmac_unit, DRV_RTDMAC_PRIO_FIX);
        if (ret != 0) 
        {
            return -1;
        }

        p_instance_ctrl->p_dmac_handle_irq->Unit = p_instance_ctrl->p_cfg->dmac_unit;
        p_instance_ctrl->p_dmac_handle_irq->SubCh = p_instance_ctrl->p_cfg->dmac_channel;
        p_instance_ctrl->p_dmac_handle_irq->irq_channel = p_instance_ctrl->p_cfg->dmac_irq_id;
        p_instance_ctrl->p_dmac_handle_irq->p_context = p_instance_ctrl;

        // Allocate Context_t
        Context_t *p_usr_context = (Context_t *)pvPortMalloc(sizeof(Context_t));
        if (p_usr_context == NULL) 
        {
            printf("ERROR: Cannot malloc Context_t for DMA interrupt\n");
            return -1;
        }
        p_usr_context->ctx = p_instance_ctrl->p_dmac_handle_irq;

        ret = RCAR_DMAC_CALLBACK_SET(p_instance_ctrl->p_dmac_handle_irq, (void *)rcar_i2c_dma_callback, p_usr_context);
        if (ret != 0) {
            return -1;
        }

        vPortFree(p_usr_context);
    }
}

static uint32_t loc_ReadCommon(r_i2c_Unit_t Unit, uint32_t SlaveAddr,
                               uint8_t *Bytes, uint32_t NumBytes)
{
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t val;
    uint32_t i;
    int r;

    /* Wait for the slave address to be transmitted*/
    r = loc_WaitMsrEvent(Unit, R_I2C_MAT_BIT);
    if (r < 0) {
	printf("[loc_ReadCommon] loc_WaitMsrEvent : Return value(r) is %d.(Wait for the slave address to be transmitted) Failed(0)\r\n",r);
        return 0;
    }

    if (NumBytes == (uint32_t)1) {
        /* If there is only 1 byte to receive, generate a STOP
         * condition after the byte has been received */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x8A);

        /* Clear ICMSR_MAT and ICMSR_MDR bits to resume transfer of
         * data */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
        val &= (uint32_t)~(R_I2C_MAT_BIT | R_I2C_MDR_BIT);
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Wait for transfer to complete */
        r = loc_WaitMsrEvent(Unit, (uint32_t)R_I2C_MDR_BIT);
        if (r < 0) {
	    printf("[loc_ReadCommon] loc_WaitMsrEvent : Return value(r) is %d.(Wait for transfer to complete) Failed(0)\r\n",r);
            return -1;
        }

        /* Copy the byte into the buffer */
        Bytes[0] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD);
        
        /* Clear MDR */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
        val &= ~(uint32_t)R_I2C_MDR_BIT;
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Wait MST */
        r = loc_WaitMsrEvent(Unit, R_I2C_MST_BIT);
        if (r < 0) 
        {
            return -1;
        }

        /* Clear MST */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
        val &= ~(uint32_t)R_I2C_MST_BIT;
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        return 0;
    } else {

        /* Suspend data transfer */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x88);

        /* Clear ICMSR_MAT and ICMSR_MDR bits to resume transmission
         * of data */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
        val &= (uint32_t)~(R_I2C_MAT_BIT | R_I2C_MDR_BIT);

        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Wait for Data Empty event */
        r = loc_WaitMsrEvent(Unit, R_I2C_MDR_BIT);
        if (r < 0) {
	    printf("[loc_ReadCommon] loc_WaitMsrEvent : Return value(r) is %d.(Wait for Data Empty event 1) Failed(0)\r\n",r);
            return -1;
        }

        /* Copy the first byte into the buffer */
        Bytes[0] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD);

        i = 1;
        while (i < (NumBytes - (uint32_t)1)) {
            /* Clear ICMSR_MDR bit */
            val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
            val &= (uint32_t)~R_I2C_MDR_BIT;
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            /* Wait for Data Empty event */
            r = loc_WaitMsrEvent(Unit, R_I2C_MDR_BIT);
            if (r < 0) {
		printf("[loc_ReadCommon] loc_WaitMsrEvent : Return value(r) is %d.(Wait for Data Empty event 2) Failed(%u)\r\n",r,i);
                return i;
            }

            /* Copy the next byte into the buffer */
            Bytes[i++] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD);
        }

        /* Generate a STOP condition after transmission */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x8A);

        /* Clear ICMSR_MDR bit */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
        val &= (uint32_t)~R_I2C_MDR_BIT;
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Wait for transmission to complete */
        r = loc_WaitMsrEvent(Unit, R_I2C_MDR_BIT);
        if (r < 0) {
	    printf("[loc_ReadCommon] loc_WaitMsrEvent : Return value(r) is %d.(Wait for transmission to complete) Failed(%u)\r\n",r,i);
            return i;
        }

        /* Copy the last byte into the buffer */
        Bytes[i++] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD);

        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
        val &= ~(uint32_t)R_I2C_MDR_BIT;
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Wait for transmission to complete */
        r = loc_WaitMsrEvent(Unit, R_I2C_MST_BIT);
        if (r < 0) {
            return -1;
        }

        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
        val &= ~(uint32_t)R_I2C_MST_BIT; 
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        return 0;
    }
}

/*
 * Note: the slave address is 7 bits long, i.e. does not include the
 * direction bit.
 */
static uint32_t RCar_I2C_Write(i2c_instance_ctrl_t * p_instance_ctrl, uint8_t * Bytes,
                     uint32_t NumBytes)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uint32_t SlaveAddr = p_instance_ctrl->p_cfg->slave;
    p_instance_ctrl->p_buff = Bytes;
    p_instance_ctrl->total = NumBytes;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t val;
    int r;

    /* Clear Master Status register */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);

    /* Set Master Interrupt Enable register (MDEE=1, MATE=1)*/
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, R_I2C_MDE_BIT | R_I2C_MAT_BIT);

    /* Set Master Address register (slave addr and write mode) */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMAR, ((SlaveAddr << 1) & ICMAR_MASK_WRITE));

    /* Load the first byte into the shift register */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICTXD, Bytes[0]);

    do {
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMCR);
    } while ((val & R_I2C_FSDA_BIT) != (uint32_t)0);

    if (p_instance_ctrl->p_cfg->dma_single == true) 
    {
        if (rcar_i2c_dma_init(p_instance_ctrl) != 0)
        {
            printf("ERROR: DMA write initialization failed\n");
            return -1;
        }
        /* 1st byte is loaded to ICTXD register for sending to bus at next step */
        p_instance_ctrl->loaded = DMA_BUFFER_START_OFFSET;
        /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);
        return 0;
    }

    else
    {
        /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);

        /* Wait for the slave address to be transmitted*/
        r = loc_WaitMsrEvent(Unit, R_I2C_MAT_BIT);
        if (r < 0) {
            printf("[R_I2C_Write] loc_WaitMsrEvent : Return value(r) is %d.(Wait for the slave address to be transmitted) Failed(0)\r\n",r);
            return -1;
        }

        if (NumBytes == 1) {
            /* If there is only 1 byte to transmit, generate a STOP
            * condition after transmission */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x8A);

            /* Clear ICMSR_MAT and ICMSR_MDE bits to resume transmission
            * of data */
            val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~(R_I2C_MAT_BIT | R_I2C_MDE_BIT);
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            /* Wait for transmission to complete */
            r = loc_WaitMsrEvent(Unit, R_I2C_MST_BIT);
            if (r < 0) {
            printf("[R_I2C_Write] loc_WaitMsrEvent :Return value(r) is %d.(Wait for transmission to complete) Failed(0)\r\n",r);
                return -1;
            } else {
                val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
                val &= ~(uint32_t)R_I2C_MST_BIT;
                R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);
                return 0;
            }
        } else {
            int i;

            /* Clear ESG bit in ICMCR reg */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x88);

            /* Clear ICMSR_MAT and ICMSR_MDE bits to resume transmission
            * of data */
            val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~(R_I2C_MAT_BIT | R_I2C_MDE_BIT);
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            /* Wait for Data Empty event */
            r = loc_WaitMsrEvent(Unit, R_I2C_MDE_BIT);
            if (r < 0) {
            printf("[R_I2C_Write] loc_WaitMsrEvent : Return value(r) is %d.(Wait for Data Empty event 1) Failed(0)\r\n",r);
                return -1;
            }

            i = 1;
            while (i < NumBytes) {
                /* Load the next byte into the shift register */
                R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICTXD, Bytes[i++]);

                /* Clear ICMSR_MDE bit */
                val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
                val &= ~R_I2C_MDE_BIT;
                R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

                /* Wait for Data Empty event */
                r = loc_WaitMsrEvent(Unit, R_I2C_MDE_BIT);
                if (r < 0) {
            printf("[R_I2C_Write] loc_WaitMsrEvent : Return value(r) is %d.(Wait for Data Empty event 2) Failed(%u)\r\n",r,--i);
                    return --i;
                }
            }

            /* Generate a STOP condition after transmission */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x8A);

            /* Clear ICMSR_MDE bit */
            val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~R_I2C_MDE_BIT;
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            /* Wait for transmission to complete */
            r = loc_WaitMsrEvent(Unit, R_I2C_MST_BIT);
            if (r < 0) {
            printf("[R_I2C_Write] loc_WaitMsrEvent : Return value(r) is %d.(Wait for transmission to complete) Failed(%u)\r\n",r,--i);
                return --i;
            } else {
                val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & ICMSR_MASK;
                val &= ~(uint32_t)R_I2C_MST_BIT;
                R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);
                return 0;
            }
        }
    }
}

// Read from slave at specified register offset
static uint32_t RCar_I2C_ReadRegMap(i2c_instance_ctrl_t * p_instance_ctrl, uint32_t SlaveReg,
                          uint8_t *Bytes, uint32_t NumBytes)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uint32_t SlaveAddr = p_instance_ctrl->p_cfg->slave;
    p_instance_ctrl->p_buff = Bytes;
    p_instance_ctrl->total = NumBytes;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t val;
    int r;

    /* Clear Master Status register */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);

    /* Set Master Interrupt Enable register (MDEE=1, MATE=1)*/
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, R_I2C_MDE_BIT
                         | R_I2C_MDR_BIT | R_I2C_MAT_BIT);

    /* Set Master Address register (slave addr + 0x00 write mode) */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMAR, (SlaveAddr << 1) & ICMAR_MASK_WRITE);

    /* Load the slave register address into the shift register */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICTXD, SlaveReg);

    do {
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMCR);
    } while ((val & R_I2C_FSDA_BIT) != (uint32_t)0);
    
    if (p_instance_ctrl->p_cfg->dma_single == true) 
    {
        if (rcar_i2c_dma_init(p_instance_ctrl) != 0)
        {
            printf("ERROR: DMA read initialization failed\n");
            return -1;
        }
        /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);
        return 0;
    }

    else
    {
        /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);
        
        /* Wait for the slave address to be transmitted */
        r = loc_WaitMsrEvent(Unit, R_I2C_MAT_BIT);
        if (r < 0) {
        printf("[R_I2C_ReadRegMap] loc_WaitMsrEvent : Return value(r) is %d.(Wait for the slave address to be transmitted) Failed(0)\r\n",r);
            return -1;
        }
        /* Clear ESG bit in ICMCR reg */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, (uint32_t)0x88);

        /* Resume transmission (slave reg address) */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
        val &= (uint32_t)~(R_I2C_MAT_BIT | R_I2C_MDE_BIT);
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);
        /* Wait for the slave register address to be transmitted */
        r = loc_WaitMsrEvent(Unit, R_I2C_MDE_BIT);
        if (r < 0) {
        printf("[R_I2C_ReadRegMap] loc_WaitMsrEvent : Return value(r) is %d.(Wait for the slave register address to be transmitted) Failed(0)\r\n",r);
            return -1;
        }
        /* Change from Write mode to Read mode */
        /* Set Master Address register (slave addr + 0x01 read mode) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMAR, ((SlaveAddr << 1) | (uint32_t)0x01) & ICMAR_MASK_READ);

        /* Set again the ESG bit in ICMCR, because we want a repeated
        * START condition on the bus when the data tranfer is resumed */
        /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, (uint32_t)0x89);

        /* Clear ICMSR_MDE bits to resume transmission of data */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
        val &= (uint32_t)~R_I2C_MDE_BIT;
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        return loc_ReadCommon(Unit, SlaveAddr, Bytes, NumBytes);
    }
}

// Read from slave at default offset 0x00
static uint32_t RCar_I2C_Read(r_i2c_Unit_t Unit, uint32_t SlaveAddr, uint8_t *Bytes, uint32_t NumBytes)
{
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t val;

    /* Clear Master Status register */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);

    /* Set Master Interrupt Enable register (MDRE=1, MATE=1)*/
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, R_I2C_MDR_BIT | R_I2C_MAT_BIT);

    /* Set Master Address register (slave addr + 0x01 read mode) */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMAR, ((SlaveAddr << 1) + 1) & ICMAR_MASK_READ);

    do {
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMCR);
    } while (val & R_I2C_FSDA_BIT);

    /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);

    return loc_ReadCommon(Unit, SlaveAddr, Bytes, NumBytes);
}

static int RCar_I2C_Close(i2c_instance_ctrl_t *p_instance_ctrl)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint8_t ret;
    
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, 0);
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, ICMCR_CLEAR);

    ret = R_StateManager_ClockOff(clock_id);
    if (ret)
    {
        printf("Error: Failed to set clock id %d OFF.\r\n", clock_id);
        return ret;
    }

    if (p_instance_ctrl->p_cfg->dma_single == true)
    {
        if (p_instance_ctrl->p_dmac_handle_irq != NULL) 
        {
            /* Free p_dma_cfg */ 
            if (p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg != NULL) 
            {
                vPortFree(p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg);
                p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg = NULL;
            }
            
            /* Free p_dmac_handle_irq */ 
            vPortFree(p_instance_ctrl->p_dmac_handle_irq);
            p_instance_ctrl->p_dmac_handle_irq = NULL;
        }

        /* Disable GIC interrupt */
        RCar_I2C_DisableGICInterrupt(Unit);
    }

    return 0;
}

static void rcar_i2c_dma_callback(void *p_context)
{
    if (!p_context) {
        printf("Error: Invalid context callback DMA\n");
        return;
    }
    
    i2c_instance_ctrl_t *p_instance_ctrl = (i2c_instance_ctrl_t *)p_context;
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;

    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    /* Disable DMA transmission */
    R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICDMAER, 0); 

    if (p_instance_ctrl->read == false)
    {
        p_instance_ctrl->loaded = p_instance_ctrl->total;
    }
    return;
}

static uint32_t rcar_dma_request_id(r_i2c_Unit_t Unit, bool is_read)
{
    /* Currently, not support channel 0 */ 
    uint32_t rx_ids[] = 
    {
        R_I2C_IF0, MID_RID_I2C1_MST_RX, MID_RID_I2C2_MST_RX,
        MID_RID_I2C3_MST_RX, MID_RID_I2C4_MST_RX, MID_RID_I2C5_MST_RX,
        MID_RID_I2C6_MST_RX, MID_RID_I2C7_MST_RX, MID_RID_I2C8_MST_RX
    }; 

    /* Currently, not support channel 0 */ 
    uint32_t tx_ids[] = 
    {
        R_I2C_IF0, MID_RID_I2C1_MST_TX, MID_RID_I2C2_MST_TX,
        MID_RID_I2C3_MST_TX, MID_RID_I2C4_MST_TX, MID_RID_I2C5_MST_TX,
        MID_RID_I2C6_MST_TX, MID_RID_I2C7_MST_TX, MID_RID_I2C8_MST_TX
    }; 
    
    if (Unit >= R_I2C_IF1 && Unit <= R_I2C_IF8) 
    {
        return is_read ? rx_ids[Unit] : tx_ids[Unit];
    }
    
    printf("ERROR: I2C channel %d does not support DMA currently\n", Unit);
    return -1;
}

static int rcar_i2c_dma_init(i2c_instance_ctrl_t * p_instance_ctrl)
{
    volatile bool is_read = p_instance_ctrl->read;
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint8_t *buf;
    uint32_t len;
    int ret;

    if (is_read) 
    {
        /* Read mode
         * The last two bytes needs to be fetched using PIO in
         * order for the STOP phase to work.
         */
        buf = p_instance_ctrl->p_buff;
        len = p_instance_ctrl->total - 2;
    } 
    else 
    {
        /* Write mode
         * First byte in message was sent using PIO.
         */
        buf = p_instance_ctrl->p_buff + 1;
        len = p_instance_ctrl->total - 1;
    }

    if (len <= 0)
    {
        printf("ERROR: Data length is too small to use DMA.\n");
        return -1;
    }

    rDmacCfg_t *cfg = p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg;
    cfg->mSrcAddr = ((is_read) ? (i2c_base_addr + R_I2C_ICRXD) : (uintptr_t)(buf));
    cfg->mDestAddr = (is_read) ? (uintptr_t)buf : (i2c_base_addr + R_I2C_ICTXD);
    cfg->mSrcAddrMode = (is_read) ? DRV_RTDMAC_ADDR_FIXED : DRV_RTDMAC_ADDR_INCREMENTED;
    cfg->mDestAddrMode = (is_read) ? DRV_RTDMAC_ADDR_INCREMENTED : DRV_RTDMAC_ADDR_FIXED;
    cfg->mTransferCount = len;
    cfg->mDMAMode = DRV_DMAC_DMA_NO_DESCRIPTOR;
    cfg->mTransferUnit = DRV_RTDMAC_TRANS_UNIT_1BYTE;
    cfg->mResource = DRV_RTDMAC_RESOUCE_MAX;
    cfg->mLowSpeed = DRV_RTDMAC_SPEED_NORMAL;
    cfg->mPrioLevel = 0;
    cfg->mSourceRequest = rcar_dma_request_id(Unit, is_read);
    if (cfg->mSourceRequest == -1) 
    {
        printf("ERROR: Failed to get DMA request ID for channel %d\n", Unit);
        return -1;
    }
    
    return 0;
}

static void rcar_i2c_irq_recv(i2c_instance_ctrl_t *p_instance_ctrl, uint32_t msr)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    volatile bool read_done = p_instance_ctrl->dma_read_done;
    volatile bool final_phase_read = p_instance_ctrl->dma_final_phase_read;

    if ((msr & R_I2C_MAT_BIT) == R_I2C_MAT_BIT && (msr & R_I2C_MDR_BIT) == R_I2C_MDR_BIT)
    {
        /* Clear ESG */
        uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMCR) & 0xff;
        val &= ~R_I2C_ESG_BIT;   
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, val);

        /* Enable DMA transmit mode */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICDMAER, 0x02); 
        
        /* Clear MAT and MDR */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
        val &= ~R_I2C_MAT_BIT;  
        val &= ~R_I2C_MDR_BIT;  
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Enable DMAC for receive mode */
        RCAR_DMAC_EXEC(p_instance_ctrl->p_dmac_handle_irq->Unit, p_instance_ctrl->p_dmac_handle_irq->SubCh, p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg, 0);
        return;
    }

    else if (final_phase_read == false && read_done == false)
    {
        /* Make STOP condition */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x8A);
        
        /* Read ICRXD to get the next-to-last-byte and save it to buffer */
        p_instance_ctrl->p_buff[p_instance_ctrl->total - 2] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD) & (uint32_t)0xff;  

        /* Clear MDR */
        uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
        val &= ~R_I2C_MDR_BIT;  
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Next time interrupt will be the last byte for reading */
        p_instance_ctrl->dma_final_phase_read = true;
        return;
    }

    else if (final_phase_read == true && read_done == false)
    {
        /* Read ICRXD to get the last byte and save it to buffer */
        p_instance_ctrl->p_buff[p_instance_ctrl->total - 1] = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICRXD) & (uint32_t)0xff;
        
        /* Clear MDR */
        uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
        val &= ~R_I2C_MDR_BIT;   
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        /* Enable MST interrupt for getting ICMSR.MST = 1 event */
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMIER) & 0x7f;
        val |= R_I2C_MST_BIT;   
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, val);

        p_instance_ctrl->dma_read_done = true;
        return;
    }

    else if (read_done == true)
    {
        /* Disable all interrupt I2C */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, 0);
        
        /* Clear MST */
        uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
        val &= ~R_I2C_MST_BIT; 
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

        if (p_instance_ctrl->p_callback != NULL) 
        {
            i2c_master_callback_args_t args = 
            {
                .event = I2C_MASTER_EVENT_RX_COMPLETE,
                .p_context = p_instance_ctrl->p_context /* p_context is &g_i2c_device_ctrl_x */
            };
            p_instance_ctrl->p_callback(&args);
        }

        return;
    }
}

static void rcar_i2c_irq_send(i2c_instance_ctrl_t *p_instance_ctrl, uint32_t msr)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    volatile bool read = p_instance_ctrl->read;
    volatile bool write_done = p_instance_ctrl->dma_write_done;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);

    /* Handle both Read & Write */
    if ((msr & R_I2C_MAT_BIT) == R_I2C_MAT_BIT) 
    {
        /* Clear ESG */
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x88);
        
        /* Clear MAT, MDE */
        uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
        val &= ~R_I2C_MAT_BIT;
        val &= ~R_I2C_MDE_BIT; 
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);
        return; 
    }

    /* Read handle */
    if (read == true)
    {
        if ((msr & R_I2C_MDE_BIT) == R_I2C_MDE_BIT)
        {
            /* Change from Write mode to Read mode */
            /* Set Master Address register (slave addr + 0x01 read mode) */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMAR, ((p_instance_ctrl->p_cfg->slave << 1) | 0x01) & ICMAR_MASK_READ);

            /* Set again the ESG bit in ICMCR, because we want a repeated
            * START condition on the bus when the data tranfer is resumed */
            /* Set Master Control register (MDBS=1, MIE=1, ESG=1) */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, 0x89);

            /* Clear ICMSR_MDE bits to resume transmission of data */
            uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~R_I2C_MDE_BIT;
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);
            return;
        }
    }

    /* Write handle */
    else
    {
        if (p_instance_ctrl->loaded == DMA_BUFFER_START_OFFSET)
        {
            /* Setting DMA */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICDMAER, R_I2C_TMDMAE);
            RCAR_DMAC_EXEC(p_instance_ctrl->p_dmac_handle_irq->Unit, p_instance_ctrl->p_dmac_handle_irq->SubCh, p_instance_ctrl->p_dmac_handle_irq->p_dma_cfg, 0);
            return;
        }

        else if (p_instance_ctrl->loaded >= p_instance_ctrl->total && write_done == false) 
        {
            /* Clear MDE */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMCR, (uint32_t)0x8A);
            uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~R_I2C_MDE_BIT; 
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            p_instance_ctrl->dma_write_done = true;
            /* Enable MST interrupt */
            val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMIER) & 0xff;
            val |= R_I2C_MST_BIT;
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, val);
            return;
        }

        else if (write_done == true)
        {
            /* Clear MST */
            uint32_t val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & 0x7f;
            val &= ~R_I2C_MST_BIT;
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, val);

            /* Disable all I2C interrupt */
            R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, 0);
            
            if (p_instance_ctrl->p_callback != NULL) 
            {
                i2c_master_callback_args_t args = 
                {
                    .event = I2C_MASTER_EVENT_TX_COMPLETE,
                    .p_context = p_instance_ctrl->p_context /* p_context is &g_i2c_device_ctrl_x */
                };
                p_instance_ctrl->p_callback(&args);
            }
            return;
        }
    }
}

static int R_I2C_SetInterruptCallback(r_i2c_Unit_t Unit, IrqHandlerFn handler, void *ctx)
{
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t int_id;
    switch (Unit) {
	case R_I2C_IF0:
	    int_id = INTID_I2C_IF0;
	    break;
	case R_I2C_IF1:
	    int_id = INTID_I2C_IF1;
	    break;
	case R_I2C_IF2:
	    int_id = INTID_I2C_IF2;
	    break;
	case R_I2C_IF3:
	    int_id = INTID_I2C_IF3;
	    break;
	case R_I2C_IF4:
	    int_id = INTID_I2C_IF4;
	    break;
	case R_I2C_IF5:
	    int_id = INTID_I2C_IF5;
	    break;
	case R_I2C_IF6:
	    int_id = INTID_I2C_IF6;
	    break;
	case R_I2C_IF7:
	    int_id = INTID_I2C_IF7;
	    break;
	case R_I2C_IF8:
	    int_id = INTID_I2C_IF8;
	    break;
	default:
	    int_id = INTID_NO_EXIST;
	    printf("ERROR: IRQ FAILED - no INTID exist!\n");
        return -1;
	}
    /* Set Handler for Irq */
    Irq_SetupEntry(int_id, handler, ctx);

    /* Set priority for Irq */
    Irq_SetPriority(int_id, IPRIORITY(3));

    /* Enable Irq */
    Irq_Enable(int_id);

    return 0;
}

static int RCar_I2C_DisableGICInterrupt(r_i2c_Unit_t Unit)
{
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    uint32_t int_id;
    switch (Unit) {
	case R_I2C_IF0:
	    int_id = INTID_I2C_IF0;
	    break;
	case R_I2C_IF1:
	    int_id = INTID_I2C_IF1;
	    break;
	case R_I2C_IF2:
	    int_id = INTID_I2C_IF2;
	    break;
	case R_I2C_IF3:
	    int_id = INTID_I2C_IF3;
	    break;
	case R_I2C_IF4:
	    int_id = INTID_I2C_IF4;
	    break;
	case R_I2C_IF5:
	    int_id = INTID_I2C_IF5;
	    break;
	case R_I2C_IF6:
	    int_id = INTID_I2C_IF6;
	    break;
	case R_I2C_IF7:
	    int_id = INTID_I2C_IF7;
	    break;
	case R_I2C_IF8:
	    int_id = INTID_I2C_IF8;
	    break;
	default:
	    int_id = INTID_NO_EXIST;
	    printf("ERROR: IRQ FAILED - no INTID exist!\n");
        return -1;
	}

    /* Disable Irq */
    Irq_Disable(int_id);

    return 0;
}

static int R_I2C_Irq_handler(i2c_instance_ctrl_t * p_instance_ctrl)
{
    r_i2c_Unit_t Unit = p_instance_ctrl->p_cfg->channel;
    uintptr_t i2c_base_addr = R_I2C_PRV_GetRegbase(Unit);
    static uint32_t msr;
    uint32_t val = 0;

    /* Only handle interrupts that are currently enabled */
    msr = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMSR) & (uint32_t)0x7f;
    msr &= R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMIER);

    if (val == 0) {
        val = R_I2C_PRV_RegRead32(i2c_base_addr + R_I2C_ICMAR) & (uint32_t)0x1;
    }

    if ((msr & R_I2C_MAL_BIT) != 0) {
        /* Arbitration lost */
        printf("ERROR: I2C arbitration lost - auto STOP\n");
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, 0);
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);
    }

    if ((msr & R_I2C_MNR_BIT) != 0) {
        /* HW automatically sends STOP after received NACK */
        printf("ERROR: I2C NACK received - auto STOP\n");
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, R_I2C_MST_BIT);
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMIER, 0);
        R_I2C_PRV_RegWrite32(i2c_base_addr + R_I2C_ICMSR, 0);
    }

    if (val != 0) {
        rcar_i2c_irq_recv(p_instance_ctrl, msr);
    } else {
        rcar_i2c_irq_send(p_instance_ctrl, msr);
    }

    return 0;
}

static int i2c_abort_seq_master (i2c_instance_ctrl_t * const p_instance_ctrl) {
    // Todo: Implement later.
    (void) p_instance_ctrl;
    return 0;
}

static void r_i2c_isr_handler(void * const p_context)
{
    i2c_instance_ctrl_t * p_instance_ctrl = (i2c_instance_ctrl_t *) p_context;

    // Call to HAL driver to process data.
    R_I2C_Irq_handler(p_instance_ctrl);

    // if (p_instance_ctrl->p_callback != NULL) {
    //     p_instance_ctrl->p_callback(p_context);
    // }
}
