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
    maxQuietHistory = 16384,
    maxCaptureHistory = 32768
};

extern int quietMoveHistory[64][64];
extern int captureMoveHistory[2][64][64];



int scaledBonus(int score, int bonus, int gravity);
void updateQuietHistory(int bestMove, int depth, moves *badQuiets);
void updateCaptureHistory(board *position, int bestMove, int depth, moves *noisyMoves);
void clearHistory(void);


#endif //POTENTIAL_HISTORY_H
