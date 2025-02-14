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
    maxQuietHistory = 16384
};

// quietHistory[fromSquare][toSquare]
extern int quietHistory[64][64];
// rootHistory[side to move][fromSquare][toSquare]
extern int rootHistory[2][64][64];



int scaledBonus(int score, int bonus, int gravity);
void updateQuietMoveHistory(int bestMove, int depth, moves *badQuiets);
void updateRootHistory(board *position, int bestMove, int depth, moves *badQuiets);
void clearQuietHistory(void);


#endif //POTENTIAL_HISTORY_H