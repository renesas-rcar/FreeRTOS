/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "CMSIS_5/cmsis_rcar_gen5.h"
#include "scif.h"
#include "scif_private.h"

static uint32_t scif_base;

/* * Log output control constants. 
 * Used to toggle console_putc functionality. 
 */
#define SCIF_LOG_STATE_OFF              (0U)
#define SCIF_LOG_STATE_ON               (1U)

/* Registers */
#define SCSMR           0x00    /* Serial Mode Register */
#define SCBRR           0x04    /* Bit Rate Register */
#define SCSCR           0x08    /* Serial Control Register */
#define SCFTDR          0x0c    /* Transmit FIFO Data Register */
#define SCFSR           0x10    /* Serial Status Register */
#define SCFRDR          0x14    /* Receive FIFO Data Register */
#define SCFCR           0x18    /* FIFO Control Register */
#define SCFDR           0x1c    /* FIFO Data Count Register */
#define SCSPTR          0x20    /* Serial Port Register */
#define SCLSR           0x24    /* Line Status Register */
#define DL              0x30    /* Frequency Division Register */
#define CKS             0x34    /* Clock Select Register */
#define HSSRR           0x40    /* Sampling rate Register */

/* SCSMR (Serial Mode Register) */
#define SCSMR_C_A       BIT(7)  /* Communication Mode */
#define SCSMR_CHR       BIT(6)  /* 7-bit Character Length */
#define SCSMR_PE        BIT(5)  /* Parity Enable */
#define SCSMR_O_E       BIT(4)  /* Odd Parity */
#define SCSMR_STOP      BIT(3)  /* Stop Bit Length */
#define SCSMR_CKS1      BIT(1)  /* Clock Select 1 */
#define SCSMR_CKS0      BIT(0)  /* Clock Select 0 */
#define SCIF_SCSMR_INIT_DATA    ~((uint16_t)(SCSMR_CHR | SCSMR_PE | SCSMR_STOP | SCSMR_CKS1 | SCSMR_CKS0))

/* SCSCR (Serial Control Register) */
#define SCSCR_TEIE      BIT(11) /* Transmit End Interrupt Enable */
#define SCSCR_TIE       BIT(7)  /* Transmit Interrupt Enable */
#define SCSCR_RIE       BIT(6)  /* Receive Interrupt Enable */
#define SCSCR_TE        BIT(5)  /* Transmit Enable */
#define SCSCR_RE        BIT(4)  /* Receive Enable */
#define SCSCR_REIE      BIT(3)  /* Receive Error Interrupt Enable */
#define SCSCR_TOIE      BIT(2)  /* Timeout Interrupt Enable */
#define SCSCR_CKE1      BIT(1)  /* Clock Enable 1 */
#define SCSCR_CKE0      BIT(0)  /* Clock Enable 0 */
#define SCIF_SCSCR_INIT_DATA    (uint16_t)(SCSCR_TE | SCSCR_RE)

/* SCFCR (FIFO Control Register) */
#define SCFCR_RTRG1     BIT(7)  /* Receive FIFO Data Count Trigger 1 */
#define SCFCR_RTRG0     BIT(6)  /* Receive FIFO Data Count Trigger 0 */
#define SCFCR_TTRG1     BIT(5)  /* Transmit FIFO Data Count Trigger 1 */
#define SCFCR_TTRG0     BIT(4)  /* Transmit FIFO Data Count Trigger 0 */
#define SCFCR_MCE       BIT(3)  /* Modem Control Enable */
#define SCFCR_TFRST     BIT(2)  /* Transmit FIFO Data Register Reset */
#define SCFCR_RFRST     BIT(1)  /* Receive FIFO Data Register Reset */
#define SCFCR_LOOP      BIT(0)  /* Loopback Test */
#define SCIF_SCFCR_RESET_FIFO   (uint16_t)(SCFCR_TFRST | SCFCR_RFRST)

/* SCFSR (Serial Status Register) */
#define SCFSR_PER3      BIT(15) /* Parity Error Count 3 */
#define SCFSR_PER2      BIT(14) /* Parity Error Count 2 */
#define SCFSR_PER1      BIT(13) /* Parity Error Count 1 */
#define SCFSR_PER0      BIT(12) /* Parity Error Count 0 */
#define SCFSR_FER3      BIT(11) /* Framing Error Count 3 */
#define SCFSR_FER2      BIT(10) /* Framing Error Count 2 */
#define SCFSR_FER_1     BIT(9)  /* Framing Error Count 1 */
#define SCFSR_FER0      BIT(8)  /* Framing Error Count 0 */
#define SCFSR_ER        BIT(7)  /* Receive Error */
#define SCFSR_TEND      BIT(6)  /* Transmission ended */
#define SCFSR_TDFE      BIT(5)  /* Transmit FIFO Data Empty */
#define SCFSR_BRK       BIT(4)  /* Break Detect */
#define SCFSR_FER       BIT(3)  /* Framing Error */
#define SCFSR_PER       BIT(2)  /* Parity Error */
#define SCFSR_RDF       BIT(1)  /* Receive FIFO Data Full */
#define SCFSR_DR        BIT(0)  /* Receive Data Ready */

/* SCLSR (Line Status Register) on (H)SCIF */
#define SCLSR_TO        BIT(2)  /* Timeout */
#define SCLSR_ORER      BIT(0)  /* Overrun Error */

#define HSCIF_DL_DIV1           (uint16_t)(1U << 0U)
#define HSCIF_CKS_CKS           (uint16_t)(1U << 15U)
#define HSCIF_CKS_XIN           (uint16_t)(1U << 14U)
#define HSCIF_CKS_SC_CLK_EXT    ~((uint16_t)(HSCIF_CKS_CKS | HSCIF_CKS_XIN))

#define HSCIF_HSSRR_SRE         (uint16_t)(1U << 15U)
#define HSCIF_HSSRR_SRCYC       (uint16_t)(0x1FU << 0U)
#define HSCIF_HSSRR_SRCYC8      (uint16_t)(7U << 0U)    /* Sampling rate 8-1 */
#define HSCIF_HSSRR_VAL         (uint16_t)(HSCIF_HSSRR_SRE | HSCIF_HSSRR_SRCYC8)

static uint8_t is_log_enable = SCIF_LOG_STATE_ON;

void wait(uint32_t count)
{
	volatile uint32_t cnt = count;
	do
	{
		;    /* do nothing */
	} while(cnt-- > 0);
}

typedef void (*uart_irq_callback_user_data_t)(void *user_data);

static void uart_rcar_write_8(uint32_t offs, uint8_t value)
{
	sys_write8(value, scif_base + offs);
}

static uint16_t uart_rcar_read_16(uint32_t offs)
{
	return sys_read16(scif_base + offs);
}

static void uart_rcar_write_16(uint32_t offs, uint16_t value)
{
	sys_write16(value, scif_base + offs);
}

static void uart_rcar_set_baudrate(uint32_t port, uint32_t baud_rate)
{
    uint16_t reg_val;
    const uint32_t clock_rate_s0d12 = 66660000u; // S0D12 Clock rate
    const uint32_t clock_rate_sga_syncd4 = 266666666u; // SGASYNCD4 Clock rate

    if (baud_rate >= 3000000) {
        /* 24MHz / (3000000 * 8) = 1 */
        uart_rcar_write_16(DL, HSCIF_DL_DIV1);
        reg_val = uart_rcar_read_16(CKS);
        reg_val &= HSCIF_CKS_SC_CLK_EXT;
        uart_rcar_write_16(CKS, reg_val);
        /* Sampling rate 8  */
        reg_val = uart_rcar_read_16(HSSRR);
        reg_val &= ~(HSCIF_HSSRR_SRE | HSCIF_HSSRR_SRCYC);
        reg_val |= HSCIF_HSSRR_VAL;
        uart_rcar_write_16(HSSRR, reg_val);
        wait(0x2000U);
    } else {
        if (port <= 4) {
            reg_val = ((clock_rate_s0d12 + 16 * baud_rate) / (32 * baud_rate) - 1);
        } else {
	        reg_val = (clock_rate_sga_syncd4 / baud_rate / 8 / 2 - 1);
        }
	    uart_rcar_write_8(SCBRR, reg_val);
    }
}

static void uart_rcar_irq_rx_enable(void)
{
	uint16_t reg_val;

	reg_val = uart_rcar_read_16(SCSCR);
	reg_val |= (SCSCR_RIE);
	uart_rcar_write_16(SCSCR, reg_val);
}

int32_t console_init(uint32_t port) {
    if (port >= (sizeof(serial_channels_arr) / sizeof(serial_channels_arr[0])))
    {
        return -1;
    }

	uint16_t reg_val;

    scif_base = serial_channels_arr[port];
    if (scif_base == (uint32_t)0)
    {
        return -1;
    }

	/* Disable Transmit and Receive */
	reg_val = uart_rcar_read_16(SCSCR);
	reg_val &= ~(SCSCR_TE | SCSCR_RE);
	uart_rcar_write_16(SCSCR, reg_val);

	/* Emptying Transmit and Receive FIFO */
	reg_val = uart_rcar_read_16(SCFCR);
	reg_val |= (SCFCR_TFRST | SCFCR_RFRST);
	uart_rcar_write_16(SCFCR, reg_val);

	/* Resetting Errors Registers */
	reg_val = uart_rcar_read_16(SCFSR);
	reg_val &= ~(SCFSR_ER | SCFSR_DR | SCFSR_BRK | SCFSR_RDF);
	uart_rcar_write_16(SCFSR, reg_val);

	reg_val = uart_rcar_read_16(SCLSR);
	reg_val &= ~(SCLSR_TO | SCLSR_ORER);
	uart_rcar_write_16(SCLSR, reg_val);

    if (UART_BAUDRATE < 3000000) {
        /* Select internal clock */
	    reg_val = uart_rcar_read_16(SCSCR);
	    reg_val &= ~(SCSCR_CKE1 | SCSCR_CKE0);
	    uart_rcar_write_16(SCSCR, reg_val);
    }
    else {
        /* external clock, SC_CLK pin used for output pin */
        uart_rcar_write_16(SCSCR, SCSCR_CKE1);
    }

    /* 8bit data, no-parity, 1 stop, Po/1 */
	reg_val = uart_rcar_read_16(SCSMR);
	reg_val &= SCIF_SCSMR_INIT_DATA;
    uart_rcar_write_16(SCSMR, reg_val);

	/* Set baudrate */
	uart_rcar_set_baudrate(port, UART_BAUDRATE);

	/* reset-off tx-fifo, rx-fifo. */
	reg_val = uart_rcar_read_16(SCFCR);
    reg_val &= ~(SCIF_SCFCR_RESET_FIFO);
	uart_rcar_write_16( SCFCR, reg_val);


    /* 8bit data, no-parity, 1 stop, Po/1 */
    reg_val = uart_rcar_read_16(SCSCR);
	reg_val |= SCIF_SCSCR_INIT_DATA;

	uart_rcar_write_16(SCSCR, reg_val);

	uart_rcar_irq_rx_enable();

	return 0;
}

static void uart_rcar_poll_out(unsigned char out_char)
{
	uint16_t reg_val;

	/* TODO: Add spinlock here */

	/* Wait for empty space in transmit FIFO */
	while (0u == (uart_rcar_read_16(SCFSR) & SCFSR_TDFE)) {
	}

	uart_rcar_write_8(SCFTDR, out_char);

	reg_val = uart_rcar_read_16(SCFSR);
	reg_val &= (~(SCFSR_TDFE | SCFSR_TEND));
	uart_rcar_write_16(SCFSR, reg_val);

	/* TODO: Remove spinlock here */
}

void console_apply_log_state(uint8_t enable) {
	is_log_enable = enable ? SCIF_LOG_STATE_ON : SCIF_LOG_STATE_OFF;
}

void console_putc(char c) {
	/* Runtime API check logs state */
	if (is_log_enable == SCIF_LOG_STATE_ON) {
		uart_rcar_poll_out(c);
	}
}

int console_getc(unsigned char *p_char) {
	uint16_t reg_val;
	uint8_t ret = 0;

	/* Receive FIFO empty */
	if (!((uart_rcar_read_16(SCFSR)) & SCFSR_RDF)) {
		ret = -1;
	}
	else
	{
		*p_char = uart_rcar_read_16(SCFRDR);

		reg_val = uart_rcar_read_16(SCFSR);
		reg_val &= ~SCFSR_RDF;
		uart_rcar_write_16(SCFSR, reg_val);
	}
	return ret;
}
