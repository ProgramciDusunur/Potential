//
// Created by erena on 31.05.2024.
//
#pragma once

#include "board.h"
#include "math.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "history.h"
#include <values.h>
#include "time.h"
#include "table.h"
#include "move.h"
#include "evaluation.h"
#include "uci.h"


#define maxPly 64

extern int lmr_full_depth_moves;
extern int lmr_reduction_limit;
extern int nullMoveDepth;

extern U64 nodes;
extern U64 variant;

extern int lmrTable[maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];


//void communicate(time* time);

extern int isRepetition(board* position);
extern void initializeLMRTable(void);
extern int scoreMove(int move, board* position);
extern void sort_moves(moves *moveList, int bestMove, board* position);
extern void enable_pv_scoring(moves *moveList, board* position);
extern void printMove(int move);
extern int getLmrReduction(int depth, int moveNumber);
extern void clearCounterMoves(void);
extern int quiescence(int alpha, int beta, board* position, int negamaxScore, time* time);
extern int negamax(int alpha, int beta, int depth, board* position, time* time, bool cutNode);
extern void searchPosition(int depth, board* position, bool benchmark, time* time);



