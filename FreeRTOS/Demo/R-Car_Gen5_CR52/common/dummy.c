/*
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>

__attribute__((weak)) uint32_t __tcm_start__ = 0u;
__attribute__((weak)) uint32_t __tcm_end__   = 0u;

__attribute__((weak)) uint32_t __kernel_region_start__ = 0u;
__attribute__((weak)) uint32_t __kernel_region_end__   = 0u;

__attribute__((weak)) uint32_t _TCM_SIZE   = 0u;

//Dummy for these function when using newlib
__attribute__((weak)) int _open(int fd)
{
  return (0);
}

__attribute__((weak)) int _close(int fd)
{
//  (void)fd;
  return (0);
}
__attribute__((weak)) int _write (int fd, char* buf, int nbytes)
{
    return (0);
}
__attribute__((weak)) int _fstat(int fd, void *buf)
{
  (void)fd;

  return (0);
}

__attribute__((weak)) int _isatty(int fd)
{
  (void)fd;
  return (1);
}
__attribute__((weak)) off_t _lseek(int fd, off_t offset, int whence)
{
  (void)fd;
  (void)offset;
  (void)whence;
  //errno = ESPIPE;
  return ((off_t)-1);
}
__attribute__((weak)) int
_read (int fd, char* buf, int nbytes)
{
}

__attribute__((weak))  pid_t _getpid (void)
{
  return  1;

}

__attribute__((weak)) int _kill (pid_t pid, int sig)
{
  return 1;
}

__attribute__((weak)) void vApplicationTickHook( void )
{
    /* Weak function  */
    /* if want to use this function redefine this function */
}

typedef uint8_t e_log_level_t;
__attribute__((weak)) void LDR_FRTOS_BSP_LOG_HANDLER(e_log_level_t log_level,
                                  const char* module_name, const char* fmt, ...)
{
    const char *log_str[] = {"INFO", "DEBUG", "WARN", "ERROR", "FATAL"};
    char buf[512];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    fprintf(stdout, "[%s] [%s]: %s", log_str[log_level], module_name, buf);
}

__attribute__((weak)) void vApplicationMallocFailedHook( void )
{
    /* Weak function  */
    /* if want to use this function redefine this function */
    printf("%s\n", __func__);
    for(;;) {}
}

typedef void* TaskHandle_t;
__attribute__((weak)) void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                                            char *pcTaskName)
{
    /* Weak function  */
    /* if want to use this function redefine this function */
    ( void ) xTask;
    printf("%s\n", __func__);
    printf("Task name: %s\n", pcTaskName);
}

__attribute__((weak)) void vApplicationAssertHook(void *arg)
{
    /* Weak function  */
    /* if want to use this function redefine this function */
}
