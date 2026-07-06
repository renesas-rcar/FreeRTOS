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
#include "scif_private.h"

static bool portInitialized = false;

static void outbyte(char c);

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

    static int uart_set_pfc(e_serial_devices_t device)
    {
        st_module_config_t uart_module;

        if ((size_t)device >= ARRAY_SIZE(dev_to_module_list)) {
            return -1;
        }

        uart_module.module_id = dev_to_module_list[device];

        if (uart_module.module_id == MODULE_INVALID) {
            return -1;
        }

        uart_module.is_enabled = 1;

        return pfcInitModule(uart_module);
    }

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
    while (length-- > 0) {
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
