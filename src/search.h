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

#define maxPly 64

extern int lmr_full_depth_moves;
extern int lmr_reduction_limit;
extern int lateMovePruningBaseReduction;
extern int nullMoveDepth;

extern U64 searchNodes;

extern int lmrTable[maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];


int isRepetition(board* position);
void initializeLMRTable();
int scoreMove(int move, board* position);
void sort_moves(moves *moveList, int bestMove, board* position);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(int move);
int getLmrReduction(int depth, int moveNumber);
void clearCounterMoves();
int quiescence(int alpha, int beta, board* position, int negamaxScore, time* time);
int negamax(int alpha, int beta, int depth, board* position, time* time, bool cutNode);
void searchPosition(int depth, board* position, bool benchmark, time* time);

#endif //POTENTIAL_SEARCH_H
