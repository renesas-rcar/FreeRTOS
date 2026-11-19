/*
 * Copyright (c) 2026 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/**
 * @defgroup MFIS_COMMON_PRIVATE MFIS COMMON PRIVATE
 * @{
 * @brief MFIS COMMON PRIVATE platform.
 *
 */

#ifndef R_MFIS_COMMON_PRIVATE_H
#define R_MFIS_COMMON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions shared by the MFIS v1 and v2 */
/*----------------------------------------------------------------------*/
#define SCP2RT0_IRQ_ID              (0xD6U)
#define SCP2RT1_IRQ_ID              (0xD7U)

#define MFIS_PROTECT_WRITE_DISABLE    (0xACC00001U)
#define MFIS_PROTECT_WRITE_ENABLE     (0xACC00000U)

#define MFIS_INVALID_CHANNEL_ERROR_INTSRC   (0xFFFFU)
#define MFIS_INVALID_CHANNEL_ERROR_MSG      (0xFFFFU)
/* Definitions shared by the MFIS v2 */
/*----------------------------------------------------------------------*/
#define MFISIMR_ACK_INT_BIT                 BIT(0U) /* bit 0 */
#define MFISIMR_RCV_INT_BIT                 BIT(1U) /* bit 1 */

#define MFISISR_ACK_INT                     BIT(0U)
#define MFISISR_RCV_INT_BIT                 BIT(1U)

#define MFISCHN_ACK_ST_PND_ACK_BIT          BIT(0U) /* bit 0 */
#define MFISCHN_SIG_SND_SIG_BIT             BIT(0U) /* bit 0 */

#define MFISICR_ACK_INT_BIT                 BIT(0U) /* bit 0 */
#define MFISICR_RCV_INT_BIT                 BIT(1U) /* bit 1 */

#define MFISCHN_CTRL_EN_AUTO_ACK_INTCLR     BIT(0U) /* bit 0 */
#define MFISCHN_RCV_ACK_RCV_ACK_BIT         BIT(0U) /* bit 0 */

#ifdef __cplusplus
}
#endif

#endif /* R_MFIS_COMMON_PRIVATE_H */
