//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"
#include "threads.h"
#include "search.h"


/*╔════════════════════╗
  ║ Correction History ║
  ╚════════════════════╝*/

TUNE_INT PAWN_CORRHIST_WEIGHT_SCALE = 250;
TUNE_INT PAWN_CORRHIST_GRAIN = 236;
TUNE_INT MINOR_CORRHIST_WEIGHT_SCALE = 266;
TUNE_INT MINOR_CORRHIST_GRAIN = 256;
TUNE_INT MAJOR_CORRHIST_WEIGHT_SCALE = 241;
TUNE_INT MAJOR_CORRHIST_GRAIN = 292;
TUNE_INT NON_PAWN_CORRHIST_WEIGHT_SCALE = 246;
TUNE_INT NON_PAWN_CORRHIST_GRAIN = 296;
TUNE_INT KRP_CORRHIST_WEIGHT_SCALE = 255;
TUNE_INT KRP_CORRHIST_GRAIN = 276;
TUNE_INT CONT_CORRHIST_WEIGHT_SCALE = 259;
TUNE_INT CONT_CORRHIST_GRAIN = 223;
int CORRHIST_LIMIT = 1024;
int BASE_CORRHIST_SIZE = 16384;
int CORRHIST_MAX = 16384;

/* Update History */

int getHistoryBonus(int depth) {
    return myMIN(10 + 200 * depth, 4096);
}

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void adjust_single_quiet_hist_entry(ThreadData *t, int side, uint16_t move, int bonus) {
    int from = getMoveSource(move);
    int to = getMoveTarget(move);
    
    bool threatSource = is_square_threatened(&t->pos, from);
    bool threatTarget = is_square_threatened(&t->pos, to);
    
    t->search_d.quietHistory[side][from][to][threatSource][threatTarget] += bonus;
}

void updateQuietMoveHistory(ThreadData *t, uint16_t bestMove, int side, int bonus, int malus, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int score = t->search_d.quietHistory[side][from][to][is_square_threatened(&t->pos, from)][is_square_threatened(&t->pos, to)];

    t->search_d.quietHistory[side][from][to][is_square_threatened(&t->pos, from)][is_square_threatened(&t->pos, to)] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = t->search_d.quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(&t->pos, badQuietFrom)][is_square_threatened(&t->pos, badQuietTo)];
        int scaled_malus = malus + index * BAD_QUIET_INDEX_SCALE;

        t->search_d.quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(&t->pos, badQuietFrom)][is_square_threatened(&t->pos, badQuietTo)] +=
        scaledBonus(badQuietScore, -scaled_malus, maxQuietHistory);
    }
}

void updatePawnHistory(ThreadData *t, uint16_t bestMove, int bonus, int malus, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int score = t->shared_history->pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to];

    t->shared_history->pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to] += scaledBonus(score, bonus, maxPawnHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;

        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        t->shared_history->pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[badQuietFrom]][badQuietTo] += scaledBonus(score, -malus, maxPawnHistory);
    }
}

void updateCaptureHistory(ThreadData *t, uint16_t bestMove, int bonus) {
    int piece = t->pos.mailbox[getMoveSource(bestMove)];
    int to = getMoveTarget(bestMove);
    int capturedPiece = t->pos.mailbox[getMoveTarget(bestMove)];

    int score = t->search_d.captureHistory[piece][to][capturedPiece];

    t->search_d.captureHistory[piece][to][capturedPiece] += scaledBonus(score, bonus, maxCaptureHistory);
}

void updateCaptureHistoryMalus(ThreadData *t, int bonus, moves *noisyMoves, uint16_t bestMove) {
    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyPiece = t->pos.mailbox[getMoveSource(noisyMoves->moves[index])];
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);
        int noisyCapturedPiece = t->pos.mailbox[getMoveTarget(noisyMoves->moves[index])];

        if (noisyMoves->moves[index] == bestMove) continue;

        int noisyMoveScore = t->search_d.captureHistory[noisyPiece][noisyTo][noisyCapturedPiece];        

        t->search_d.captureHistory[noisyPiece][noisyTo][noisyCapturedPiece] += scaledBonus(noisyMoveScore, -bonus, maxCaptureHistory);
    }
}

int getAllCHScore(ThreadData *t, uint16_t move, int quiet_hist_score, SearchStack *ss) {
    return (getContinuationHistoryScore(t, 1, move, ss) + quiet_hist_score) / 2 +
           getContinuationHistoryScore(t, 2, move, ss) +
           getContinuationHistoryScore(t, 4, move, ss);
}

int getContinuationHistoryScore(ThreadData *t, int offSet, uint16_t move, SearchStack *ss) {
    if (t->pos.ply < offSet) return 0;
    SearchStack *prev = ss - offSet;
    return t->shared_history->continuationHistory[prev->piece][getMoveTarget(prev->move)]
                              [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)];
}

void updateSingleCHScore(ThreadData *t, uint16_t move, const int offSet, const int bonus, int quiet_hist_score, SearchStack *ss) {
    if (t->pos.ply < offSet) return;
    int base_conthist_score = getAllCHScore(t, move, quiet_hist_score, ss);
    SearchStack *prev = ss - offSet;
    const int scaledBonus = bonus - (int)(((int64_t)base_conthist_score * abs(bonus) * CONTHIST_MULT) / 16777216);
    t->shared_history->continuationHistory[prev->piece][getMoveTarget(prev->move)]
                          [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)] += scaledBonus;
}

void updateAllCH(ThreadData *t, uint16_t move, int bonus, int quiet_hist_score, SearchStack *ss) {
    updateSingleCHScore(t, move, 1, bonus, quiet_hist_score, ss);
    updateSingleCHScore(t, move, 2, bonus, quiet_hist_score, ss);
    updateSingleCHScore(t, move, 4, bonus, quiet_hist_score, ss);
}

void updateContinuationHistory(ThreadData *t, uint16_t bestMove, int bonus, int malus, moves *badQuiets, int quiet_hist_score, SearchStack *ss) {
    updateAllCH(t, bestMove, bonus, quiet_hist_score, ss);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        updateAllCH(t, badQuiets->moves[index], -malus, quiet_hist_score, ss);
    }
}

/* Update Correction History */

static inline void apply_corrhist_update(int16_t *entry, const int scaledDiff, const int newWeight, const int weightScale) {    
    int val = *entry;
    val = (val * (weightScale - newWeight) + scaledDiff * newWeight) / weightScale;
        
    if (val > CORRHIST_MAX) val = CORRHIST_MAX;
    else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

    *entry = val;
}

void update_pawn_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * PAWN_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    // Masking for faster indexing (assuming SIZE is power of 2)
    int16_t *entry = &t->shared_history->pawn_corrhist[t->pos.side][t->pos.pawnKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight, PAWN_CORRHIST_WEIGHT_SCALE);
}

void update_minor_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * MINOR_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->minor_corrhist[t->pos.side][t->pos.minorKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight, MINOR_CORRHIST_WEIGHT_SCALE);
}

void update_major_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * MAJOR_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->major_corrhist[t->pos.side][t->pos.majorKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight, MAJOR_CORRHIST_WEIGHT_SCALE);
}

void update_non_pawn_corrhist(ThreadData *t, const int depth, const int diff) {    
    const int side = t->pos.side;
    const int scaledDiff = diff * NON_PAWN_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    const int mask = t->shared_history->corrhist_mask;
    
    int16_t *white_ptr = &t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    apply_corrhist_update(white_ptr, scaledDiff, newWeight, NON_PAWN_CORRHIST_WEIGHT_SCALE);
    
    int16_t *black_ptr = &t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    apply_corrhist_update(black_ptr, scaledDiff, newWeight, NON_PAWN_CORRHIST_WEIGHT_SCALE);
}

void update_king_rook_pawn_corrhist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * KRP_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->krp_corrhist[t->pos.side][t->pos.krpKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight, KRP_CORRHIST_WEIGHT_SCALE);
}

void update_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, const int scaledDiff, const int newWeight, SearchStack *ss) {
    if (t->pos.ply < pliesBack) return;
    SearchStack *prev = ss - pliesBack;

    const int m1 = prev->move;
    const int m2 = ss->move;
    
    if (m1 && m2) {
        int16_t *entry_ptr = &t->shared_history->contCorrhist[prev->piece][getMoveTarget(m1)]
                                         [ss->piece][getMoveTarget(m2)];
                
        int val = *entry_ptr;
        val = (val * (CONT_CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CONT_CORRHIST_WEIGHT_SCALE;
                
        if (val > CORRHIST_MAX) val = CORRHIST_MAX;
        else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

        *entry_ptr = (int16_t)val;
    }
}

static inline int adjust_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, SearchStack *ss) {
    if (t->pos.ply < pliesBack) return 0;
    SearchStack *prev = ss - pliesBack;

    const int m1 = prev->move;
    const int m2 = ss->move;

    if (m1 && m2) {
        return t->shared_history->contCorrhist[prev->piece][getMoveTarget(m1)]
                               [ss->piece][getMoveTarget(m2)];
    }
    return 0;
}

void update_continuation_corrhist(ThreadData *t, const int depth, const int diff, SearchStack *ss) {
    const int scaledDiff = diff * CONT_CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    update_single_cont_corrhist_entry(t, 1, scaledDiff, newWeight, ss);
    update_single_cont_corrhist_entry(t, 2, scaledDiff, newWeight, ss);
    update_single_cont_corrhist_entry(t, 3, scaledDiff, newWeight, ss);
    update_single_cont_corrhist_entry(t, 4, scaledDiff, newWeight, ss);
    update_single_cont_corrhist_entry(t, 5, scaledDiff, newWeight, ss);
}

int adjust_eval_with_corrhist(ThreadData *t, int rawEval, SearchStack *ss) {       
    rawEval = (rawEval * (300 - t->pos.fifty)) / 300;
    
    const int side = t->pos.side;
    const int mask = t->shared_history->corrhist_mask;

    // Batch memory access    
    int adjust = t->shared_history->pawn_corrhist[side][t->pos.pawnKey & mask] * 256 / PAWN_CORRHIST_GRAIN
               + t->shared_history->minor_corrhist[side][t->pos.minorKey & mask] * 256 / MINOR_CORRHIST_GRAIN
               + t->shared_history->major_corrhist[side][t->pos.majorKey & mask] * 256 / MAJOR_CORRHIST_GRAIN
               + t->shared_history->krp_corrhist[side][t->pos.krpKey & mask] * 256 / KRP_CORRHIST_GRAIN
               + (t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask]
               + t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask]) * 256 / NON_PAWN_CORRHIST_GRAIN
               + (adjust_single_cont_corrhist_entry(t, 1, ss)
               + adjust_single_cont_corrhist_entry(t, 2, ss)
               + adjust_single_cont_corrhist_entry(t, 3, ss)               
               + adjust_single_cont_corrhist_entry(t, 4, ss)
               + adjust_single_cont_corrhist_entry(t, 5, ss)) * 256 / CONT_CORRHIST_GRAIN;

    const int mateFound = mateValue - maxPly;
    
    rawEval += adjust / 256;
    
    if (rawEval >= mateFound) return mateFound - 1;
    if (rawEval <= -mateFound) return -mateFound + 1;
    
    return rawEval;
}

int get_correction_value(ThreadData *t, SearchStack *ss) {
    const int side = t->pos.side;
    const int mask = t->shared_history->corrhist_mask;

    const int pawn_correction = t->shared_history->pawn_corrhist[side][t->pos.pawnKey & mask] * 256 / PAWN_CORRHIST_GRAIN;
    const int minor_correction = t->shared_history->minor_corrhist[side][t->pos.minorKey & mask] * 256 / MINOR_CORRHIST_GRAIN;
    const int major_correction = t->shared_history->major_corrhist[side][t->pos.majorKey & mask] * 256 / MAJOR_CORRHIST_GRAIN;
    const int krp_correction = t->shared_history->krp_corrhist[side][t->pos.krpKey & mask] * 256 / KRP_CORRHIST_GRAIN;
    const int white_non_pawn_correction = t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    const int black_non_pawn_correction = t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    const int np_correction = (white_non_pawn_correction + black_non_pawn_correction) * 256 / NON_PAWN_CORRHIST_GRAIN;
    const int continuation_correction = adjust_single_cont_corrhist_entry(t, 2, ss) * 256 / CONT_CORRHIST_GRAIN;
    
    int correction = pawn_correction + minor_correction + major_correction +
                    krp_correction + np_correction +
                    continuation_correction;

    return correction;
}

void clear_histories(void) {
    int how_many_threads = thread_pool.thread_count;

    for (int i = 0; i < how_many_threads; i++) {
        memset(thread_pool.threads[i]->search_d.quietHistory, 0, sizeof(thread_pool.threads[i]->search_d.quietHistory));
        memset(thread_pool.threads[i]->search_d.captureHistory, 0, sizeof(thread_pool.threads[i]->search_d.captureHistory));        
    }
    
    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        SharedHistory *sh = thread_pool.shared_histories[i];
        memset(sh->pawnHistory, 0, sizeof(sh->pawnHistory));
        memset(sh->contCorrhist, 0, sizeof(sh->contCorrhist));
        memset(sh->continuationHistory, 0, sizeof(sh->continuationHistory));
        for (int c = 0; c < 2; c++) {
            memset(sh->pawn_corrhist[c], 0, (sh->corrhist_mask + 1) * sizeof(int16_t));
            memset(sh->minor_corrhist[c], 0, (sh->corrhist_mask + 1) * sizeof(int16_t));
            memset(sh->major_corrhist[c], 0, (sh->corrhist_mask + 1) * sizeof(int16_t));
            memset(sh->krp_corrhist[c], 0, (sh->corrhist_mask + 1) * sizeof(int16_t));
            for (int c2 = 0; c2 < 2; c2++) {
                memset(sh->non_pawn_corrhist[c][c2], 0, (sh->corrhist_mask + 1) * sizeof(int16_t));
            }
        }
    }
    
    int threads_per_l3 = 8;
    for (int i = 0; i < how_many_threads; i++) {
        thread_pool.threads[i]->shared_history = thread_pool.shared_histories[i / threads_per_l3];
    }
}

void quiet_history_aging(void) {
    int how_many_threads = thread_pool.thread_count;

    for (int i = 0;i < how_many_threads;i++) {
        int16_t *p = (int16_t *)thread_pool.threads[i]->search_d.quietHistory;

        for (int j = 0; j < 32768; j++) {
            p[j] >>= 1;
        }
    }            
}
