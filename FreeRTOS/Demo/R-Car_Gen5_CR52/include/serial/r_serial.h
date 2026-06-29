/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef SERIAL_H
#define SERIAL_H

/**
 * @defgroup Serial_Module Serial Module
 * @{
 * @brief This module provides functions to configure and control Serial.
 *
 * The Serial module allows for the configuration and control serial devices.
 * It provides functions to init port, put string, get char, put char and close.
 */

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "mfis/mfis_lock.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef UART_ID
#if (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
#define UART_ID SCIF1
#else
#define UART_ID HSCIF0
#endif // BOARD
#endif // UART_ID

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * @brief Serial devices.
 */
typedef enum e_serial_devices {
    SCIF0,                 ///< SCIF channel 0.
    SCIF1,                 ///< SCIF channel 1.
    SCIF2_UNSUPPORTED,     ///< SCIF channel 2.
    SCIF3,                 ///< SCIF channel 3.
    SCIF4,                 ///< SCIF channel 4.
    HSCIF0,                ///< HSCIF channel 0.
    HSCIF1,                ///< HSCIF channel 1.
    HSCIF2,                ///< HSCIF channel 2.
    HSCIF3,                ///< HSCIF channel 3.
} e_serial_devices_t;

/**
 * @brief Serial log output control states.
 */
typedef enum e_log_state {
    LOG_OFF,        /**< Disable serial log output. */
    LOG_ON          /**< Enable serial log output. */
} e_log_state_t;
/***********************************************************************************************************************
 * Public APIs
 **********************************************************************************************************************/
/**
 * @brief Port initialize.
 *
 * @param[in]  device - Serial channel.
 *
 * @retval 0 if successful.
 * 
 */
int32_t R_SERIAL_PortInit(e_serial_devices_t device);

/**
 * @brief Send string.
 *
 * @param[in]  buffer - Input string.
 * @param[in]  length - Length string.
 *
 * @retval 0 if successful.
 * 
 */
int32_t R_SERIAL_PutString(const unsigned char * buffer, unsigned short length);

/**
 * @brief Receive char.
 *
 * @param[in]  recv_char - Output char.
 *
 * @retval 0 if successful.
 *
 */
int32_t R_SERIAL_GetChar(unsigned char * recv_char);

/**
 * @brief Send char.
 *
 * @param[in]  send_char - Input char.
 *
 * @retval 0 if successful.
 *
 */
int32_t R_SERIAL_PutChar(unsigned char send_char);

/**
 * @brief Serial close.
 *
 *  @retval 0 if successful.
 * 
 */
int32_t R_SERIAL_Close();

/**
 * @brief Control the serial log output state at runtime.
 * 
 * @param state LOG_ON (Enable output), LOG_OFF (Mute output).
 * @return int32_t 0 if success
 */
int32_t R_SERIAL_SetLogState(e_log_state_t state);

/**
 * @brief Control synchronization for shared serial logging in AMP system.
 *
 * It's only effective when all cores using same port calling it.
 *
 * @param[in] lock_id   MFIS lock identifier.
 * @param[in] sync      Enable or disable synchronization.
 *
 * @retval None
 */
void R_SERIAL_AMP_LogSync(e_mfis_lock_id_t lock_id, bool sync);

/**
 * @brief Reconfigures the serial hardware
 * Directly updates console and PFC settings to allow dynamic port switching 
 * (e.g., SCIF1 to HSCIF0) during runtime.
 * 
 * @param device Target serial device channel.
 * @return int32_t 0 if successful.
 */
int32_t R_SERIAL_ReConfigure(e_serial_devices_t device);

#ifdef __cplusplus
}
#endif

/** @} */ // end of Serial_Module

#endif	/* SERIAL_H */
