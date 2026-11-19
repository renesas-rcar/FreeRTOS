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
 * @defgroup X5H_MFIS_CHANNEL_ID X5H MFIS Channel ID
 * @{
 * @brief MFIS Channel ID list for X5H platform.
 *
 * The full list of MFIS Channel IDs is defined in X5H HWUM specs. Only reference this enum for compatibility with the driver interface.
 */

/**
 *  @brief         X5H MFIS channel IDs for CR-CA communication.
 *  @details       R-Car Gen5 X5H has 64 CR-CA MFIS channels from 0 to 63.
 *  @note          Each CR-CA channel ID has the same numeric value as the
 *                 corresponding CR-CR channel ID.
 */
typedef enum mfis_channel_cr_ca_id {
    MFIS_CR_CA_CH0 = 0x00, /*!< CR-CA MFIS Channel 0 */
    MFIS_CR_CA_CH1,        /*!< CR-CA MFIS Channel 1 */
    MFIS_CR_CA_CH2,        /*!< CR-CA MFIS Channel 2 */
    MFIS_CR_CA_CH3,        /*!< CR-CA MFIS Channel 3 */
    MFIS_CR_CA_CH4,        /*!< CR-CA MFIS Channel 4 */
    MFIS_CR_CA_CH5,        /*!< CR-CA MFIS Channel 5 */
    MFIS_CR_CA_CH6,        /*!< CR-CA MFIS Channel 6 */
    MFIS_CR_CA_CH7,        /*!< CR-CA MFIS Channel 7 */
    MFIS_CR_CA_CH8,        /*!< CR-CA MFIS Channel 8 */
    MFIS_CR_CA_CH9,        /*!< CR-CA MFIS Channel 9 */
    MFIS_CR_CA_CH10,       /*!< CR-CA MFIS Channel 10 */
    MFIS_CR_CA_CH11,       /*!< CR-CA MFIS Channel 11 */
    MFIS_CR_CA_CH12,       /*!< CR-CA MFIS Channel 12 */
    MFIS_CR_CA_CH13,       /*!< CR-CA MFIS Channel 13 */
    MFIS_CR_CA_CH14,       /*!< CR-CA MFIS Channel 14 */
    MFIS_CR_CA_CH15,       /*!< CR-CA MFIS Channel 15 */
    MFIS_CR_CA_CH16,       /*!< CR-CA MFIS Channel 16 */
    MFIS_CR_CA_CH17,       /*!< CR-CA MFIS Channel 17 */
    MFIS_CR_CA_CH18,       /*!< CR-CA MFIS Channel 18 */
    MFIS_CR_CA_CH19,       /*!< CR-CA MFIS Channel 19 */
    MFIS_CR_CA_CH20,       /*!< CR-CA MFIS Channel 20 */
    MFIS_CR_CA_CH21,       /*!< CR-CA MFIS Channel 21 */
    MFIS_CR_CA_CH22,       /*!< CR-CA MFIS Channel 22 */
    MFIS_CR_CA_CH23,       /*!< CR-CA MFIS Channel 23 */
    MFIS_CR_CA_CH24,       /*!< CR-CA MFIS Channel 24 */
    MFIS_CR_CA_CH25,       /*!< CR-CA MFIS Channel 25 */
    MFIS_CR_CA_CH26,       /*!< CR-CA MFIS Channel 26 */
    MFIS_CR_CA_CH27,       /*!< CR-CA MFIS Channel 27 */
    MFIS_CR_CA_CH28,       /*!< CR-CA MFIS Channel 28 */
    MFIS_CR_CA_CH29,       /*!< CR-CA MFIS Channel 29 */
    MFIS_CR_CA_CH30,       /*!< CR-CA MFIS Channel 30 */
    MFIS_CR_CA_CH31,       /*!< CR-CA MFIS Channel 31 */
    MFIS_CR_CA_CH32,       /*!< CR-CA MFIS Channel 32 */
    MFIS_CR_CA_CH33,       /*!< CR-CA MFIS Channel 33 */
    MFIS_CR_CA_CH34,       /*!< CR-CA MFIS Channel 34 */
    MFIS_CR_CA_CH35,       /*!< CR-CA MFIS Channel 35 */
    MFIS_CR_CA_CH36,       /*!< CR-CA MFIS Channel 36 */
    MFIS_CR_CA_CH37,       /*!< CR-CA MFIS Channel 37 */
    MFIS_CR_CA_CH38,       /*!< CR-CA MFIS Channel 38 */
    MFIS_CR_CA_CH39,       /*!< CR-CA MFIS Channel 39 */
    MFIS_CR_CA_CH40,       /*!< CR-CA MFIS Channel 40 */
    MFIS_CR_CA_CH41,       /*!< CR-CA MFIS Channel 41 */
    MFIS_CR_CA_CH42,       /*!< CR-CA MFIS Channel 42 */
    MFIS_CR_CA_CH43,       /*!< CR-CA MFIS Channel 43 */
    MFIS_CR_CA_CH44,       /*!< CR-CA MFIS Channel 44 */
    MFIS_CR_CA_CH45,       /*!< CR-CA MFIS Channel 45 */
    MFIS_CR_CA_CH46,       /*!< CR-CA MFIS Channel 46 */
    MFIS_CR_CA_CH47,       /*!< CR-CA MFIS Channel 47 */
    MFIS_CR_CA_CH48,       /*!< CR-CA MFIS Channel 48 */
    MFIS_CR_CA_CH49,       /*!< CR-CA MFIS Channel 49 */
    MFIS_CR_CA_CH50,       /*!< CR-CA MFIS Channel 50 */
    MFIS_CR_CA_CH51,       /*!< CR-CA MFIS Channel 51 */
    MFIS_CR_CA_CH52,       /*!< CR-CA MFIS Channel 52 */
    MFIS_CR_CA_CH53,       /*!< CR-CA MFIS Channel 53 */
    MFIS_CR_CA_CH54,       /*!< CR-CA MFIS Channel 54 */
    MFIS_CR_CA_CH55,       /*!< CR-CA MFIS Channel 55 */
    MFIS_CR_CA_CH56,       /*!< CR-CA MFIS Channel 56 */
    MFIS_CR_CA_CH57,       /*!< CR-CA MFIS Channel 57 */
    MFIS_CR_CA_CH58,       /*!< CR-CA MFIS Channel 58 */
    MFIS_CR_CA_CH59,       /*!< CR-CA MFIS Channel 59 */
    MFIS_CR_CA_CH60,       /*!< CR-CA MFIS Channel 60 */
    MFIS_CR_CA_CH61,       /*!< CR-CA MFIS Channel 61 */
    MFIS_CR_CA_CH62,       /*!< CR-CA MFIS Channel 62 */
    MFIS_CR_CA_CH63,       /*!< CR-CA MFIS Channel 63 */
    MFIS_CR_CA_CHMAX,      /*!< Maximum number of CR-CA MFIS channels */
} e_mfis_channel_cr_ca_id_t;

/**
 *  @brief         X5H MFIS channel IDs for CR-CR communication.
 *  @details       R-Car Gen5 X5H has 64 CR-CR MFIS channels from 0 to 63.
 *  @note          Each CR-CR channel ID has the same numeric value as the
 *                 corresponding CR-CA channel ID.
 */
typedef enum mfis_channel_cr_cr_id {
    MFIS_CR_CR_CH0  = MFIS_CR_CA_CH0,  /*!< CR-CR MFIS Channel 0 */
    MFIS_CR_CR_CH1  = MFIS_CR_CA_CH1,  /*!< CR-CR MFIS Channel 1 */
    MFIS_CR_CR_CH2  = MFIS_CR_CA_CH2,  /*!< CR-CR MFIS Channel 2 */
    MFIS_CR_CR_CH3  = MFIS_CR_CA_CH3,  /*!< CR-CR MFIS Channel 3 */
    MFIS_CR_CR_CH4  = MFIS_CR_CA_CH4,  /*!< CR-CR MFIS Channel 4 */
    MFIS_CR_CR_CH5  = MFIS_CR_CA_CH5,  /*!< CR-CR MFIS Channel 5 */
    MFIS_CR_CR_CH6  = MFIS_CR_CA_CH6,  /*!< CR-CR MFIS Channel 6 */
    MFIS_CR_CR_CH7  = MFIS_CR_CA_CH7,  /*!< CR-CR MFIS Channel 7 */
    MFIS_CR_CR_CH8  = MFIS_CR_CA_CH8,  /*!< CR-CR MFIS Channel 8 */
    MFIS_CR_CR_CH9  = MFIS_CR_CA_CH9,  /*!< CR-CR MFIS Channel 9 */
    MFIS_CR_CR_CH10 = MFIS_CR_CA_CH10, /*!< CR-CR MFIS Channel 10 */
    MFIS_CR_CR_CH11 = MFIS_CR_CA_CH11, /*!< CR-CR MFIS Channel 11 */
    MFIS_CR_CR_CH12 = MFIS_CR_CA_CH12, /*!< CR-CR MFIS Channel 12 */
    MFIS_CR_CR_CH13 = MFIS_CR_CA_CH13, /*!< CR-CR MFIS Channel 13 */
    MFIS_CR_CR_CH14 = MFIS_CR_CA_CH14, /*!< CR-CR MFIS Channel 14 */
    MFIS_CR_CR_CH15 = MFIS_CR_CA_CH15, /*!< CR-CR MFIS Channel 15 */
    MFIS_CR_CR_CH16 = MFIS_CR_CA_CH16, /*!< CR-CR MFIS Channel 16 */
    MFIS_CR_CR_CH17 = MFIS_CR_CA_CH17, /*!< CR-CR MFIS Channel 17 */
    MFIS_CR_CR_CH18 = MFIS_CR_CA_CH18, /*!< CR-CR MFIS Channel 18 */
    MFIS_CR_CR_CH19 = MFIS_CR_CA_CH19, /*!< CR-CR MFIS Channel 19 */
    MFIS_CR_CR_CH20 = MFIS_CR_CA_CH20, /*!< CR-CR MFIS Channel 20 */
    MFIS_CR_CR_CH21 = MFIS_CR_CA_CH21, /*!< CR-CR MFIS Channel 21 */
    MFIS_CR_CR_CH22 = MFIS_CR_CA_CH22, /*!< CR-CR MFIS Channel 22 */
    MFIS_CR_CR_CH23 = MFIS_CR_CA_CH23, /*!< CR-CR MFIS Channel 23 */
    MFIS_CR_CR_CH24 = MFIS_CR_CA_CH24, /*!< CR-CR MFIS Channel 24 */
    MFIS_CR_CR_CH25 = MFIS_CR_CA_CH25, /*!< CR-CR MFIS Channel 25 */
    MFIS_CR_CR_CH26 = MFIS_CR_CA_CH26, /*!< CR-CR MFIS Channel 26 */
    MFIS_CR_CR_CH27 = MFIS_CR_CA_CH27, /*!< CR-CR MFIS Channel 27 */
    MFIS_CR_CR_CH28 = MFIS_CR_CA_CH28, /*!< CR-CR MFIS Channel 28 */
    MFIS_CR_CR_CH29 = MFIS_CR_CA_CH29, /*!< CR-CR MFIS Channel 29 */
    MFIS_CR_CR_CH30 = MFIS_CR_CA_CH30, /*!< CR-CR MFIS Channel 30 */
    MFIS_CR_CR_CH31 = MFIS_CR_CA_CH31, /*!< CR-CR MFIS Channel 31 */
    MFIS_CR_CR_CH32 = MFIS_CR_CA_CH32, /*!< CR-CR MFIS Channel 32 */
    MFIS_CR_CR_CH33 = MFIS_CR_CA_CH33, /*!< CR-CR MFIS Channel 33 */
    MFIS_CR_CR_CH34 = MFIS_CR_CA_CH34, /*!< CR-CR MFIS Channel 34 */
    MFIS_CR_CR_CH35 = MFIS_CR_CA_CH35, /*!< CR-CR MFIS Channel 35 */
    MFIS_CR_CR_CH36 = MFIS_CR_CA_CH36, /*!< CR-CR MFIS Channel 36 */
    MFIS_CR_CR_CH37 = MFIS_CR_CA_CH37, /*!< CR-CR MFIS Channel 37 */
    MFIS_CR_CR_CH38 = MFIS_CR_CA_CH38, /*!< CR-CR MFIS Channel 38 */
    MFIS_CR_CR_CH39 = MFIS_CR_CA_CH39, /*!< CR-CR MFIS Channel 39 */
    MFIS_CR_CR_CH40 = MFIS_CR_CA_CH40, /*!< CR-CR MFIS Channel 40 */
    MFIS_CR_CR_CH41 = MFIS_CR_CA_CH41, /*!< CR-CR MFIS Channel 41 */
    MFIS_CR_CR_CH42 = MFIS_CR_CA_CH42, /*!< CR-CR MFIS Channel 42 */
    MFIS_CR_CR_CH43 = MFIS_CR_CA_CH43, /*!< CR-CR MFIS Channel 43 */
    MFIS_CR_CR_CH44 = MFIS_CR_CA_CH44, /*!< CR-CR MFIS Channel 44 */
    MFIS_CR_CR_CH45 = MFIS_CR_CA_CH45, /*!< CR-CR MFIS Channel 45 */
    MFIS_CR_CR_CH46 = MFIS_CR_CA_CH46, /*!< CR-CR MFIS Channel 46 */
    MFIS_CR_CR_CH47 = MFIS_CR_CA_CH47, /*!< CR-CR MFIS Channel 47 */
    MFIS_CR_CR_CH48 = MFIS_CR_CA_CH48, /*!< CR-CR MFIS Channel 48 */
    MFIS_CR_CR_CH49 = MFIS_CR_CA_CH49, /*!< CR-CR MFIS Channel 49 */
    MFIS_CR_CR_CH50 = MFIS_CR_CA_CH50, /*!< CR-CR MFIS Channel 50 */
    MFIS_CR_CR_CH51 = MFIS_CR_CA_CH51, /*!< CR-CR MFIS Channel 51 */
    MFIS_CR_CR_CH52 = MFIS_CR_CA_CH52, /*!< CR-CR MFIS Channel 52 */
    MFIS_CR_CR_CH53 = MFIS_CR_CA_CH53, /*!< CR-CR MFIS Channel 53 */
    MFIS_CR_CR_CH54 = MFIS_CR_CA_CH54, /*!< CR-CR MFIS Channel 54 */
    MFIS_CR_CR_CH55 = MFIS_CR_CA_CH55, /*!< CR-CR MFIS Channel 55 */
    MFIS_CR_CR_CH56 = MFIS_CR_CA_CH56, /*!< CR-CR MFIS Channel 56 */
    MFIS_CR_CR_CH57 = MFIS_CR_CA_CH57, /*!< CR-CR MFIS Channel 57 */
    MFIS_CR_CR_CH58 = MFIS_CR_CA_CH58, /*!< CR-CR MFIS Channel 58 */
    MFIS_CR_CR_CH59 = MFIS_CR_CA_CH59, /*!< CR-CR MFIS Channel 59 */
    MFIS_CR_CR_CH60 = MFIS_CR_CA_CH60, /*!< CR-CR MFIS Channel 60 */
    MFIS_CR_CR_CH61 = MFIS_CR_CA_CH61, /*!< CR-CR MFIS Channel 61 */
    MFIS_CR_CR_CH62 = MFIS_CR_CA_CH62, /*!< CR-CR MFIS Channel 62 */
    MFIS_CR_CR_CH63 = MFIS_CR_CA_CH63, /*!< CR-CR MFIS Channel 63 */
    MFIS_CR_CR_CHMAX,                  /*!< Maximum number of CR-CR MFIS channels */
} e_mfis_channel_cr_cr_id_t;

#ifdef __cplusplus
}
#endif
/** @} */  /* end of X5H_MFIS_Channel_ID */
#endif /* R_MFIS_CHANNEL_ID_H */