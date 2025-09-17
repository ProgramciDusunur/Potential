//
// Created by erena on 13.09.2024.
//

#include "history.h"
#include "evaluation.h"
#include "utils.h"

// quietHistory[side to move][fromSquare][toSquare][threatSource][threatTarget]
int16_t quietHistory[2][64][64][2][2];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq][threatSource][threatTarget]
int16_t continuationHistory[12][64][12][64][2][2];
// continuationCorrectionHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
int16_t contCorrhist[12][64][12][64];
// pawnHistory [pawnKey][piece][to]
int16_t pawnHistory[2048][12][64];
// captureHistory[piece][toSquare][capturedPiece]
int16_t captureHistory[12][64][13];


int getHistoryBonus(int depth) {
    return myMIN(10 + 200 * depth, 4096);
}

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateQuietMoveHistory(int bestMove, int side, int depth, moves *badQuiets, board *pos) {
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

void updatePawnHistory(board *pos, int bestMove, int depth, moves *badQuiets) {
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

void updateCaptureHistory(board *position, int bestMove, int depth) {
    int piece = getMovePiece(bestMove);
    int to = getMoveTarget(bestMove);
    int capturedPiece = position->mailbox[getMoveTarget(bestMove)];

    int bonus = getHistoryBonus(depth);
    int score = captureHistory[piece][to][capturedPiece];

    captureHistory[piece][to][capturedPiece] += scaledBonus(score, bonus, maxCaptureHistory);
}

void updateCaptureHistoryMalus(board *position, int depth, moves *noisyMoves, int bestMove) {
    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyPiece = getMovePiece(noisyMoves->moves[index]);
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);
        int noisyCapturedPiece = position->mailbox[getMoveTarget(noisyMoves->moves[index])];

        if (noisyMoves->moves[index] == bestMove) continue;

        int noisyMoveScore = captureHistory[noisyPiece][noisyTo][noisyCapturedPiece];        

        captureHistory[noisyPiece][noisyTo][noisyCapturedPiece] += scaledBonus(noisyMoveScore, -getHistoryBonus(depth), maxCaptureHistory);
    }
}

int getContinuationHistoryScore(board *pos, int offSet, int move) {
    const int ply = pos->ply - offSet;
    return ply >= 0 ? continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)][is_square_threatened(pos, getMoveSource(move))][is_square_threatened(pos, getMoveTarget(move))] : 0;
}

void updateSingleCHScore(board *pos, int move, const int offSet, const int bonus) {
    const int ply = pos->ply - offSet;
    if (ply >= 0) {
        const int scaledBonus = bonus - getContinuationHistoryScore(pos, offSet, move) * abs(bonus) / maxQuietHistory;
        continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)][is_square_threatened(pos, getMoveSource(move))][is_square_threatened(pos, getMoveTarget(move))] += scaledBonus;
    }
}

void updateAllCH(board *pos, int move, int bonus) {
    updateSingleCHScore(pos, move, 1, bonus);
    updateSingleCHScore(pos, move, 2, bonus);
    updateSingleCHScore(pos, move, 4, bonus);
}

void updateContinuationHistory(board *pos, int bestMove, int depth, moves *badQuiets) {
    int bonus = getHistoryBonus(depth);

    updateAllCH(pos, bestMove, bonus);

    for (int index = 0; index < badQuiets->count; index++) {
        if (badQuiets->moves[index] == bestMove) continue;
        updateAllCH(pos, badQuiets->moves[index], -bonus);
    }
}

void clearQuietHistory(void) {
    memset(quietHistory, 0, sizeof(quietHistory));
}
