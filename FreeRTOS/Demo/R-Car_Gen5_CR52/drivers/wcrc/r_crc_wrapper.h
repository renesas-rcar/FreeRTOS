/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef RENESAS_CRC_WRAPPER_H
#define RENESAS_CRC_WRAPPER_H

#include "wcrc/r_wcrc_common.h"

int wcrcSetMode(wcrc_instance_ctrl_t * const p_instance_ctrl);

int wcrcClose(wcrc_instance_ctrl_t * const p_instance_ctrl);

int wcrcStart(wcrc_instance_ctrl_t * const p_instance_ctrl);

int wcrc_set_callback(wcrc_sub_module_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                     void (* p_callback)(void *), void * const p_context);

int wcrcGetCrcSize(wcrc_sub_module_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                  uint32_t * p_crc_size);

int wcrcSetBufferAddress(uint8_t module, wcrc_instance_ctrl_t * const p_instance_ctrl,
                        uint32_t addr);

#endif /* RENESAS_CRC_WRAPPER_H */
