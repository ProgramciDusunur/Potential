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
    maxPawnHistory = 16384,
    maxCaptureHistory = 16384
};

// quietHistory[side to move][fromSquare][toSquare][threatSource][threatTarget]
extern int16_t quietHistory[2][64][64][2][2];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
extern int16_t continuationHistory[12][64][12][64];
// continuationCorrectionHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
extern int16_t contCorrhist[12][64][12][64];
// pawnHistory [pawnKey][piece][to][threatSource][threatTarget]
extern int16_t pawnHistory[512][12][64][2][2];
// captureHistory[piece][toSquare][capturedPiece]
extern int16_t captureHistory[12][64][13];
// kingRookPawn Correction History [side to move][key]
extern int16_t krpCorrhist[2][16384];

int scaledBonus(int score, int bonus, int gravity);
void updateQuietMoveHistory(int bestMove, int side, int depth, moves *badQuiets, board *pos);
void updatePawnHistory(board *pos, int bestMove, int depth, moves *badQuiets);
void updateSingleCHScore(board *pos, int move, const int offSet, const int bonus);
void updateAllCH(board *pos, int move, int bonus);
int getHistoryBonus(int depth);
void updateContinuationHistory(board *pos, int bestMove, int depth, moves *badQuiets);
int getContinuationHistoryScore(board *pos, int offSet, int move);
void updateCaptureHistory(board *position, int bestMove, int depth);
void updateCaptureHistoryMalus(board *position, int depth, moves *noisyMoves, int bestMove);
void clearQuietHistory(void);


#endif //POTENTIAL_HISTORY_H
