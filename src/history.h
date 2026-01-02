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
#include <immintrin.h>

enum {
    maxQuietHistory = 16384,
    maxPawnHistory = 16384,
    maxCaptureHistory = 16384
};

// quietHistory[side to move][fromSquare][toSquare][threatSource][threatTarget]
extern int16_t quietHistory[2][64][64][2][2];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
extern int16_t continuationHistory[12][64][12][64];
// continuationCorrectionHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
extern int16_t contCorrhist[12][64][12][64];
// pawnHistory [pawnKey][piece][to]
extern int16_t pawnHistory[2048][12][64];
// captureHistory[piece][toSquare][capturedPiece]
extern int16_t captureHistory[12][64][13];
// kingRookPawn Correction History [side to move][key]
extern int16_t krpCorrhist[2][16384];

extern int CORRHIST_WEIGHT_SCALE;
extern int CORRHIST_GRAIN;
extern int CORRHIST_LIMIT;
extern int CORRHIST_SIZE;
extern int CORRHIST_MAX;
  
extern int16_t PAWN_CORRECTION_HISTORY[2][16384];
extern int16_t MINOR_CORRECTION_HISTORY[2][16384];
extern int16_t MAJOR_CORRECTION_HISTORY[2][16384];
extern int16_t NON_PAWN_CORRECTION_HISTORY[2][2][16384];

int scaledBonus(int score, int bonus, int gravity);
void adjust_single_quiet_hist_entry(board *pos, int side, uint16_t move, int bonus);
void updateQuietMoveHistory(uint16_t bestMove, int side, int depth, moves *badQuiets, board *pos);
void updatePawnHistory(board *pos, uint16_t bestMove, int depth, moves *badQuiets);
void updateSingleCHScore(board *pos, uint16_t move, const int offSet, const int bonus, int quiet_hist_score);
int getAllCHScore(board *pos, uint16_t move, int quiet_hist_score);
void updateAllCH(board *pos, uint16_t move, int bonus, int quiet_hist_score);
int getHistoryBonus(int depth);
void updateContinuationHistory(board *pos, uint16_t bestMove, int depth, moves *badQuiets, int quiet_hist_score);
int getContinuationHistoryScore(board *pos, int offSet, uint16_t move);
void updateCaptureHistory(board *position, uint16_t bestMove, int depth);
void updateCaptureHistoryMalus(board *position, int depth, moves *noisyMoves, uint16_t bestMove);
void update_pawn_correction_hist(board *position, const int depth, const int diff);
void update_minor_correction_hist(board *position, const int depth, const int diff);
void update_major_correction_hist(board *position, const int depth, const int diff);
void update_non_pawn_corrhist(board *position, const int depth, const int diff);
void update_single_cont_corrhist_entry(board *pos, const int pliesBack, const int scaledDiff, const int newWeight);
void update_king_rook_pawn_corrhist(board *position, const int depth, const int diff);
int adjust_eval_with_corrhist(board *pos, int rawEval);
void clear_histories(void);

static inline __attribute__((always_inline)) void quiet_history_aging(void) {
    int16_t *p = (int16_t *)quietHistory;
    const int total_elements = 32768;

    #if defined(__AVX512F__) && defined(__AVX512BW__)
    if (__builtin_cpu_supports("avx512bw")) {
        for (int i = 0; i < total_elements; i += 32) {
            __m512i data = _mm512_loadu_si512((__m512i*)&p[i]);
            data = _mm512_srai_epi16(data, 1);
            _mm512_storeu_si512((__m512i*)&p[i], data);
        }
        return;
    }
    #endif

    #if defined(__AVX2__)
    if (__builtin_cpu_supports("avx2")) {
        for (int i = 0; i < total_elements; i += 16) {
            __m256i data = _mm256_loadu_si256((__m256i*)&p[i]);
            data = _mm256_srai_epi16(data, 1);
            _mm256_storeu_si256((__m256i*)&p[i], data);
        }
        return;
    }
    #endif

    #if defined(__SSE2__)
    if (__builtin_cpu_supports("sse2")) {
        for (int i = 0; i < total_elements; i += 8) {
            __m128i data = _mm_loadu_si128((__m128i*)&p[i]);
            data = _mm_srai_epi16(data, 1);
            _mm_storeu_si128((__m128i*)&p[i], data);
        }
        return;
    }
    #endif

    for (int i = 0; i < total_elements; i++) {
        p[i] >>= 1;
    }
}

#endif //POTENTIAL_HISTORY_H
