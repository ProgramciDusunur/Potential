//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_SEARCH_H
#define POTENTIAL_SEARCH_H

#pragma once


#include <stdint.h>
#include "structs.h"
#include "time.h"
#include "values.h"
#include "history.h"
#include "math.h"
#include "evaluation.h"
#include "uci.h"
#include "utils.h"
#include "move.h"

#define maxPly 64

extern U64 searchNodes;

extern int lmrTable[maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];


int isRepetition(board* position);
int SEE(board *pos, int move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
void initializeLMRTable(void);
int scoreMove(int move, board* position);
int quiescenceScoreMove(int move, board* position);
void sort_moves(moves *moveList, int bestMove, board* position);
void quiescence_sort_moves(moves *moveList, board* position);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(int move);
int getLmrReduction(int depth, int moveNumber);
uint8_t justPawns(board *pos);
void clearCounterMoves(void);
int quiescence(int alpha, int beta, board* position, time* time);
int negamax(int alpha, int beta, int depth, board* position, time* time, bool cutNode);
void searchPosition(int depth, board* position, bool benchmark, time* time);

#endif //POTENTIAL_SEARCH_H
