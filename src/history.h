//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_HISTORY_H
#define POTENTIAL_HISTORY_H


#pragma once



#include "structs.h"
#include "bit_manipulation.h"
#include "move.h"
#include <stdio.h>
#include <stdbool.h>


enum {
    maxHistory = 16000
};

extern int historyMoves[64][64];



int scaledBonus(int score, int bonus);
void updateHistory(int bestMove, int depth);
void clearHistory(void);


#endif //POTENTIAL_HISTORY_H
