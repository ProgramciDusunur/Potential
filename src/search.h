#include "spsa.h"
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

extern TUNE_INT LMR_FULL_DEPTH_MOVES;
extern TUNE_INT LMR_REDUCTION_LIMIT;
extern TUNE_INT lateMovePruningBaseReduction;
extern TUNE_INT NMP_DEPTH;
extern TUNE_INT CONTHIST_MULT;

extern int LMR_TABLE[2][maxPly][maxPly];

int isRepetition(board* position);
uint8_t isMaterialDraw(board *pos);
void initializeLMRTable(void);
int scoreMove(uint16_t move, ThreadData *t, SearchStack *ss, check_info_t *check_info);
void printMove(uint16_t move);
int getLmrReduction(int depth, int moveNumber, bool isQuiet);
uint8_t justPawns(board *pos);
int SEE(board *pos, uint16_t move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
int quiescence(int alpha, int beta, ThreadData *t, my_time* time, SearchStack *ss);
int negamax(int alpha, int beta, int depth, ThreadData *t, my_time* time, SearchStack *ss, bool cutNode);
int searchPosition(int depth, bool benchmark, ThreadData *t, my_time* time);
void init_move_scores(moves *moveList, int *move_scores, uint16_t tt_move, ThreadData *t, SearchStack *ss, check_info_t *check_info);
void pick_next_move(int index, moves *moveList, int *move_scores);
void init_quiescence_scores(moves *moveList, int *move_scores, board* position);

#endif //POTENTIAL_SEARCH_H
