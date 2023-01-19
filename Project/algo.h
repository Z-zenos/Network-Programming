#pragma once
#ifndef __ALGO_H__
#define __ALGO_H__

#include "config.h"

int checkVerticalLine     (char [BOARD_S][BOARD_S], char, int, int);
int checkHorizontalLine   (char [BOARD_S][BOARD_S], char, int, int);
int checkDiagonallyLine1  (char [BOARD_S][BOARD_S], char, int, int);
int checkDiagonallyLine2  (char [BOARD_S][BOARD_S], char, int, int);
int checkWinning          (char [BOARD_S][BOARD_S], char, int, int);

#endif