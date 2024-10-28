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
    maxHistory = 16384,
    captureHistMax = 32768
};

extern int historyMoves[64][64];


int getCaptureHistoryScore(const SearchData *sd, const int move);
void updateCaptureHistoryScore(SearchData *sd, const int move, int bonus);
int scaledBonus(int score, int bonus);
void updateHistory(SearchData *sd, int bestMove, int depth, moves *badQuiets, moves *badCaptures);
void clearHistory(void);
void clearCaptureHistory(SearchData *sd);


#endif //POTENTIAL_HISTORY_H
