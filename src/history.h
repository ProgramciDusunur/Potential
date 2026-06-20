#include "spsa.h"
//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_HISTORY_H
#define POTENTIAL_HISTORY_H


#pragma once


#include "structs.h"
#include "bit_manipulation.h"
#include "move.h"
#include <stdio.h>
#include <stdbool.h>
#include "threads.h"

enum {
    maxQuietHistory = 16384,
    maxPawnHistory = 16384,
    maxCaptureHistory = 16384
};

extern TUNE_INT PAWN_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT PAWN_CORRHIST_GRAIN;
extern TUNE_INT MINOR_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT MINOR_CORRHIST_GRAIN;
extern TUNE_INT MAJOR_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT MAJOR_CORRHIST_GRAIN;
extern TUNE_INT NON_PAWN_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT NON_PAWN_CORRHIST_GRAIN;
extern TUNE_INT KRP_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT KRP_CORRHIST_GRAIN;
extern TUNE_INT CONT_CORRHIST_WEIGHT_SCALE;
extern TUNE_INT CONT_CORRHIST_GRAIN;
extern int CORRHIST_LIMIT;
extern int BASE_CORRHIST_SIZE;
extern int BASE_CONT_CORRHIST_SIZE;
extern int CORRHIST_MAX;
extern int BASE_MATERIAL_CORRHIST_SIZE;
extern TUNE_INT BAD_QUIET_INDEX_SCALE;

int scaledBonus(int score, int bonus, int gravity);
void adjust_single_quiet_hist_entry(ThreadData *t, int side, uint16_t move, int bonus);
void updateQuietMoveHistory(ThreadData *t, uint16_t bestMove, int side, int bonus, int malus, moves *badQuiets);
void updatePawnHistory(ThreadData *t, uint16_t bestMove, int bonus, int malus, moves *badQuiets);
void updateSingleCHScore(ThreadData *t, uint16_t move, const int offSet, const int bonus, int quiet_hist_score, SearchStack *ss);
int getAllCHScore(ThreadData *t, uint16_t move, int quiet_hist_score, SearchStack *ss);
void updateAllCH(ThreadData *t, uint16_t move, int bonus, int quiet_hist_score, SearchStack *ss);
int getHistoryBonus(int depth);
void updateContinuationHistory(ThreadData *t, uint16_t bestMove, int bonus, int malus, moves *badQuiets, int quiet_hist_score, SearchStack *ss);
int getContinuationHistoryScore(ThreadData *t, int offSet, uint16_t move, SearchStack *ss);
void updateCaptureHistory(ThreadData *t, uint16_t bestMove, int bonus);
void updateCaptureHistoryMalus(ThreadData *t, int bonus, moves *noisyMoves, uint16_t bestMove);
void update_pawn_correction_hist(ThreadData *t, const int depth, const int diff);
void update_minor_correction_hist(ThreadData *t, const int depth, const int diff);
void update_major_correction_hist(ThreadData *t, const int depth, const int diff);
void update_non_pawn_corrhist(ThreadData *t, const int depth, const int diff);
void update_continuation_corrhist(ThreadData *t, const int depth, const int diff, SearchStack *ss);
void update_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, const int scaledDiff, const int newWeight, SearchStack *ss);
void update_king_rook_pawn_corrhist(ThreadData *t, const int depth, const int diff);
void update_material_correction_hist(ThreadData *t, const int depth, const int diff);
int adjust_eval_with_corrhist(ThreadData *t, int rawEval, SearchStack *ss);
int get_correction_value(ThreadData *t, SearchStack *ss);
void clear_histories(void);
void quiet_history_aging(void);


#endif //POTENTIAL_HISTORY_H
