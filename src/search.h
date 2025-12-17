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

extern int LMR_TABLE[2][maxPly][maxPly];
extern int SEE_PIECE_VALUES[];

int isRepetition(board* position);
bool is_material_draw(board *pos);
void initializeLMRTable(void);
int scoreMove(uint16_t move, board* position);
void sort_moves(moves *moveList, uint16_t tt_move, board* position);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(uint16_t move);
int getLmrReduction(int depth, int moveNumber, bool isQuiet);
bool just_pawns(board *pos);
int SEE(board *pos, uint16_t move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
int quiescence(int alpha, int beta, board* position, my_time* time);
void quiescence_sort_moves(moves *moveList, board* position);
int quiescenceScoreMove(uint16_t move, board* position);
int negamax(int alpha, int beta, int depth, board* pos, my_time* time, bool cutNode);
void searchPosition(int depth, board* position, bool benchmark, my_time* time);

#endif //POTENTIAL_SEARCH_H
