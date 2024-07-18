//
// Created by erena on 29.06.2024.
//

#pragma once

#include "board.h"
#include "bit_manipulation.h"
#include "move.h"
#include <stdio.h>
#include <stdbool.h>


enum {
    maxHistory = 16000
};

extern int historyMoves[64][64];



extern void updateHistory(int bestMove, int depth, moves* badQuiets);
extern int scaledBonus(int score, int bonus);
extern void clearHistory();

