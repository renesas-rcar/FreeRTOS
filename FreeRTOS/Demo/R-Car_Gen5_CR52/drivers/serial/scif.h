/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef SCIF_H
#define SCIF_H

#include <stdint.h>

int32_t console_init(uint32_t port);
void console_apply_log_state(uint8_t enable);
void console_putc(char c);
int console_getc(unsigned char *p_char);
#endif	/* SCIF_H */
