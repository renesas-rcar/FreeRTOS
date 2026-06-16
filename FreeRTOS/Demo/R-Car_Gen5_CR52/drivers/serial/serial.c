/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
 
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#include "CMSIS_5/cmsis_rcar_gen5.h"
#include "scif.h"
#include "serial/r_serial.h"
#include "pfc/r_pfc_api.h"

#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
/* PFC (Pin Function Controller) */
#define RCAR_PFC_GPSR1  0xC0800840u       /* R/W 32 GPIO/Peripheral_Function Select register PortGroup 1 */
#define RCAR_PFC_GPSR1_SCIF_ENABLE  0x0001F000u  /* bit16:HRX0, bit15:HSCK0, bit14:HRTS0#, bit13:HCTS0#, bit12:HTX0 */
#define RCAR_PFC_IP1SR1 0xC0800864u       /* R/W 32 Peripheral Function Select register 1 PortGroup 1 */
#define RCAR_PFC_IP1SR1_SCIF_ENABLE  0x11110000u /* [31:16]: 0x1111 (select SCK0, RTS0#, CTS0#, TX0) */
#define RCAR_PFC_IP1SR1_SCIF_CLEAR_MASK  0x0000FFFFu /* [31:16]:clear */
#define RCAR_PFC_IP2SR1 0xC0800868u       /* R/W 32 Peripheral Function Select register 2 PortGroup 1 */
#define RCAR_PFC_IP2SR1_SCIF_ENABLE  0x00000001u /* [3:0]: 0x1 (select RX0) */
#define RCAR_PFC_IP2SR1_SCIF_CLEAR_MASK  0xFFFFFFF0u /* [3:0]: clear */
#define RCAR_PFC_PMMR(addr)  ((addr) & 0xFFFFF800u) /* R/W 32 LSI Multiplexed Pin Setting Mask Register */
#else
/* Offset of RW, SET, CLEAR registers */
#define PFC_RW_OFFSET   (0x0000U)
#define PFC_SET_OFFSET  (0x0200U)
#define PFC_CLR_OFFSET  (0x0400U)

/* Read/Write registers */
#define PFC_PORT_GRP_MASK   (0xFFFFF800U)

/* Port Group0 */
#define PFC_PORT_GRP0   (0x38080000U + 0x0000U)    /* Port Group0 */

#define PFC_GP0_GPSR_RW     (PFC_PORT_GRP0 + PFC_RW_OFFSET + 0x0040U)
#define PFC_GP0_ALTSEL0_RW  (PFC_PORT_GRP0 + PFC_RW_OFFSET + 0x0060U)
#define PFC_GP0_ALTSEL1_RW  (PFC_PORT_GRP0 + PFC_RW_OFFSET + 0x0064U)
#define PFC_GP0_ALTSEL2_RW  (PFC_PORT_GRP0 + PFC_RW_OFFSET + 0x0068U)
#define PFC_GP0_ALTSEL3_RW  (PFC_PORT_GRP0 + PFC_RW_OFFSET + 0x006CU)

#define	PFC_PMMR(addr)			((addr) & (uintptr_t)0xFFFFF800U)	// R/W	32	LSI Multiplexed Pin Setting Mask Register


#define PFC_TX              (0x00000001U)                   /* HTX0 / TX0 */
#define PFC_RX              (0x00000002U)                   /* HRX0 / RX0 */
#define PFC_SCIF_EXTCLK     (0x00000020U)             /* Mask value of IPSR (External Clock) */
#define PFC_SCIF_MASK       (PFC_TX | PFC_RX | PFC_SCIF_EXTCLK)                   /* SCIF0/HSCIF0 RX/TX */
static inline void pfc_reg_write(uint32_t addr, uint32_t data)
{
    sys_write32(~data, (addr & PFC_PORT_GRP_MASK));
    sys_write32(data, addr);

}
#endif

static bool portInitialized = false;

static void outbyte(char c);

static void uart_rcar_pfc_init(void);

static int uart_set_pfc(e_serial_devices_t device);

static bool log_sync = false;
static e_mfis_lock_id_t mfis_lock_id = MFIS_LOCK_ID_MAX_NUM;

#define UART_TIMEOUT 100

int32_t R_SERIAL_PortInit(e_serial_devices_t device)
{
	int ret = 0;

    if (log_sync)
    {
        if (R_MFIS_LockAcquire(mfis_lock_id, UART_TIMEOUT) != MFIS_LOCK_SUCCESS) {
            return -1;
        }
    }

    if (!portInitialized)
    {
        if(console_init(device) == 0)
        {
            portInitialized = true;
        }
        else
        {
            ret = -1;
        }
    }

    ret = uart_set_pfc(device);

    if (log_sync)
    {
        R_MFIS_LockRelease(mfis_lock_id);
    }

	return ret;
}

int32_t R_SERIAL_ReConfigure(e_serial_devices_t device)
{
	int ret = 0;

	ret = console_init(device);
    if (ret != 0) {
        return ret;
    }
	
	ret = uart_set_pfc(device);
	return ret;
}

#if (BOARD == X5H_IRONHIDE)
static int uart_set_pfc(e_serial_devices_t device)
{
    int ret = 0;
    st_module_config_t uart_module;

    uart_module.is_enabled = 1;

    switch (device) {
        case SCIF0:
            uart_module.module_id = MODULE_SCIF0;
            break;
        case SCIF1:
            uart_module.module_id = MODULE_SCIF1;
            break;
        case SCIF3:
            uart_module.module_id = MODULE_SCIF3;
            break;
        case SCIF4:
            uart_module.module_id = MODULE_SCIF4;
            break;
        case HSCIF0:
            uart_module.module_id = MODULE_HSCIF0;
            break;
        case HSCIF1:
            uart_module.module_id = MODULE_HSCIF1;
            break;
        case HSCIF2:
            uart_module.module_id = MODULE_HSCIF2;
            break;
        case HSCIF3:
            uart_module.module_id = MODULE_HSCIF3;
            break;
        case SCIF2_UNSUPPORTED:
        default:
            ret = -1;
            goto end_set_pfc;
    }

    ret = pfcInitModule(uart_module);

end_set_pfc:
    return ret;
}
#else
static int uart_set_pfc(e_serial_devices_t device)
{
    return 0;
}
#endif //#if (BOARD == X5H_IRONHIDE)

int32_t R_SERIAL_PutString(const unsigned char *buffer, unsigned short length)
{
    if (!portInitialized) {
        return -1;
    }

    if (log_sync)
    {
        if (R_MFIS_LockAcquire(mfis_lock_id, UART_TIMEOUT) != MFIS_LOCK_SUCCESS)
        {
            return -1;
        }
    }

    /* Send each character in the string, one at a time. */
    while (length--) {
        if (*buffer == '\n') {
            console_putc('\r');
        }
        console_putc(*buffer);
        buffer++;
    }

    if (log_sync)
    {
        R_MFIS_LockRelease(mfis_lock_id);
    }

	return 0;
}

int32_t R_SERIAL_GetChar(unsigned char *recv_char)
{
    if (recv_char != NULL) {
        return console_getc(recv_char);
    }
	return -1;
}

int32_t R_SERIAL_PutChar(unsigned char send_char)
{
    if (log_sync)
    {
        if (R_MFIS_LockAcquire(mfis_lock_id, UART_TIMEOUT) != MFIS_LOCK_SUCCESS)
        {
            return -1;
        }
    }

    console_putc(send_char);

    if (log_sync)
    {
        R_MFIS_LockRelease(mfis_lock_id);
    }

    return 0;
}

int32_t R_SERIAL_Close(void)
{
	/* Not supported */
	return 0;
}

int32_t R_SERIAL_SetLogState(e_log_state_t state)
{
	console_apply_log_state(state);
	return 0;
}

void R_SERIAL_AMP_LogSync(e_mfis_lock_id_t lock_id, bool sync)
{
    mfis_lock_id = lock_id;
    log_sync = sync;
}

/* Override std C lib output for printf, fprintf */
int _write(int file, char *ptr, int len)
{
	int i;
    (void) file;

    if (log_sync)
    {
        if (R_MFIS_LockAcquire(mfis_lock_id, UART_TIMEOUT) != MFIS_LOCK_SUCCESS) {
            return -1;
        }
    }

    for (i = 0; i < len; i++) {
        outbyte(*ptr++);
    }

    if (log_sync)
    {
        R_MFIS_LockRelease(mfis_lock_id);
    }

    return len;
}

int printf_raw(const char *format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = vfprintf(stderr, format, args);
	va_end(args);

	return ret;
}

/* TO DO: Remove when done fix HSCIF issue. */
int printf_delay(const char *format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vfprintf(stderr, format, args);
    va_end(args);

    vTaskDelay(1);

    return ret;
}

static void outbyte(char c)
{
	/* Standard practice to convert \n to \r\n */
    if (c == '\n') {
		console_putc('\r');
    }

	console_putc(c);
}

#if (BOARD == X5H_VDK || BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
static void uart_rcar_pfc_init(void)
{
	uint32_t drv_data;

	/* GPSR1:Set 0xf to [16:12] */
	drv_data = sys_read32(RCAR_PFC_GPSR1);
	drv_data = drv_data | RCAR_PFC_GPSR1_SCIF_ENABLE;
	sys_write32(~drv_data, RCAR_PFC_PMMR(RCAR_PFC_GPSR1));
	sys_write32(drv_data, RCAR_PFC_GPSR1);

	/* IP1SR1:Set 0x1111 to [31:16] */
	drv_data = sys_read32(RCAR_PFC_IP1SR1);
	drv_data = (drv_data & RCAR_PFC_IP1SR1_SCIF_CLEAR_MASK) | RCAR_PFC_IP1SR1_SCIF_ENABLE;
	sys_write32(~drv_data, RCAR_PFC_PMMR(RCAR_PFC_IP1SR1));
	sys_write32(drv_data, RCAR_PFC_IP1SR1);

	/* IP2SR1:Set 0x1 to [3:0] */
	drv_data = sys_read32(RCAR_PFC_IP2SR1);
	drv_data = (drv_data & RCAR_PFC_IP2SR1_SCIF_CLEAR_MASK) | RCAR_PFC_IP2SR1_SCIF_ENABLE;
	sys_write32(~drv_data, RCAR_PFC_PMMR(RCAR_PFC_IP2SR1));
	sys_write32(drv_data, RCAR_PFC_IP2SR1);
}
#else
static void uart_rcar_pfc_init(void)
{
	uint32_t reg;
	uint32_t gpsr_scif_val = 0U;
	uint32_t altsel_scif_val[4] = {0U};

	/* This setting value set to GPSR, the GP05_00/GP00_00 and GP05_01/GP00_01 switch to peripheral function. */
	gpsr_scif_val = (PFC_TX | PFC_RX);

	/* When Mode pin is HSCIF 3Mbps.
	* This setting value set to GPSR, it's pin function of the external clock switch to peripheral function. */
	gpsr_scif_val |= PFC_SCIF_EXTCLK;

	/* Setting value set to ALTSELn. */
	reg = sys_read32(PFC_GP0_ALTSEL0_RW);
	reg &= (~(PFC_SCIF_MASK));
	reg |= altsel_scif_val[0U];
	pfc_reg_write(PFC_GP0_ALTSEL0_RW, reg);

	reg = sys_read32(PFC_GP0_ALTSEL1_RW);
	reg &= (~(PFC_SCIF_MASK));
	reg |= altsel_scif_val[1U];
	pfc_reg_write(PFC_GP0_ALTSEL1_RW, reg);

	reg = sys_read32(PFC_GP0_ALTSEL2_RW);
	reg &= (~(PFC_SCIF_MASK));
	reg |= altsel_scif_val[2U];
	pfc_reg_write(PFC_GP0_ALTSEL2_RW, reg);

	reg = sys_read32(PFC_GP0_ALTSEL3_RW);
	reg &= (~(PFC_SCIF_MASK));
	reg |= altsel_scif_val[3U];
	pfc_reg_write(PFC_GP0_ALTSEL3_RW, reg);

	/* Setting value set to GPSR. */
	reg = sys_read32(PFC_GP0_GPSR_RW);
	reg &= (~(PFC_SCIF_MASK));
	reg |= gpsr_scif_val;
	pfc_reg_write(PFC_GP0_GPSR_RW, reg);
}
#endif
