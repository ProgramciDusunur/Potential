//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"

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
// kingRookPawn Correction History [side to move][key]
int16_t krpCorrhist[2][16384];

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

void adjust_single_capture_hist_entry(board *pos, uint16_t move, int piece, int bonus) {    
    int to = getMoveTarget(move);
    uint16_t capturedPiece = pos->mailbox[getMoveTarget(move)];

    captureHistory[piece][to][capturedPiece] += bonus;
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

void clearQuietHistory(void) {
    memset(quietHistory, 0, sizeof(quietHistory));
}
