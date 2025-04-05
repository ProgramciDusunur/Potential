//
// Created by erena on 13.09.2024.
//

#include "history.h"

#include "utils.h"

// quietHistory[side to move][fromSquare][toSquare]
int quietHistory[2][64][64];
// rootHistory[side to move][fromSquare][toSquare]
int rootHistory[2][64][64];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
int continuationHistory[12][64][12][64];

int getHistoryBonus(int depth) {
    return myMIN(16 * depth * depth + 32 * depth + 16, 4096);
}

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateRootHistory(board *position, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);

    int score = rootHistory[position->side][from][to];

    rootHistory[position->side][from][to] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = rootHistory[position->side][badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        rootHistory[position->side][badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus, maxQuietHistory);
    }
}

void updateQuietMoveHistory(int bestMove, int side, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = getHistoryBonus(depth);
    int score = quietHistory[side][from][to];

    quietHistory[side][from][to] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = quietHistory[side][badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        quietHistory[side][badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus, maxQuietHistory);
    }
}


int getContinuationHistoryScore(board *pos, int offSet, int move) {
    const int ply = pos->ply - offSet;
    return ply >= 0 ? continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)] : 0;
}

void updateSingleCHScore(board *pos, int move, const int offSet, const int bonus) {
    const int ply = pos->ply - offSet;
    if (ply >= 0) {
        const int scaledBonus = bonus - getContinuationHistoryScore(pos, offSet, move) * abs(bonus) / maxQuietHistory;
        continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)] += scaledBonus;
    }
}

void updateAllCH(board *pos, int move, int bonus) {
    updateSingleCHScore(pos, move, 1, bonus);
    updateSingleCHScore(pos, move, 2, bonus);
    updateSingleCHScore(pos, move, 4, bonus);
    updateSingleCHScore(pos, move, 6, bonus);
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
