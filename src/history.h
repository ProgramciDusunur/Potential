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
    maxCaptureHistory = 16384
};

extern int quietMoveHistory[64][64];
extern int captureMoveHistory[2][64][64];
extern int continuationHistory[12][64][12][64];



int scaledBonus(int score, int bonus, int gravity);
void updateQuietHistory(int bestMove, int depth, moves *badQuiets);
void updateCaptureHistory(board *position, int bestMove, int depth, moves *noisyMoves);
void updateContinuationHistoryMoves(board *position, SearchStack *ss, int bestMove, int depth, moves *quietMoves);
void updateContinuationHistoryEntry(board *position, int offSet, SearchStack *ss, int depth, int move, bool isBestMove);
int getContinuationHistoryScore(SearchStack *ss, int move, int offSet);
void clearQuietHistory(void);
void clearCaptureHistory(void);
void clearContinuationHistory(SearchStack *ss);



#endif //POTENTIAL_HISTORY_H
