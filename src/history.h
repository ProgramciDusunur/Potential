//
// Created by erena on 29.06.2024.
//

#pragma once

#include "board.h"
#include "bit_manipulation.h"


enum {
    maxHistory = 16000
};

int historyMoves[64][64];



void updateHistory(int moveFrom, int moveTo, int depth);
int historyBonus(int score, int bonus);

