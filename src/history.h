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
    maxHistory = 16384
};

extern int historyMoves[64][64];


void updateContinuationHistoryScore(SearchStack *ss, const int move, const int bonus);
void updateSingleContinuationHistoryScore(SearchStack *ss, const int move, const int bonus, const int offSet);
int getContinuationHistoryScore(const SearchStack *ss, const int move);
int getSingleContinuationHistoryScore(const SearchStack *ss, const int move, const int offSet);
int scaledBonus(int score, int bonus);
void updateHistory(SearchStack *ss, int bestMove, int depth, moves *badQuiets);
void clearHistory(void);
void clearContinuationHistory(SearchData *sd);


#endif //POTENTIAL_HISTORY_H
