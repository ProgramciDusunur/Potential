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

        quietHistory[side][badQuietFrom][badQuietTo][is_square_threatened(pos, badQuietFrom)][is_square_threatened(pos, badQuietTo)] +=
        scaledBonus(badQuietScore, -bonus, maxQuietHistory);
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

void update_pawn_correction_hist(board *position, const int depth, const int diff) {
    U64 pawnKey = position->pawnKey;

    int entry = PAWN_CORRECTION_HISTORY[position->side][pawnKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    PAWN_CORRECTION_HISTORY[position->side][pawnKey % CORRHIST_SIZE] = entry;
}

void update_minor_correction_hist(board *position, const int depth, const int diff) {
    U64 minorKey = position->minorKey;

    int entry = MINOR_CORRECTION_HISTORY[position->side][minorKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    MINOR_CORRECTION_HISTORY[position->side][minorKey % CORRHIST_SIZE] = entry;
}

void update_major_correction_hist(board *position, const int depth, const int diff) {
    U64 majorKey = position->majorKey;

    int entry = MAJOR_CORRECTION_HISTORY[position->side][majorKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    MAJOR_CORRECTION_HISTORY[position->side][majorKey % CORRHIST_SIZE] = entry;
}

void update_non_pawn_corrhist(board *position, const int depth, const int diff) {
    U64 whiteKey = position->whiteNonPawnKey;
    U64 blackKey = position->blackNonPawnKey;

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    int whiteEntry = NON_PAWN_CORRECTION_HISTORY[white][position->side][whiteKey % CORRHIST_SIZE];

    whiteEntry = (whiteEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    whiteEntry = clamp(whiteEntry, -CORRHIST_MAX, CORRHIST_MAX);

    int blackEntry = NON_PAWN_CORRECTION_HISTORY[black][position->side][blackKey % CORRHIST_SIZE];

    blackEntry = (blackEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    blackEntry = clamp(blackEntry, -CORRHIST_MAX, CORRHIST_MAX);

    NON_PAWN_CORRECTION_HISTORY[white][position->side][whiteKey % CORRHIST_SIZE] = whiteEntry;
    NON_PAWN_CORRECTION_HISTORY[black][position->side][blackKey % CORRHIST_SIZE] = blackEntry;
}

void update_single_cont_corrhist_entry(board *pos, const int pliesBack, const int scaledDiff, const int newWeight) {
    if (pos->ply >= pliesBack && pos->move[pos->ply - (pliesBack - 1)] && pos->move[pos->ply - pliesBack]) {
        int contCorrhistEntry = contCorrhist[pos->piece[pos->ply - (pliesBack - 1)]][getMoveTarget(pos->move[pos->ply - (pliesBack - 1)])]
                    [pos->piece[pos->ply - pliesBack]][getMoveTarget(pos->move[pos->ply - pliesBack])];
        
        contCorrhistEntry = (contCorrhistEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
        contCorrhistEntry = clamp(contCorrhistEntry, -CORRHIST_MAX, CORRHIST_MAX);

        contCorrhist[pos->piece[pos->ply - (pliesBack - 1)]][getMoveTarget(pos->move[pos->ply - (pliesBack - 1)])]
                    [pos->piece[pos->ply - pliesBack]][getMoveTarget(pos->move[pos->ply - pliesBack])] = contCorrhistEntry;

    }
}

int adjust_single_cont_corrhist_entry(board *pos, const int pliesBack) {
    if (pos->ply >= pliesBack && pos->move[pos->ply - (pliesBack - 1)] && pos->move[pos->ply - pliesBack]) {
        return contCorrhist[pos->piece[pos->ply - (pliesBack - 1)]][getMoveTarget(pos->move[pos->ply - (pliesBack - 1)])]
                    [pos->piece[pos->ply - pliesBack]][getMoveTarget(pos->move[pos->ply - pliesBack])];
    }
    return 0;
}

void update_continuation_corrhist(board *pos, const int depth, const int diff) {
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    update_single_cont_corrhist_entry(pos, 2, scaledDiff, newWeight);
    update_single_cont_corrhist_entry(pos, 4, scaledDiff, newWeight);
}

void update_king_rook_pawn_corrhist(board *position, const int depth, const int diff) {
    U64 kingRookPawnKey = position->krpKey;

    int entry = krpCorrhist[position->side][kingRookPawnKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 4 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    krpCorrhist[position->side][kingRookPawnKey % CORRHIST_SIZE] = entry;
}

int adjust_eval_with_corrhist(board *pos, int rawEval) {   
    rawEval = rawEval * (300 - pos->fifty) / 300;
    
    U64 pawnKey = pos->pawnKey;
    U64 minorKey = pos->minorKey;
    U64 majorKey = pos->majorKey;
    U64 krpKey = pos->krpKey;

    int pawnEntry = PAWN_CORRECTION_HISTORY[pos->side][pawnKey % CORRHIST_SIZE];
    int minorEntry = MINOR_CORRECTION_HISTORY[pos->side][minorKey % CORRHIST_SIZE];
    int majorEntry = MAJOR_CORRECTION_HISTORY[pos->side][majorKey % CORRHIST_SIZE];
    int krpEntry = krpCorrhist[pos->side][krpKey % CORRHIST_SIZE];

    U64 whiteNPKey = pos->whiteNonPawnKey;
    int whiteNPEntry = NON_PAWN_CORRECTION_HISTORY[white][pos->side][whiteNPKey % CORRHIST_SIZE];

    U64 blackNPKey = pos->blackNonPawnKey;
    int blackNPEntry = NON_PAWN_CORRECTION_HISTORY[black][pos->side][blackNPKey % CORRHIST_SIZE];

    int contCorrhistEntry = adjust_single_cont_corrhist_entry(pos, 2);        

    int mateFound = mateValue - maxPly;

    int adjust = pawnEntry + minorEntry + majorEntry + whiteNPEntry + blackNPEntry + contCorrhistEntry + krpEntry;

    return clamp(rawEval + adjust / CORRHIST_GRAIN, -mateFound + 1, mateFound - 1);
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
