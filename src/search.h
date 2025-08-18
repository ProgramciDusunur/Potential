//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_SEARCH_H
#define POTENTIAL_SEARCH_H

#pragma once

#include "structs.h"
#include "timeman.h"
#include "values.h"
#include "history.h"
#include "math.h"
#include "evaluation.h"
#include "uci.h"
#include <stdint.h>
#include "utils.h"

extern int LMR_FULL_DEPTH_MOVES;
extern int LMR_REDUCTION_LIMIT;
extern int lateMovePruningBaseReduction;
extern int NMP_DEPTH;
extern int CORRHIST_WEIGHT_SCALE;
extern int CORRHIST_GRAIN;
extern int CORRHIST_SIZE;
extern int CORRHIST_MAX;

extern U64 searchNodes;

extern int LMR_TABLE[2][maxPly][maxPly];
extern int counterMoves[2][maxPly][maxPly];
extern int PAWN_CORRECTION_HISTORY[2][16384];
extern int MINOR_CORRECTION_HISTORY[2][16384];
extern int NON_PAWN_CORRECTION_HISTORY[2][2][16384];
extern int MAJOR_CORRECTION_HISTORY[2][16384];

int isRepetition(board* position);
uint8_t isMaterialDraw(board *pos);
void initializeLMRTable(void);
int scoreMove(int move, board* position);
void sort_moves(moves *moveList, int tt_move, board* position);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(int move);
int getLmrReduction(int depth, int moveNumber, bool isQuiet);
uint8_t justPawns(board *pos);
void clearCounterMoves(void);
int SEE(board *pos, int move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
void updatePawnCorrectionHistory(board *position, const int depth, const int diff);
void updateMinorCorrectionHistory(board *position, const int depth, const int diff);
void updateMajorCorrectionHistory(board *position, const int depth, const int diff);
int adjustEvalWithCorrectionHistory(board *position, const int rawEval);
void update_non_pawn_corrhist(board *position, const int depth, const int diff);
int quiescence(int alpha, int beta, board* position, my_time* time);
void quiescence_sort_moves(moves *moveList, board* position);
int quiescenceScoreMove(int move, board* position);
int negamax(int alpha, int beta, int depth, board* pos, my_time* time, bool cutNode);
void searchPosition(int depth, board* position, bool benchmark, my_time* time);

#endif //POTENTIAL_SEARCH_H
