/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef SSI_H
#define SSI_H

#include <stdint.h>
#include <stddef.h>

/*
 * SSICR
 */
#define	FORCE		(1u << 31)	/* Fixed */
#define	DMEN		(1u << 28)	/* DMA Enable */
#define	UIEN		(1u << 27)	/* Underflow Interrupt Enable */
#define	OIEN		(1u << 26)	/* Overflow Interrupt Enable */
#define	IIEN		(1u << 25)	/* Idle Mode Interrupt Enable */
#define	DIEN		(1u << 24)	/* Data Interrupt Enable */
#define	CHNL_4		(1u << 22)	/* Channels */
#define	CHNL_6		(2u << 22)	/* Channels */
#define	CHNL_8		(3u << 22)	/* Channels */
#define DWL_MASK	(7u << 19)	/* Data Word Length mask */
#define	DWL_8		(0u << 19)	/* Data Word Length */
#define	DWL_16		(1u << 19)	/* Data Word Length */
#define	DWL_18		(2u << 19)	/* Data Word Length */
#define	DWL_20		(3u << 19)	/* Data Word Length */
#define	DWL_22		(4u << 19)	/* Data Word Length */
#define	DWL_24		(5u << 19)	/* Data Word Length */
#define	DWL_32		(6u << 19)	/* Data Word Length */

/*
 * System word length
 */
#define	SWL_16		(1 << 16)	/* R/W System Word Length */
#define	SWL_24		(2 << 16)	/* R/W System Word Length */
#define	SWL_32		(3 << 16)	/* R/W System Word Length */
#define	SWL_48		(4 << 16)	/* R/W System Word Length */
#define	SWL_64		(5 << 16)	/* R/W System Word Length */
#define	SWL_128		(6 << 16)	/* R/W System Word Length */

#define	SCKD		(1 << 15)	/* Serial Bit Clock Direction */
#define	SWSD		(1 << 14)	/* Serial WS Direction */
#define	SCKP		(1 << 13)	/* Serial Bit Clock Polarity */
#define	SWSP		(1 << 12)	/* Serial WS Polarity */
#define	SDTA		(1 << 10)	/* Serial Data Alignment */
#define	PDTA		(1 <<  9)	/* Parallel Data Alignment */
#define	DEL			(1 <<  8)	/* Serial Data Delay */
#define	CKDV(v)		((v) <<  4)	/* Serial Clock Division Ratio */
#define	TRMD		(1 <<  1)	/* Transmit/Receive Mode Select */
#define	EN			(1 <<  0)	/* SSI Module Enable */

/*
 * SSISR
 */
#define	UIRQ		(1 << 27)	/* Underflow Error Interrupt Status */
#define	OIRQ		(1 << 26)	/* Overflow Error Interrupt Status */
#define	IIRQ		(1 << 25)	/* Idle Mode Interrupt Status */
#define	DIRQ		(1 << 24)	/* Data Interrupt Status Flag */
#define IDST        (1 << 0)        /* Idle Mode Status Flag */

/*
 * SSIWSR
 */
#define CONT		(1 << 8)	/* WS Continue Function */
#define MONO		(1 << 1)	/* TDM format / Monaural format */
#define WS_MODE		(1 << 0)	/* WS Mode */

typedef struct _ssi_conf_t {
	uint32_t cr_own;
	uint32_t cr_clk;
	uint32_t cr_mode;
	uint32_t cr_en;
	uint32_t cr_role;
	uint32_t wsr;
	uint8_t chan;
}ssi_conf_t;


typedef struct _audio_ssi_conf_t {
	ssi_conf_t* playback;
	ssi_conf_t* capture;
}audio_ssi_conf_t;

#define SSI0 (0xEC549000u)
#define SSICR0(channel) (SSI0 + ((channel) * 0x10000u))
#define SSI0SR(channel) (SSI0 + 0x04u + ((channel) * 0x10000u))
#define SSI0TDR(channel) (SSI0 + 0x08u + ((channel) * 0x10000u))
#define SSI0RDR(channel) (SSI0 + 0x0cu + ((channel) * 0x10000u))

#define SSIWSR0(channel) (SSI0 + 0x20u + ((channel) * 0x10000u))

#define SSIFMR0(channel) (SSI0 + 0x24u + ((channel) * 0x10000u))
#define SSIFSR0(channel) (SSI0 + 0x28u + ((channel) * 0x10000u))

#define SSICRE0(channel) (SSI0 + 0x30u + ((channel) * 0x10000u))

int ssi_init(ssi_conf_t *p_conf);
void ssi_start(ssi_conf_t *p_conf);
void ssi_stop(ssi_conf_t *p_conf);
void ssi_trans(ssi_conf_t *p_conf, uint32_t data);

void ssi_reset_reg(ssi_conf_t *p_conf);

#endif
