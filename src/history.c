//
// Created by erena on 13.09.2024.
//

#include "history.h"

// [square][square]
int quietMoveHistory[64][64];
// [side][square][square]
int captureMoveHistory[2][64][64];
// [piece][move][piece][move]
int continuationHistory[12][64][12][64];

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateQuietHistory(board *position, SearchStack *ss, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int score = quietMoveHistory[from][to];

    quietMoveHistory[from][to] += scaledBonus(score, bonus, maxQuietHistory);
    updateContinuationHistoryScore(position, ss, bestMove, bonus);

    for (int index = 0; index < badQuiets->count; index++) {
        int move = badQuiets->moves[index];
        int badQuietFrom = getMoveSource(move);
        int badQuietTo = getMoveTarget(move);

        int badQuietScore = quietMoveHistory[badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        quietMoveHistory[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus, maxQuietHistory);
        updateContinuationHistoryScore(position, ss, move, -bonus);
    }
}

void updateCaptureHistory(board *position, int bestMove, int depth, moves *noisyMoves) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int score = captureMoveHistory[position->side][from][to];

    captureMoveHistory[position->side][from][to] += scaledBonus(score, bonus, maxCaptureHistory);

    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyFrom = getMoveSource(noisyMoves->moves[index]);
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);

        int noisyMoveScore = captureMoveHistory[position->side][noisyFrom][noisyTo];

        if (noisyMoves->moves[index] == bestMove) continue;

        captureMoveHistory[position->side][noisyFrom][noisyTo] += scaledBonus(noisyMoveScore, -bonus, maxCaptureHistory);
    }
}

int getSingleContinuationHistoryScore(const SearchStack *ss, const int move, int offSet) {
    return continuationHistory[getMovePiece((ss - offSet)->piece)][getMoveTarget((ss - offSet)->move)][getMovePiece(move)][getMoveTarget(move)];
}

// Returns the history score of a move
int getContinuationHistoryScore(const SearchStack *ss, const int move) {
    return getSingleContinuationHistoryScore(ss, move, 1);
    // + getSingleContinuationHistoryScore(ss, move, 2);
}

void updateSingleContinuationHistoryScore(const board *position, SearchStack *ss, const int move, const int bonus, const int offSet) {
    if (position->ply >= offSet) {
        const int scaledBonus = bonus - getSingleContinuationHistoryScore(ss, move, offSet) * abs(bonus) / maxQuietHistory;
        continuationHistory[getMoveTarget((ss - offSet)->piece)][getMoveTarget((ss - offSet)->move)][getMovePiece(move)][getMoveTarget(move)] += scaledBonus;
    }
}

void updateContinuationHistoryScore(board *position, SearchStack *ss, const int move, const int bonus) {
    const int scaledBonus = bonus - getContinuationHistoryScore(ss, move) * abs(bonus) / 8192;
    updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 1);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 2);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 4);
}





void clearQuietHistory(void) {
    memset(quietMoveHistory, 0, sizeof(quietMoveHistory));
}

void clearCaptureHistory(void) {
    memset(captureMoveHistory, 0, sizeof(captureMoveHistory));
}

void clearContinuationHistory(void) {
    memset(continuationHistory, 0, sizeof(continuationHistory));
}
