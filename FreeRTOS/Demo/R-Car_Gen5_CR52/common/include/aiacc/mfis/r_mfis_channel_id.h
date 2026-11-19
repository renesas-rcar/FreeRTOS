/*
 *
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef R_MFIS_CONFIG_H
#define R_MFIS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup AIACC_MFIS_CHANNEL_ID AIACC MFIS Channel ID
 * @{
 * @brief MFIS Channel ID list for AIACC platform.
 *
 * The full list of MFIS Channel IDs is defined in AIACC HWUM specs. Only reference this enum for compatibility with the driver interface.
 */

/**
 *  @brief         AIACC MFIS channel IDs for CR-CA communication.
 *  @note          CR-CA MFIS communication is not supported on AIACC. This
 *                 enum contains only MFIS_CR_CA_CHMAX as an indicator; no
 *                 usable CR-CA channels are available.
 */
typedef enum mfis_channel_cr_ca_id {
    MFIS_CR_CA_CHMAX, /*!< Placeholder indicating CR-CA MFIS is unsupported */
} e_mfis_channel_cr_ca_id_t;

/**
 *  @brief         AIACC MFIS channel IDs for CR-CR communication.
 *  @details       The AIACC platform has 38 CR-CR MFIS channels, numbered 0 to 37.
 */
typedef enum mfis_channel_cr_cr_id {
    MFIS_CR_CR_CH0 = 0x00, /*!< CR-CR MFIS Channel 0 */
    MFIS_CR_CR_CH1,        /*!< CR-CR MFIS Channel 1 */
    MFIS_CR_CR_CH2,        /*!< CR-CR MFIS Channel 2 */
    MFIS_CR_CR_CH3,        /*!< CR-CR MFIS Channel 3 */
    MFIS_CR_CR_CH4,        /*!< CR-CR MFIS Channel 4 */
    MFIS_CR_CR_CH5,        /*!< CR-CR MFIS Channel 5 */
    MFIS_CR_CR_CH6,        /*!< CR-CR MFIS Channel 6 */
    MFIS_CR_CR_CH7,        /*!< CR-CR MFIS Channel 7 */
    MFIS_CR_CR_CH8,        /*!< CR-CR MFIS Channel 8 */
    MFIS_CR_CR_CH9,        /*!< CR-CR MFIS Channel 9 */
    MFIS_CR_CR_CH10,       /*!< CR-CR MFIS Channel 10 */
    MFIS_CR_CR_CH11,       /*!< CR-CR MFIS Channel 11 */
    MFIS_CR_CR_CH12,       /*!< CR-CR MFIS Channel 12 */
    MFIS_CR_CR_CH13,       /*!< CR-CR MFIS Channel 13 */
    MFIS_CR_CR_CH14,       /*!< CR-CR MFIS Channel 14 */
    MFIS_CR_CR_CH15,       /*!< CR-CR MFIS Channel 15 */
    MFIS_CR_CR_CH16,       /*!< CR-CR MFIS Channel 16 */
    MFIS_CR_CR_CH17,       /*!< CR-CR MFIS Channel 17 */
    MFIS_CR_CR_CH18,       /*!< CR-CR MFIS Channel 18 */
    MFIS_CR_CR_CH19,       /*!< CR-CR MFIS Channel 19 */
    MFIS_CR_CR_CH20,       /*!< CR-CR MFIS Channel 20 */
    MFIS_CR_CR_CH21,       /*!< CR-CR MFIS Channel 21 */
    MFIS_CR_CR_CH22,       /*!< CR-CR MFIS Channel 22 */
    MFIS_CR_CR_CH23,       /*!< CR-CR MFIS Channel 23 */
    MFIS_CR_CR_CH24,       /*!< CR-CR MFIS Channel 24 */
    MFIS_CR_CR_CH25,       /*!< CR-CR MFIS Channel 25 */
    MFIS_CR_CR_CH26,       /*!< CR-CR MFIS Channel 26 */
    MFIS_CR_CR_CH27,       /*!< CR-CR MFIS Channel 27 */
    MFIS_CR_CR_CH28,       /*!< CR-CR MFIS Channel 28 */
    MFIS_CR_CR_CH29,       /*!< CR-CR MFIS Channel 29 */
    MFIS_CR_CR_CH30,       /*!< CR-CR MFIS Channel 30 */
    MFIS_CR_CR_CH31,       /*!< CR-CR MFIS Channel 31 */
    MFIS_CR_CR_CH32,       /*!< CR-CR MFIS Channel 32 */
    MFIS_CR_CR_CH33,       /*!< CR-CR MFIS Channel 33 */
    MFIS_CR_CR_CH34,       /*!< CR-CR MFIS Channel 34 */
    MFIS_CR_CR_CH35,       /*!< CR-CR MFIS Channel 35 */
    MFIS_CR_CR_CH36,       /*!< CR-CR MFIS Channel 36 */
    MFIS_CR_CR_CH37,       /*!< CR-CR MFIS Channel 37 */
    MFIS_CR_CR_CHMAX,      /*!< Maximum number of CR-CR MFIS channels */
} e_mfis_channel_cr_cr_id_t;

#ifdef __cplusplus
}
#endif
/** @} */  /* end of AIACC_MFIS_Channel_ID */
#endif /* R_MFIS_CHANNEL_ID_H */