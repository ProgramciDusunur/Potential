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

extern int lmrTable[maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];

extern int lmr_full_depth_moves;
extern int lmr_reduction_limit;
extern int lateMovePruningBaseReduction;
extern int nullMoveDepth;

// performance test node count, variant count
extern U64 nodes, variant;

extern void initializeLMRTable();
extern int getLmrReduction(int depth, int moveNumber, bool isPv, bool improving);
extern void clearCounterMoves();
extern int scoreMove(int move, board* position);
extern int sort_moves(moves *moveList, int bestMove, board* position);
extern void enable_pv_scoring(moves *moveList, board* position);
extern int isRepetition(board* position);
extern int quiescence(int alpha, int beta, board* position, int negamaxScore);
extern int negamax(int alpha, int beta, int depth, board* position);
extern void searchPosition(int depth, board* position, bool benchmark);