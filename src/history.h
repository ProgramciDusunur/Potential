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


int getSingleContinuationHistoryScore(const SearchStack *ss, int move, int offSet);
int getContinuationHistoryScore(const SearchStack *ss, int move);
void updateSingleContinuationHistoryScore(const board *position, SearchStack *ss, int move, int bonus, int offSet);
void updateContinuationHistoryScore(board *position, SearchStack *ss, int move, int bonus);
int scaledBonus(int score, int bonus);
void updateHistory(board *position, SearchStack *ss, int bestMove, int depth, moves *badQuiets);
void clearHistory(void);
void clearContinuationHistory(SearchStack *ss);


#endif //POTENTIAL_HISTORY_H
