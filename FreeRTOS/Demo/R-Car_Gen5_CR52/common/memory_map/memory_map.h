/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
 *************************************************************************************************************/

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "board.h"

#if (BOARD == X5H_VDK)
#include "memory_map_x5h_vdk.h"
#elif (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2 || BOARD == MDP_X5H_HIL)
#include "memory_map_x5h_ironhide.h"
#else
#include "memory_map_ai_acc.h"
#endif

#endif //
