//
// Created by erena on 31.05.2024.
//
#pragma once

#include "board.h"
#include "math.h"
#include <stdbool.h>


#define maxPly 64

int lmrTable[maxPly][maxPly];

void initializeLMRTable();
int getLmrReduction(int depth, int moveNumber, bool isPv);
//int counterMoves[2][maxPly][maxPly];

