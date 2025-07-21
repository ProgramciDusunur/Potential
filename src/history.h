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
    maxPawnHistory = 16384
};

// quietHistory[side to move][fromSquare][toSquare]
extern int quietHistory[2][64][64];
// rootHistory[side to move][fromSquare][toSquare]
extern int rootHistory[2][64][64];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
extern int continuationHistory[12][64][12][64];
// pawnHistory [pawnKey][piece][to]
extern int16_t pawnHistory[2048][12][64];


int scaledBonus(int score, int bonus, int gravity);
void updateQuietMoveHistory(int bestMove, int side, int depth, moves *badQuiets);
void updateRootHistory(board *position, int bestMove, int depth, moves *badQuiets);
void updatePawnHistory(board *pos, int bestMove, int depth, moves *badQuiets);
void updateSingleCHScore(board *pos, int move, const int offSet, const int bonus);
void updateAllCH(board *pos, int move, int bonus);
int getHistoryBonus(int depth);
void updateContinuationHistory(board *pos, int bestMove, int depth, moves *badQuiets);
int getContinuationHistoryScore(board *pos, int offSet, int move);
void clearQuietHistory(void);


#endif //POTENTIAL_HISTORY_H
