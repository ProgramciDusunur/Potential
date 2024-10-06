//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_SEARCH_H
#define POTENTIAL_SEARCH_H

#pragma once

#include "structs.h"
#include "time.h"
#include "values.h"
#include "history.h"
#include "math.h"
#include "evaluation.h"
#include "uci.h"

extern int lmr_full_depth_moves;
extern int lmr_reduction_limit;
extern int lateMovePruningBaseReduction;
extern int nullMoveDepth;

extern U64 searchNodes;

extern int lmrTable[maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];


int isRepetition(board* position);
void initializeLMRTable(void);
int scoreMove(int move, board* position, SearchStack *ss);
void sort_moves(moves *moveList, int bestMove, board* position, SearchStack *ss);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(int move);
int getLmrReduction(int depth, int moveNumber);
void clearCounterMoves(void);
int quiescence(int alpha, int beta, board* position, SearchStack *ss, int negamaxScore, time* time, bool improving);
int negamax(int alpha, int beta, int depth, board* position, SearchStack *ss, time* time, bool cutNode);
void searchPosition(int depth, board* position, SearchStack *ss, bool benchmark, time* time);

#endif //POTENTIAL_SEARCH_H
