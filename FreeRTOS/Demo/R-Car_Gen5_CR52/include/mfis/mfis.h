/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef MFIS_H
#define MFIS_H

#include <stdint.h>
#include "mfis/r_mfis_channel_id.h"

 typedef enum mfis_corepair
{
    MFIS_CORE_PAIR_CR_CR = 0,     /*!< CR, CR communication pair core */
    MFIS_CORE_PAIR_CR_CA = 1,     /*!< CR, CA communication pair core */
    MFIS_CORE_PAIR_CR_SCP = 2     /*!< CR, SCP communication pair core */
} e_mfis_corepair_t;

/* MFIS instance for each channel */
typedef enum mfis_type
{
    MFIS_TYPE_RECEVER = 0,
    MFIS_TYPE_SENDER = 1,
    MFIS_TYPE_OTHER = 2
} mfis_type_t;

struct mfis_channel
{
    uint8_t ch;
    uint16_t int_source;
    uint32_t recv_message;
    void (*cb_function)(void*);
    void* arg;
    mfis_type_t type;
    e_mfis_corepair_t core_pair;
};

int mfis_init(struct mfis_channel *ch);
int mfis_deinit(struct mfis_channel *ch);
int mfis_trigger_interrupt(struct mfis_channel *ch, uint16_t int_number);
int mfis_send_message(struct mfis_channel *ch, uint32_t value);
uint16_t mfis_get_int_source_num(struct mfis_channel *ch);
uint16_t mfis_get_message(struct mfis_channel *ch);

#endif // MFIS_H

