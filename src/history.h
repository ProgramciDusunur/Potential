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
#include "utils.h"


enum {
    maxQuietHistory = 16384,
    QUIET_HISTORY_BONUS_MIN = 1200,
    QUIET_HISTORY_BONUS_MAX = 1300,
    QUIET_HISTORY_MALUS_MIN = 1300,
    QUIET_HISTORY_MALUS_MAX = 1200


};

extern int historyMoves[64][64];



int scaledBonus(int score, int bonus, int gravity);
void updateQuietMoveHistory(int bestMove, int depth, moves *badQuiets);
void clearHistory(void);


#endif //POTENTIAL_HISTORY_H
