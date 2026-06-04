//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"
#include "threads.h"
#include "search.h"


/*╔═════════╗
  ║ History ║
  ╚═════════╝*/


/*╔════════════════════╗
  ║ Correction History ║
  ╚════════════════════╝*/

int CORRHIST_WEIGHT_SCALE = 256;
int CORRHIST_GRAIN = 256;
int CORRHIST_LIMIT = 1024;
int BASE_CORRHIST_SIZE = 16384;
int BASE_CONT_CORRHIST_SIZE = 1048576;
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
    return t->search_d.continuationHistory[prev->piece][getMoveTarget(prev->move)]
                              [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)];
}

void updateSingleCHScore(ThreadData *t, uint16_t move, const int offSet, const int bonus, int quiet_hist_score, SearchStack *ss) {
    if (t->pos.ply < offSet) return;
    int base_conthist_score = getAllCHScore(t, move, quiet_hist_score, ss);
    SearchStack *prev = ss - offSet;
    const int scaledBonus = bonus - (base_conthist_score * abs(bonus) * CONTHIST_MULT) / 16384;
    t->search_d.continuationHistory[prev->piece][getMoveTarget(prev->move)]
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

static inline void apply_corrhist_update(int16_t *entry, const int scaledDiff, const int newWeight) {    
    int val = *entry;
    val = (val * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
        
    if (val > CORRHIST_MAX) val = CORRHIST_MAX;
    else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

    *entry = val;
}

void update_pawn_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    // Masking for faster indexing (assuming SIZE is power of 2)
    int16_t *entry = &t->shared_history->pawn_corrhist[t->pos.side][t->pos.pawnKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_minor_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->minor_corrhist[t->pos.side][t->pos.minorKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_major_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->major_corrhist[t->pos.side][t->pos.majorKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_non_pawn_corrhist(ThreadData *t, const int depth, const int diff) {    
    const int side = t->pos.side;
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    const int mask = t->shared_history->corrhist_mask;
    
    int16_t *white_ptr = &t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    apply_corrhist_update(white_ptr, scaledDiff, newWeight);
    
    int16_t *black_ptr = &t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    apply_corrhist_update(black_ptr, scaledDiff, newWeight);
}

void update_king_rook_pawn_corrhist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->shared_history->krp_corrhist[t->pos.side][t->pos.krpKey & t->shared_history->corrhist_mask];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, const int scaledDiff, const int newWeight) {
    if (t->pos.ply < pliesBack) return;

    uint64_t diff_key = t->pos.hashKey ^ t->pos.repetitionTable[t->pos.repetitionIndex - pliesBack + 1];
    int16_t *entry_ptr = &t->shared_history->contCorrhist[diff_key & t->shared_history->cont_mask];
            
    int val = *entry_ptr;
    val = (val * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
            
    if (val > CORRHIST_MAX) val = CORRHIST_MAX;
    else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

    *entry_ptr = (int16_t)val;
}

static inline int adjust_single_cont_corrhist_entry(ThreadData *t, const int pliesBack) {
    if (t->pos.ply < pliesBack) return 0;
    
    uint64_t diff_key = t->pos.hashKey ^ t->pos.repetitionTable[t->pos.repetitionIndex - pliesBack + 1];
    return t->shared_history->contCorrhist[diff_key & t->shared_history->cont_mask];
}

void update_continuation_corrhist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    update_single_cont_corrhist_entry(t, 1, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(t, 2, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(t, 3, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(t, 4, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(t, 5, scaledDiff, newWeight);
}

int adjust_eval_with_corrhist(ThreadData *t, int rawEval) { 
    if (abs(rawEval) > 4000) return rawEval;
    
    int correction = get_correction_value(t) / 300;
    
    const int side = t->pos.side;
    const int mask = t->shared_history->corrhist_mask;

    // Batch memory access    
    int adjust = t->shared_history->pawn_corrhist[side][t->pos.pawnKey & mask]
               + t->shared_history->minor_corrhist[side][t->pos.minorKey & mask]
               + t->shared_history->major_corrhist[side][t->pos.majorKey & mask]
               + t->shared_history->krp_corrhist[side][t->pos.krpKey & mask]
               + t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask]
               + t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask]
               + adjust_single_cont_corrhist_entry(t, 1)
               + adjust_single_cont_corrhist_entry(t, 2)
               + adjust_single_cont_corrhist_entry(t, 3)               
               + adjust_single_cont_corrhist_entry(t, 4)
               + adjust_single_cont_corrhist_entry(t, 5);

    const int mateFound = mateValue - maxPly;
    
    rawEval += adjust / CORRHIST_GRAIN;
    
    if (rawEval >= mateFound) return mateFound - 1;
    if (rawEval <= -mateFound) return -mateFound + 1;
    
    return rawEval;
}

int get_correction_value(ThreadData *t) {
    int side = t->pos.side;
    const int mask = t->shared_history->corrhist_mask;
    
    const int pawn_correction = t->shared_history->pawn_corrhist[side][t->pos.pawnKey & mask];
    const int minor_correction = t->shared_history->minor_corrhist[side][t->pos.minorKey & mask];
    const int major_correction = t->shared_history->major_corrhist[side][t->pos.majorKey & mask];
    const int krp_correction = t->shared_history->krp_corrhist[side][t->pos.krpKey & mask];
    const int white_non_pawn_correction = t->shared_history->non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    const int black_non_pawn_correction = t->shared_history->non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    const int continuation_correction = adjust_single_cont_corrhist_entry(t, 1)
               + adjust_single_cont_corrhist_entry(t, 2)
               + adjust_single_cont_corrhist_entry(t, 3)               
               + adjust_single_cont_corrhist_entry(t, 4)
               + adjust_single_cont_corrhist_entry(t, 5);  
    int correction = pawn_correction + minor_correction + major_correction +
                    krp_correction + white_non_pawn_correction + black_non_pawn_correction +
                    continuation_correction;

    return correction;
}

void clear_histories(void) {
    int how_many_threads = thread_pool.thread_count;

    for (int i = 0; i < how_many_threads; i++) {
        memset(thread_pool.threads[i]->search_d.quietHistory, 0, sizeof(thread_pool.threads[i]->search_d.quietHistory));
        memset(thread_pool.threads[i]->search_d.captureHistory, 0, sizeof(thread_pool.threads[i]->search_d.captureHistory));        
        memset(thread_pool.threads[i]->search_d.continuationHistory, 0, sizeof(thread_pool.threads[i]->search_d.continuationHistory));
    }
    
    for (int i = 0; i < thread_pool.shared_history_count; i++) {
        SharedHistory *sh = thread_pool.shared_histories[i];
        memset(sh->pawnHistory, 0, sizeof(sh->pawnHistory));
        memset(sh->contCorrhist, 0, (sh->cont_mask + 1) * sizeof(int16_t));
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
