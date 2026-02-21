//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"


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
    int score = t->search_d.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to];

    t->search_d.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[from]][to] += scaledBonus(score, bonus, maxPawnHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;

        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        t->search_d.pawnHistory[t->pos.pawnKey % 2048][t->pos.mailbox[badQuietFrom]][badQuietTo] += scaledBonus(score, -bonus, maxPawnHistory);
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

int getAllCHScore(ThreadData *t, uint16_t move, int quiet_hist_score) {
    return (getContinuationHistoryScore(t, 1, move) + quiet_hist_score) / 2 +
           getContinuationHistoryScore(t, 2, move) +
           getContinuationHistoryScore(t, 4, move);
}

int getContinuationHistoryScore(ThreadData *t, int offSet, uint16_t move) {
    const int ply = t->pos.ply - offSet;
    return ply >= 0 ? t->search_d.continuationHistory[t->pos.piece[ply]][getMoveTarget(t->pos.move[ply])]
                              [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)] : 0;
}

void updateSingleCHScore(ThreadData *t, uint16_t move, const int offSet, const int bonus, int quiet_hist_score) {
    int base_conthist_score = getAllCHScore(t, move, quiet_hist_score);
    const int ply = t->pos.ply - offSet;
    if (ply >= 0) {
        const int scaledBonus = bonus - base_conthist_score * abs(bonus) / maxQuietHistory;
        t->search_d.continuationHistory[t->pos.piece[ply]][getMoveTarget(t->pos.move[ply])]
                              [t->pos.mailbox[getMoveSource(move)]][getMoveTarget(move)] += scaledBonus;
    }
}

void updateAllCH(ThreadData *t, uint16_t move, int bonus, int quiet_hist_score) {
    updateSingleCHScore(t, move, 1, bonus, quiet_hist_score);
    updateSingleCHScore(t, move, 2, bonus, quiet_hist_score);
    updateSingleCHScore(t, move, 4, bonus, quiet_hist_score);
}

void updateContinuationHistory(ThreadData *t, uint16_t bestMove, int depth, moves *badQuiets, int quiet_hist_score) {
    int bonus = getHistoryBonus(depth);

    updateAllCH(t, bestMove, bonus, quiet_hist_score);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        updateAllCH(t, badQuiets->moves[index], -bonus, quiet_hist_score);
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
    int16_t *entry = &t->search_d.pawn_corrhist[t->pos.side][t->pos.pawnKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_minor_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->search_d.minor_corrhist[t->pos.side][t->pos.minorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_major_correction_hist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->search_d.major_corrhist[t->pos.side][t->pos.majorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_non_pawn_corrhist(ThreadData *t, const int depth, const int diff) {    
    const int side = t->pos.side;
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    const int mask = CORRHIST_SIZE - 1;
    
    int16_t *white_ptr = &t->search_d.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    apply_corrhist_update(white_ptr, scaledDiff, newWeight);
    
    int16_t *black_ptr = &t->search_d.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    apply_corrhist_update(black_ptr, scaledDiff, newWeight);
}

void update_king_rook_pawn_corrhist(ThreadData *t, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &t->search_d.krp_corrhist[t->pos.side][t->pos.krpKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_single_cont_corrhist_entry(ThreadData *t, const int pliesBack, const int scaledDiff, const int newWeight) {    
    if (t->pos.ply < pliesBack) return;

    const int idx1 = t->pos.ply - pliesBack;
    const int idx2 = t->pos.ply;
    
    const int m1 = t->pos.move[idx1];
    const int m2 = t->pos.move[idx2];
    
    if (m1 && m2) {        
        int16_t *entry_ptr = &t->search_d.contCorrhist[t->pos.piece[idx1]][getMoveTarget(m1)]
                                         [t->pos.piece[idx2]][getMoveTarget(m2)];
                
        int val = *entry_ptr;
        val = (val * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
                
        if (val > CORRHIST_MAX) val = CORRHIST_MAX;
        else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

        *entry_ptr = (int16_t)val;
    }
}

static inline int adjust_single_cont_corrhist_entry(ThreadData *t, const int pliesBack) {    
    if (t->pos.ply >= pliesBack) {
        const int m1 = t->pos.move[t->pos.ply - pliesBack];
        const int m2 = t->pos.move[t->pos.ply];

        if (m1 && m2) {
            return t->search_d.contCorrhist[t->pos.piece[t->pos.ply - pliesBack]][getMoveTarget(m1)]
                               [t->pos.piece[t->pos.ply]][getMoveTarget(m2)];
        }
    }
    return 0;
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
    rawEval = (rawEval * (300 - t->pos.fifty)) / 300;
    
    const int side = t->pos.side;
    const int mask = CORRHIST_SIZE - 1;

    // Batch memory access    
    int adjust = t->search_d.pawn_corrhist[side][t->pos.pawnKey & mask]
               + t->search_d.minor_corrhist[side][t->pos.minorKey & mask]
               + t->search_d.major_corrhist[side][t->pos.majorKey & mask]
               + t->search_d.krp_corrhist[side][t->pos.krpKey & mask]
               + t->search_d.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask]
               + t->search_d.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask]
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
    const int side = t->pos.side;
    const int mask = CORRHIST_SIZE - 1;

    const int pawn_correction = t->search_d.pawn_corrhist[side][t->pos.pawnKey & mask];
    const int minor_correction = t->search_d.minor_corrhist[side][t->pos.minorKey & mask];
    const int major_correction = t->search_d.major_corrhist[side][t->pos.majorKey & mask];
    const int krp_correction = t->search_d.krp_corrhist[side][t->pos.krpKey & mask];
    const int white_non_pawn_correction = t->search_d.non_pawn_corrhist[white][side][t->pos.whiteNonPawnKey & mask];
    const int black_non_pawn_correction = t->search_d.non_pawn_corrhist[black][side][t->pos.blackNonPawnKey & mask];
    const int continuation_correction = adjust_single_cont_corrhist_entry(t, 2);
    
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
        memset(thread_pool.threads[i]->search_d.pawnHistory, 0, sizeof(thread_pool.threads[i]->search_d.pawnHistory));
        memset(thread_pool.threads[i]->search_d.continuationHistory, 0, sizeof(thread_pool.threads[i]->search_d.continuationHistory));


        memset(thread_pool.threads[i]->search_d.contCorrhist, 0, sizeof(thread_pool.threads[i]->search_d.contCorrhist));
        memset(thread_pool.threads[i]->search_d.pawn_corrhist, 0, sizeof(thread_pool.threads[i]->search_d.pawn_corrhist));
        memset(thread_pool.threads[i]->search_d.minor_corrhist, 0, sizeof(thread_pool.threads[i]->search_d.minor_corrhist));
        memset(thread_pool.threads[i]->search_d.major_corrhist, 0, sizeof(thread_pool.threads[i]->search_d.major_corrhist));
        memset(thread_pool.threads[i]->search_d.non_pawn_corrhist, 0, sizeof(thread_pool.threads[i]->search_d.non_pawn_corrhist));    
        memset(thread_pool.threads[i]->search_d.krp_corrhist, 0, sizeof(thread_pool.threads[i]->search_d.krp_corrhist));
    }                        
}

void quiet_history_aging(void) {
    int how_many_threads = thread_pool.thread_count;

    for (int i = 0;i < how_many_threads;i++) {
        int16_t *p = (int16_t *)thread_pool.threads[i]->search_d.quietHistory;

        for (int i = 0; i < 32768; i++) {
            p[i] >>= 1;
        }
    }            
}
