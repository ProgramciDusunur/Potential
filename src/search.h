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
int scoreMove(uint16_t move, ThreadData *t, SearchStack *ss);
void enable_pv_scoring(moves *moveList, board* position);
void printMove(uint16_t move);
int getLmrReduction(int depth, int moveNumber, bool isQuiet);
uint8_t justPawns(board *pos);
int SEE(board *pos, uint16_t move, int threshold);
uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq);
int quiescence(int alpha, int beta, ThreadData *t, my_time* time, SearchStack *ss);
int negamax(int alpha, int beta, int depth, ThreadData *t, my_time* time, SearchStack *ss, bool cutNode);
void searchPosition(int depth, bool benchmark, ThreadData *t, my_time* time);

#endif //POTENTIAL_SEARCH_H
