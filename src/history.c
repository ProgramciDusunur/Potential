//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"


/*╔═════════╗
  ║ History ║
  ╚═════════╝*/

// quietHistory[side to move][fromSquare][toSquare][threatSource][threatTarget]
int16_t quietHistory[2][64][64][2][2];

// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
int16_t continuationHistory[12][64][12][64];

// continuationCorrectionHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
int16_t contCorrhist[12][64][12][64];

// pawnHistory [pawnKey][piece][to]
int16_t pawnHistory[2048][12][64];

// captureHistory [piece][toSquare][capturedPiece]
int16_t captureHistory[12][64][13];

/*╔════════════════════╗
  ║ Correction History ║
  ╚════════════════════╝*/

int CORRHIST_WEIGHT_SCALE = 256;
int CORRHIST_GRAIN = 256;
int CORRHIST_LIMIT = 1024;
int CORRHIST_SIZE = 16384;
int CORRHIST_MAX = 16384;

// pawn correction history [side to move][key]
int16_t PAWN_CORRECTION_HISTORY[2][16384];

// minor correction history [side to move][key]
int16_t MINOR_CORRECTION_HISTORY[2][16384];

// major correction history [side to move][key]
int16_t MAJOR_CORRECTION_HISTORY[2][16384];

// non pawn correction history [side to move][key]
int16_t NON_PAWN_CORRECTION_HISTORY[2][2][16384];

// king rook pawn correction history [side to move][key]
int16_t krpCorrhist[2][16384];

/* Update History */

int getHistoryBonus(int depth) {
    return myMIN(10 + 200 * depth, 4096);
}

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void adjust_single_quiet_hist_entry(board *pos, int side, uint16_t move, int bonus) {
    int from = getMoveSource(move);
    int to = getMoveTarget(move);

    bool threatSource = is_square_threatened(pos, from);
    bool threatTarget = is_square_threatened(pos, to);
    
    quietHistory[side][from][to][threatSource][threatTarget] += bonus;
}

void updateQuietMoveHistory(uint16_t bestMove, int side, int depth, moves *badQuiets, board *pos) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);
    int score = quietHistory[side][from][to][is_square_threatened(pos, from)][is_square_threatened(pos, to)];

    quietHistory[side][from][to][is_square_threatened(pos, from)][is_square_threatened(pos, to)] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(pos, badQuietFrom)][is_square_threatened(pos, badQuietTo)];
        int adjusted_malus_bonus = bonus * 1280 / 1024;
        int scaled_bonus = adjusted_malus_bonus + index * 30;

        quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(pos, badQuietFrom)][is_square_threatened(pos, badQuietTo)] +=
        scaledBonus(badQuietScore, -scaled_bonus, maxQuietHistory);
    }
}

void updatePawnHistory(board *pos, uint16_t bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);
    int score = pawnHistory[pos->pawnKey % 2048][pos->mailbox[from]][to];

    pawnHistory[pos->pawnKey % 2048][pos->mailbox[from]][to] += scaledBonus(score, bonus, maxPawnHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;

        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        pawnHistory[pos->pawnKey % 2048][pos->mailbox[badQuietFrom]][badQuietTo] += scaledBonus(score, -bonus, maxPawnHistory);
    }
}

void updateCaptureHistory(board *position, uint16_t bestMove, int depth) {
    int piece = position->mailbox[getMoveSource(bestMove)];
    int to = getMoveTarget(bestMove);
    int capturedPiece = position->mailbox[getMoveTarget(bestMove)];

    int bonus = getHistoryBonus(depth);
    int score = captureHistory[piece][to][capturedPiece];

    captureHistory[piece][to][capturedPiece] += scaledBonus(score, bonus, maxCaptureHistory);
}

void updateCaptureHistoryMalus(board *position, int depth, moves *noisyMoves, uint16_t bestMove) {
    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyPiece = position->mailbox[getMoveSource(noisyMoves->moves[index])];
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);
        int noisyCapturedPiece = position->mailbox[getMoveTarget(noisyMoves->moves[index])];

        if (noisyMoves->moves[index] == bestMove) continue;

        int noisyMoveScore = captureHistory[noisyPiece][noisyTo][noisyCapturedPiece];        

        captureHistory[noisyPiece][noisyTo][noisyCapturedPiece] += scaledBonus(noisyMoveScore, -getHistoryBonus(depth), maxCaptureHistory);
    }
}

int getAllCHScore(board *pos, uint16_t move, int quiet_hist_score) {
    return (getContinuationHistoryScore(pos, 1, move) + quiet_hist_score) / 2 +
           getContinuationHistoryScore(pos, 2, move) +
           getContinuationHistoryScore(pos, 4, move);
}

int getContinuationHistoryScore(board *pos, int offSet, uint16_t move) {
    const int ply = pos->ply - offSet;
    return ply >= 0 ? continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)] : 0;
}

void updateSingleCHScore(board *pos, uint16_t move, const int offSet, const int bonus, int quiet_hist_score) {
    int base_conthist_score = getAllCHScore(pos, move, quiet_hist_score);
    const int ply = pos->ply - offSet;
    if (ply >= 0) {
        const int scaledBonus = bonus - base_conthist_score * abs(bonus) / maxQuietHistory;
        continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)] += scaledBonus;
    }
}

void updateAllCH(board *pos, uint16_t move, int bonus, int quiet_hist_score) {
    updateSingleCHScore(pos, move, 1, bonus, quiet_hist_score);
    updateSingleCHScore(pos, move, 2, bonus, quiet_hist_score);
    updateSingleCHScore(pos, move, 4, bonus, quiet_hist_score);
}

void updateContinuationHistory(board *pos, uint16_t bestMove, int depth, moves *badQuiets, int quiet_hist_score) {
    int bonus = getHistoryBonus(depth);

    updateAllCH(pos, bestMove, bonus, quiet_hist_score);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        updateAllCH(pos, badQuiets->moves[index], -bonus, quiet_hist_score);
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

void update_pawn_correction_hist(board *position, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    // Masking for faster indexing (assuming SIZE is power of 2)
    int16_t *entry = &PAWN_CORRECTION_HISTORY[position->side][position->pawnKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_minor_correction_hist(board *position, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &MINOR_CORRECTION_HISTORY[position->side][position->minorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_major_correction_hist(board *position, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &MAJOR_CORRECTION_HISTORY[position->side][position->majorKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_non_pawn_corrhist(board *position, const int depth, const int diff) {    
    const int side = position->side;
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    const int mask = CORRHIST_SIZE - 1;
    
    int16_t *white_ptr = &NON_PAWN_CORRECTION_HISTORY[white][side][position->whiteNonPawnKey & mask];
    apply_corrhist_update(white_ptr, scaledDiff, newWeight);
    
    int16_t *black_ptr = &NON_PAWN_CORRECTION_HISTORY[black][side][position->blackNonPawnKey & mask];
    apply_corrhist_update(black_ptr, scaledDiff, newWeight);
}

void update_king_rook_pawn_corrhist(board *position, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);
    
    int16_t *entry = &krpCorrhist[position->side][position->krpKey & (CORRHIST_SIZE - 1)];
    apply_corrhist_update(entry, scaledDiff, newWeight);
}

void update_single_cont_corrhist_entry(board *pos, const int pliesBack, const int scaledDiff, const int newWeight) {    
    if (pos->ply < pliesBack) return;

    const int idx1 = pos->ply - pliesBack;
    const int idx2 = pos->ply;
    
    const int m1 = pos->move[idx1];
    const int m2 = pos->move[idx2];
    
    if (m1 && m2) {        
        int16_t *entry_ptr = &contCorrhist[pos->piece[idx1]][getMoveTarget(m1)]
                                         [pos->piece[idx2]][getMoveTarget(m2)];
                
        int val = *entry_ptr;
        val = (val * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
                
        if (val > CORRHIST_MAX) val = CORRHIST_MAX;
        else if (val < -CORRHIST_MAX) val = -CORRHIST_MAX;

        *entry_ptr = (int16_t)val;
    }
}

static inline int adjust_single_cont_corrhist_entry(board *pos, const int pliesBack) {    
    if (pos->ply >= pliesBack) {
        const int m1 = pos->move[pos->ply - pliesBack];
        const int m2 = pos->move[pos->ply];

        if (m1 && m2) {
            return contCorrhist[pos->piece[pos->ply - pliesBack]][getMoveTarget(m1)]
                               [pos->piece[pos->ply]][getMoveTarget(m2)];
        }
    }
    return 0;
}

void update_continuation_corrhist(board *pos, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    update_single_cont_corrhist_entry(pos, 2, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(pos, 3, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(pos, 4, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(pos, 5, scaledDiff, newWeight);
}

int adjust_eval_with_corrhist(board *pos, int rawEval) {       
    rawEval = (rawEval * (300 - pos->fifty)) / 300;
    
    const int side = pos->side;
    const int mask = CORRHIST_SIZE - 1;

    // Batch memory access    
    int adjust = PAWN_CORRECTION_HISTORY[side][pos->pawnKey & mask]
               + MINOR_CORRECTION_HISTORY[side][pos->minorKey & mask]
               + MAJOR_CORRECTION_HISTORY[side][pos->majorKey & mask]
               + krpCorrhist[side][pos->krpKey & mask]
               + NON_PAWN_CORRECTION_HISTORY[white][side][pos->whiteNonPawnKey & mask]
               + NON_PAWN_CORRECTION_HISTORY[black][side][pos->blackNonPawnKey & mask]
               + adjust_single_cont_corrhist_entry(pos, 2)
               + adjust_single_cont_corrhist_entry(pos, 3)               
               + adjust_single_cont_corrhist_entry(pos, 4)
               + adjust_single_cont_corrhist_entry(pos, 5);

    const int mateFound = mateValue - maxPly;
    
    rawEval += adjust / CORRHIST_GRAIN;
    
    if (rawEval >= mateFound) return mateFound - 1;
    if (rawEval <= -mateFound) return -mateFound + 1;
    
    return rawEval;
}

int get_correction_value(board *pos) {
    const int side = pos->side;
    const int mask = CORRHIST_SIZE - 1;

    const int pawn_correction = PAWN_CORRECTION_HISTORY[side][pos->pawnKey & mask];
    const int minor_correction = MINOR_CORRECTION_HISTORY[side][pos->minorKey & mask];
    const int major_correction = MAJOR_CORRECTION_HISTORY[side][pos->majorKey & mask];
    const int krp_correction = krpCorrhist[side][pos->krpKey & mask];
    const int white_non_pawn_correction = NON_PAWN_CORRECTION_HISTORY[white][side][pos->whiteNonPawnKey & mask];
    const int black_non_pawn_correction = NON_PAWN_CORRECTION_HISTORY[black][side][pos->blackNonPawnKey & mask];
    const int continuation_correction = adjust_single_cont_corrhist_entry(pos, 2);
    
    int correction = pawn_correction + minor_correction + major_correction +
                    krp_correction + white_non_pawn_correction + black_non_pawn_correction +
                    continuation_correction;

    return correction;
}

void clear_histories(void) {
    memset(quietHistory, 0, sizeof(quietHistory));            
    memset(captureHistory, 0, sizeof(captureHistory));
    memset(PAWN_CORRECTION_HISTORY, 0, sizeof(PAWN_CORRECTION_HISTORY));
    memset(pawnHistory, 0, sizeof(pawnHistory));
    memset(continuationHistory, 0, sizeof(continuationHistory));
    memset(MINOR_CORRECTION_HISTORY, 0, sizeof(PAWN_CORRECTION_HISTORY));
    memset(MAJOR_CORRECTION_HISTORY, 0, sizeof(MAJOR_CORRECTION_HISTORY));
    memset(NON_PAWN_CORRECTION_HISTORY, 0, sizeof(NON_PAWN_CORRECTION_HISTORY));
    memset(contCorrhist, 0, sizeof(contCorrhist));
    memset(krpCorrhist, 0, sizeof(krpCorrhist));
}

void quiet_history_aging(void) {    
    int16_t *p = (int16_t *)quietHistory;
    
    for (int i = 0; i < 32768; i++) {
        p[i] >>= 1;
    }
}
