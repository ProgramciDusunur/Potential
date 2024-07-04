//
// Created by erena on 29.06.2024.
//

#pragma once

#include "board.h"
#include "bit_manipulation.h"
#include "move.h"
#include <stdio.h>


enum {
    maxHistory = 16000
};

int historyMoves[64][64];



void updateHistory(int bestMove, int depth, moves* badQuiets);
int scaledBonus(int score, int bonus);
void clearHistory();

