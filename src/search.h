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
#include "see.h"

extern int LMR_FULL_DEPTH_MOVES;
extern int LMR_REDUCTION_LIMIT;
extern int lateMovePruningBaseReduction;
extern int NMP_DEPTH;

extern int LMR_TABLE[2][maxPly][maxPly];

int isRepetition(board* position);
uint8_t isMaterialDraw(board *pos);
void initializeLMRTable(void);
int scoreMove(uint16_t move, board* position);
void sort_moves(moves *moveList, uint16_t tt_move, board* position);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(uint16_t move);
int getLmrReduction(int depth, int moveNumber, bool isQuiet);
uint8_t justPawns(board *pos);
int SEE(board *pos, uint16_t move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
void updatePawnCorrectionHistory(board *position, const int depth, const int diff);
void updateMinorCorrectionHistory(board *position, const int depth, const int diff);
void updateMajorCorrectionHistory(board *position, const int depth, const int diff);
void update_continuation_corrhist(board *pos, const int depth, const int diff);
int adjust_single_cont_corrhist_entry(board *pos, const int pliesBack);
int adjustEvalWithCorrectionHistory(board *position, int rawEval);
void update_non_pawn_corrhist(board *position, const int depth, const int diff);
int quiescence(int alpha, int beta, board* position, my_time* time);
void quiescence_sort_moves(moves *moveList, board* position);
int quiescenceScoreMove(uint16_t move, board* position);
int negamax(int alpha, int beta, int depth, board* pos, my_time* time, bool cutNode);
void searchPosition(int depth, board* position, bool benchmark, my_time* time);

#endif //POTENTIAL_SEARCH_H
