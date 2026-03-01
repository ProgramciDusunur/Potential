//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"
#include "threads.h"


/*╔═════════╗
  ║ History ║
  ╚═════════╝*/


/*╔════════════════════╗
  ║ Correction History ║
  ╚════════════════════╝*/

int CORRHIST_WEIGHT_SCALE = 256;
int CORRHIST_GRAIN = 256;
int CORRHIST_LIMIT = 1024;
int CORRHIST_SIZE = 16384;
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

void updateQuietMoveHistory(ThreadData *t, uint16_t bestMove, int side, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);
    int score = t->search_d.quietHistory[side][from][to][is_square_threatened(&t->pos, from)][is_square_threatened(&t->pos, to)];

    t->search_d.quietHistory[side][from][to][is_square_threatened(&t->pos, from)][is_square_threatened(&t->pos, to)] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = t->search_d.quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(&t->pos, badQuietFrom)][is_square_threatened(&t->pos, badQuietTo)];
        int scaled_bonus = bonus + index * 30;

        t->search_d.quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(&t->pos, badQuietFrom)][is_square_threatened(&t->pos, badQuietTo)] +=
        scaledBonus(badQuietScore, -scaled_bonus, maxQuietHistory);
    }
}

void updatePawnHistory(ThreadData *t, uint16_t bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);
    int score = thread_pool.shared_history.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to];

    thread_pool.shared_history.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to] += scaledBonus(score, bonus, maxPawnHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;

        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        thread_pool.shared_history.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[badQuietFrom]][badQuietTo] += scaledBonus(score, -bonus, maxPawnHistory);
    }
}

void updateCaptureHistory(ThreadData *t, uint16_t bestMove, int depth) {
    int piece = t->pos.mailbox[getMoveSource(bestMove)];
    int to = getMoveTarget(bestMove);
    int capturedPiece = t->pos.mailbox[getMoveTarget(bestMove)];

    int bonus = getHistoryBonus(depth);
    int score = t->search_d.captureHistory[piece][to][capturedPiece];

    t->search_d.captureHistory[piece][to][capturedPiece] += scaledBonus(score, bonus, maxCaptureHistory);
}

void updateCaptureHistoryMalus(ThreadData *t, int depth, moves *noisyMoves, uint16_t bestMove) {
    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyPiece = t->pos.mailbox[getMoveSource(noisyMoves->moves[index])];
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);
        int noisyCapturedPiece = t->pos.mailbox[getMoveTarget(noisyMoves->moves[index])];

        if (noisyMoves->moves[index] == bestMove) continue;

        int noisyMoveScore = t->search_d.captureHistory[noisyPiece][noisyTo][noisyCapturedPiece];        

        t->search_d.captureHistory[noisyPiece][noisyTo][noisyCapturedPiece] += scaledBonus(noisyMoveScore, -getHistoryBonus(depth), maxCaptureHistory);
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
    const int scaledBonus = bonus - base_conthist_score * abs(bonus) / maxQuietHistory;
    t->search_d.continuationHistory[prev->piece][getMoveTarget(prev->move)]
                          [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)] += scaledBonus;
}

void updateAllCH(ThreadData *t, uint16_t move, int bonus, int quiet_hist_score, SearchStack *ss) {
    updateSingleCHScore(t, move, 1, bonus, quiet_hist_score, ss);
    updateSingleCHScore(t, move, 2, bonus, quiet_hist_score, ss);
    updateSingleCHScore(t, move, 4, bonus, quiet_hist_score, ss);
}

void updateContinuationHistory(ThreadData *t, uint16_t bestMove, int depth, moves *badQuiets, int quiet_hist_score, SearchStack *ss) {
    int bonus = getHistoryBonus(depth);

    updateAllCH(t, bestMove, bonus, quiet_hist_score, ss);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        updateAllCH(t, badQuiets->moves[index], -bonus, quiet_hist_score, ss);
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
    int16_t *entry = &thread_pool.shared_history.pawn_corrhist[t->pos.side][t->pos.pawnKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_minor_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &thread_pool.shared_history.minor_corrhist[t->pos.side][t->pos.minorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_major_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &thread_pool.shared_history.major_corrhist[t->pos.side][t->pos.majorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_non_pawn_corrhist(ThreadData *t, const int depth, const int diff) {    
    const int side = t->pos.side;
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    const int mask = CORRHIST_SIZE - 1;
    
    int16_t *white_ptr = &thread_pool.shared_history.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    apply_corrhist_update(white_ptr, scaledDiff, newWeight);
    
    int16_t *black_ptr = &thread_pool.shared_history.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    apply_corrhist_update(black_ptr, scaledDiff, newWeight);
}

void update_king_rook_pawn_corrhist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &thread_pool.shared_history.krp_corrhist[t->pos.side][t->pos.krpKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, const int scaledDiff, const int newWeight, SearchStack *ss) {
    if (t->pos.ply < pliesBack) return;
    SearchStack *prev = ss - pliesBack;

    const int m1 = prev->move;
    const int m2 = ss->move;
    
    if (m1 && m2) {        
        int16_t *entry_ptr = &thread_pool.shared_history.contCorrhist[prev->piece][getMoveTarget(m1)]
                                         [ss->piece][getMoveTarget(m2)];
                
        int val = *entry_ptr;
        val = (val * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
                
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
        return thread_pool.shared_history.contCorrhist[prev->piece][getMoveTarget(m1)]
                               [ss->piece][getMoveTarget(m2)];
    }
    return 0;
}

void update_continuation_corrhist(ThreadData *t, const int depth, const int diff, SearchStack *ss) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
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
    const int mask = CORRHIST_SIZE - 1;

    // Batch memory access    
    int adjust = thread_pool.shared_history.pawn_corrhist[side][t->pos.pawnKey & mask]
               + thread_pool.shared_history.minor_corrhist[side][t->pos.minorKey & mask]
               + thread_pool.shared_history.major_corrhist[side][t->pos.majorKey & mask]
               + thread_pool.shared_history.krp_corrhist[side][t->pos.krpKey & mask]
               + thread_pool.shared_history.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask]
               + thread_pool.shared_history.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask]
               + adjust_single_cont_corrhist_entry(t, 1, ss)
               + adjust_single_cont_corrhist_entry(t, 2, ss)
               + adjust_single_cont_corrhist_entry(t, 3, ss)               
               + adjust_single_cont_corrhist_entry(t, 4, ss)
               + adjust_single_cont_corrhist_entry(t, 5, ss);

    const int mateFound = mateValue - maxPly;
    
    rawEval += adjust / CORRHIST_GRAIN;
    
    if (rawEval >= mateFound) return mateFound - 1;
    if (rawEval <= -mateFound) return -mateFound + 1;
    
    return rawEval;
}

int get_correction_value(ThreadData *t, SearchStack *ss) {
    const int side = t->pos.side;
    const int mask = CORRHIST_SIZE - 1;

    const int pawn_correction = thread_pool.shared_history.pawn_corrhist[side][t->pos.pawnKey & mask];
    const int minor_correction = thread_pool.shared_history.minor_corrhist[side][t->pos.minorKey & mask];
    const int major_correction = thread_pool.shared_history.major_corrhist[side][t->pos.majorKey & mask];
    const int krp_correction = thread_pool.shared_history.krp_corrhist[side][t->pos.krpKey & mask];
    const int white_non_pawn_correction = thread_pool.shared_history.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    const int black_non_pawn_correction = thread_pool.shared_history.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    const int continuation_correction = adjust_single_cont_corrhist_entry(t, 2, ss);
    
    int correction = pawn_correction + minor_correction + major_correction +
                    krp_correction + white_non_pawn_correction + black_non_pawn_correction +
                    continuation_correction;

    return correction;
}

void clear_histories(void) {
    int how_many_threads = thread_pool.thread_count;

    for (int i = 0;i < how_many_threads;i++) {
        memset(thread_pool.threads[i]->search_d.quietHistory, 0, sizeof(thread_pool.threads[i]->search_d.quietHistory));
        memset(thread_pool.threads[i]->search_d.captureHistory, 0, sizeof(thread_pool.threads[i]->search_d.captureHistory));        
        memset(thread_pool.threads[i]->search_d.continuationHistory, 0, sizeof(thread_pool.threads[i]->search_d.continuationHistory));
    }                        

    memset(&thread_pool.shared_history, 0, sizeof(SharedHistory));
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
