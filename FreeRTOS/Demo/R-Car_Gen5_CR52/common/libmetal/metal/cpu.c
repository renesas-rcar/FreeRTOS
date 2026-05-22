/*
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
*
*/

#include <FreeRTOS.h>
#include <task.h>

#include "cpu.h"

void metal_cpu_yield(void)
{
    vTaskDelay(pdMS_TO_TICKS(1));
}
