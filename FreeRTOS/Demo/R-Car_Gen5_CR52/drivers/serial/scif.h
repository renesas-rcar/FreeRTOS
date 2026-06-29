/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef SCIF_H
#define SCIF_H

#include <stdint.h>

/// Initialize scif .
/// \param[in]     port          port number
/// \return 1 on success, 0 on error.

#ifndef UART_BAUDRATE
#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
#define UART_BAUDRATE 115200
#else
#define UART_BAUDRATE 115200
#endif
#endif

uint32_t console_init(uint32_t port);
void console_apply_log_state(uint8_t enable);
void console_putc(char c);
int console_getc(unsigned char *p_char);
#endif	/* SCIF_H */
