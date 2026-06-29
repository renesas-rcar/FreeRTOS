/*************************************************************************************************************
* Copyright (c) 2025 Renesas Electronics Corporation
*
* SPDX-License-Identifier: MIT
 *************************************************************************************************************/

#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include "board.h"
#ifndef BOARD_H
    #error "board.h must be included before using statement with macro BOARD"
#elif (BOARD == X5H_VDK)
#include "memory_map_x5h_vdk.h"
#elif (BOARD == X5H_IRONHIDE || BOARD == X5H_RFS2)
#include "memory_map_x5h_ironhide.h"
#else
#include "memory_map_ai_acc.h"
#endif

#endif // 
