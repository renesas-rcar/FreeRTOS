/*
 *
 * Copyright (c) 2025 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define X5H_VDK             1
#define MDP_AIACC_RFS2      2
#define X5H_IRONHIDE        3
#define X5H_RFS2            4
#define MDP_AIACC_HIL       5
#define MDP_X5H_HIL         6

#ifndef BOARD
#define BOARD               X5H_IRONHIDE
#error "Board is not defined. X5H_IRONHIDE is set as default."
#endif

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
