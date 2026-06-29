/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
#ifndef R_I2C_H
#define R_I2C_H

/**
 * @defgroup I2C_Module I2C Module
 * @{
 * @brief This module provides functions to configure and control I2C communication.
 *
 * The I2C module allows for the configuration and control of I2C communication between devices.
 * It provides functions to open, close, read, write, abort, slaveaddresset, statusget 
 * and manage I2C data transfer between master and slave devices.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "dmac/dmac_common.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** 
 * @brief Communication speed options.
 */
typedef enum e_i2c_master_rate
{
    I2C_MASTER_RATE_STANDARD = 100000, ///< 100 kHz.
    I2C_MASTER_RATE_FAST     = 400000, ///< 400 kHz.
    I2C_MASTER_RATE_FASTPLUS = 1000000 ///< 1 MHz.
} i2c_master_rate_t;

/** 
 * @brief Addressing mode options.
 */
typedef enum e_i2c_master_addr_mode
{
    I2C_MASTER_ADDR_MODE_7BIT  = 1,    ///< Use 7-bit addressing mode.
    I2C_MASTER_ADDR_MODE_10BIT = 2,    ///< Use 10-bit addressing mode.
} i2c_master_addr_mode_t;

/** 
 * @brief Callback events.
 */
typedef enum e_i2c_master_event
{
    I2C_MASTER_EVENT_ABORTED     = 1,  ///< A transfer was aborted.
    I2C_MASTER_EVENT_RX_COMPLETE = 2,  ///< A receive operation was completed successfully.
    I2C_MASTER_EVENT_TX_COMPLETE = 3   ///< A transmit operation was completed successfully.
} i2c_master_event_t;

/** 
 * @brief I2C callback parameter definition.
 */
typedef struct st_i2c_master_callback_args
{
    void const       * p_context;      ///< Pointer to user-provided context.
    i2c_master_event_t event;          ///< Event code.
} i2c_master_callback_args_t;

/** 
 * @brief I2C status indicators.
 */
typedef struct st_i2c_master_status
{
    bool open;                         ///< True if driver is open.
} i2c_master_status_t;

/** 
 * @brief I2C configuration block.
 */
typedef struct st_i2c_master_cfg
{
    uint8_t                channel;                           ///< Identifier recognizable by implementation.
    i2c_master_rate_t      rate;                              ///< Device's maximum clock rate from enum i2c_rate_t.
    uint32_t               slave;                             ///< The address of the slave device.
    i2c_master_addr_mode_t addr_mode;                         ///< Indicates how slave fields should be interpreted.
    uint8_t                ipl;                               ///< Interrupt priority level. Same for RXI, TXI, TEI and ERI.
    uint32_t               rxi_irq;                           ///< Receive IRQ number.
    uint32_t               txi_irq;                           ///< Transmit IRQ number.
    uint32_t               tei_irq;                           ///< Transmit end IRQ number.
    uint32_t               eri_irq;                           ///< Error IRQ number.
    uint32_t 		   dma_single;			      ///< DMA single mode, set 1 to use.
    uint32_t 		   dma_cont;			      ///< DMA continuous mode.

    /* Transfer API support */
    //transfer_instance_t const * p_transfer_tx;                ///< Transfer instance for I2C transmit. Set to NULL if unused.
    //transfer_instance_t const * p_transfer_rx;                ///< Transfer instance for I2C receive. Set to NULL if unused.

    void (* p_callback)(i2c_master_callback_args_t * p_args); ///< Pointer control software behavior to callback function.
    void const * p_context;                                   ///< Pointer control software behavior to the user-provided context.

    void const * p_extend;                                    ///< Implementation-specific configuration, including any hardware-specific configuration data.

    uint8_t sys_dmac_unit;       ///< SYS_DMAC2, SYS_DMAC3, etc.
    uint8_t sys_dmac_channel;        ///< DMAC_CH0, DMAC_CH1, etc.
    uint32_t sys_dmac_irq_id;   ///< INTID_SYSDMA2_CH0, etc.
} i2c_master_cfg_t;

/**
* @brief Pointer type for I2C master control.
*
* Used as a placeholder for I2C control structures, allowing functions
* to manage I2C communication without exposing implementation details.
*/
typedef void i2c_master_ctrl_t;

/** 
 * @brief I2C control structure. DO NOT INITIALIZE.
 */
typedef struct st_i2c_instance_ctrl
{
    i2c_master_cfg_t const  * p_cfg;    ///< Pointer to the configuration structure.
    i2c_master_rate_t      rate;	//Device's maximum clock rate from enum i2c_rate_t
    uint32_t                slave;      ///< The address of the slave device.
    i2c_master_addr_mode_t  addr_mode;  ///< Indicates how slave fields should be interpreted.
    uint32_t                open;       ///< Flag to determine if the device is open.
    uint32_t                * p_reg;    ///< Base register for this channel.

    uint32_t rxi_irq;                  ///< Receive IRQ number.
    uint32_t txi_irq;                  ///< Transmit IRQ number.
    uint32_t tei_irq;                  ///< Transmit end IRQ number.

    /* Current transfer information. */
    uint8_t * p_buff;                   ///< Holds the data associated with the transfer.
    uint32_t  total;                    ///< Holds the total number of data bytes to transfer.
    uint32_t  remain;                   ///< Tracks the remaining data bytes to transfer.
    uint32_t  loaded;                   ///< Tracks the number of data bytes written to the register.

    uint8_t addr_low;                   ///< Holds the last address byte to issue.
    uint8_t addr_high;                  ///< Holds the first address byte to issue in 10-bit mode.
    uint8_t addr_total;                 ///< Holds the total number of address bytes to transfer.
    uint8_t addr_remain;                ///< Tracks the remaining address bytes to transfer.
    uint8_t addr_loaded;                ///< Tracks the number of address bytes written to the register.

    volatile bool read;                 ///< Holds the direction of the data byte transfer.
    volatile bool restart;              ///< Holds whether or not the restart should be issued when done.
    volatile bool err;                  ///< Tracks whether or not an error occurred during processing.
    volatile bool restarted;            ///< Tracks whether or not a restart was issued during the previous transfer.
    volatile bool dma_single;		///< Use DMA single mode to transfer data.
    volatile bool dma_cont;		///< Use DNA continuous mode to transfer data.
    volatile bool do_dummy_read;        ///< Tracks whether a dummy read is issued on the first RX.
    volatile bool activation_on_rxi;    ///< Tracks whether the transfer is activated on RXI interrupt.
    volatile bool activation_on_txi;    ///< Tracks whether the transfer is activated on TXI interrupt.

    /* Pointer to callback and optional working memory */
    void (* p_callback)(i2c_master_callback_args_t *);      ///< Pointer to callback function.
    i2c_master_callback_args_t * p_callback_memory;         ///< Pointer to memory for callback parameters.

    /* Pointer to context to be passed into callback function */
    void const * p_context;             ///< Pointer to user-provided context data, which cannot be modified.
    
    /* Variables below are specific to DMA only */
    volatile bool dma_write_done;           ///< Write done flag when using DMA.
    volatile bool dma_read_done;            ///< Read done flag when using DMA.
    volatile bool dma_final_phase_read;     ///< Last byte read flag for DMA operations.
    
    rDmacIrqCfg_t *p_dmac_handle_irq;   ///< DMAC variables.
} i2c_instance_ctrl_t;

/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Open an I2C channel for communication.
 *
 * @param[in]  p_ctrl - Pointer to the I2C control structure, which contains information about the I2C channel.
 * @param[in]  p_cfg  - Pointer to the configuration structure containing settings for the I2C channel.
 *
 * @retval 0 if successful.
 */
int R_I2C_Open(i2c_master_ctrl_t * const p_ctrl, i2c_master_cfg_t const * const p_cfg);

/**
 * @brief Close the I2C channel and release associated resources.
 *
 * @param[in]  p_ctrl - Pointer to the I2C control structure, which was used to open the channel.
 *
 * @retval 0 if successful.
 */
int R_I2C_Close(i2c_master_ctrl_t * const p_ctrl);

/**
 * @brief Read data from an I2C slave starting at offset 0x00.
 *
 * @param[in]  p_ctrl  - Pointer to the I2C control structure.
 * @param[out] p_dest  - Pointer to the buffer where received data will be stored.
 * @param[in]  bytes   - Number of bytes to read from the I2C bus.
 * @param[in]  restart - Flag indicating whether to issue a restart condition after the current transaction.
 *
 * @retval 0 if successful.
 */
int R_I2C_Read(i2c_master_ctrl_t * const p_ctrl,
                         uint8_t * const           p_dest,
                         uint32_t const            bytes,
                         bool const                restart);

/**
 * @brief Read data from a specific register of an I2C slave device.
 *
 * This function reads a block of data starting from a specified register address
 * on the I2C slave and stores it in the provided destination buffer.
 *
 * @param[in]  p_ctrl     - Pointer to the I2C control structure.
 * @param[in]  slave_reg  - Register address on the slave device to start reading from.
 * @param[out] p_dest     - Pointer to the buffer where received data will be stored.
 * @param[in]  bytes      - Number of bytes to read from the slave device.
 *
 * @retval 0 if successful.
 */

int R_I2C_ReadRegMap(i2c_master_ctrl_t * const p_ctrl,
			uint32_t const            slave_reg,
			uint8_t * const           p_dest,
			uint32_t const            bytes);

/**
 * @brief Write data to the I2C bus from a source buffer.
 *
 * @param[in]  p_ctrl  - Pointer to the I2C control structure.
 * @param[in]  p_src   - Pointer to the buffer containing the data to be sent.
 * @param[in]  bytes   - Number of bytes to write to the I2C bus.
 * @param[in]  restart - Flag indicating whether to issue a restart condition after the current transaction.
 *
 * @retval 0 if successful.
 */

int R_I2C_Write(i2c_master_ctrl_t * const p_ctrl,
                          uint8_t * const           p_src,
                          uint32_t const            bytes,
                          bool const                restart);

/**
 * @brief Abort the current I2C transaction and reset the I2C interface.
 *
 * @param[in]  p_ctrl - Pointer to the I2C control structure.
 *
 * @retval 0 if successful.
 */
int R_I2C_Abort(i2c_master_ctrl_t * const p_ctrl);

/**
 * @brief Set the I2C slave address to be used for communication.
 *
 * @param[in]  p_ctrl   - Pointer to the I2C control structure.
 * @param[in]  slave    - The slave address to be set.
 * @param[in]  addr_mode - The addressing mode (7-bit or 10-bit).
 *
 * @retval 0 if successful.
 */
int R_I2C_SlaveAddressSet(i2c_master_ctrl_t * const    p_ctrl,
                                    uint32_t const               slave,
                                    i2c_master_addr_mode_t const addr_mode);

/**
 * @brief Set the callback function to be called when an I2C interrupt occurs.
 *
 * @param[in]  p_ctrl            - Pointer to the I2C control structure.
 * @param[in]  p_callback        - Pointer to the callback function.
 * @param[in]  p_context         - Pointer to the context structure that will be passed to the callback.
 * @param[in]  p_callback_memory - Pointer to the memory used for callback context (optional).
 *
 * @retval 0 if successful.
 */
int R_I2C_CallbackSet(i2c_master_ctrl_t * const          p_ctrl,
                                void (                           * p_callback)(i2c_master_callback_args_t *),
                                void const * const                 p_context,
                                i2c_master_callback_args_t * const p_callback_memory);

/**
 * @brief Get the current status of the I2C communication.
 *
 * @param[in]  p_ctrl   - Pointer to the I2C control structure.
 * @param[out] p_status - Pointer to a structure where the current I2C status will be stored.
 *
 * @retval 0 if successful.
 */
int R_I2C_StatusGet(i2c_master_ctrl_t * const p_ctrl, i2c_master_status_t * p_status);

#ifdef __cplusplus
}
#endif

/** @} */ // end of I2C_Module

#endif
