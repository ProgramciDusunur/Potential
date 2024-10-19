//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int getSingleContinuationHistoryScore(const SearchStack *ss, SearchData *sd, const int move, const int offSet) {
    const int previousMove = (ss - offSet)->move;
    return previousMove ? sd->continuationHistory[getMovePiece(previousMove)][getMoveTarget(previousMove)][getMovePiece(move)][getMoveTarget(move)] : 0;
}

// Returns the history score of a move
int getContinuationHistoryScore(const SearchStack *ss, SearchData *sd, const int move) {
    return getSingleContinuationHistoryScore(ss, sd, move, 1);
    //getSingleContinuationHistoryScore(ss, move, 2);
    //+ GetSingleCHScore(sd, ss, move, 4);
}

void updateSingleContinuationHistoryScore(const board *position, SearchStack *ss, SearchData *sd, const int move, const int bonus, const int offSet) {
    if (position->ply >= offSet) {
        const int previousMove = (ss - offSet)->move;
        const int scaledBonus = bonus - getSingleContinuationHistoryScore(ss, sd, move, offSet) * abs(bonus) / 16384;
        sd->continuationHistory[getMovePiece(previousMove)][getMoveTarget(previousMove)][getMovePiece(move)][getMoveTarget(move)] += scaledBonus;
    }
}

void updateContinuationHistoryScore(board *position, SearchStack *ss, SearchData *sd, const int move, const int bonus) {
    const int scaledBonus = bonus - getContinuationHistoryScore(ss, sd, move) * abs(bonus) / 8192;
    updateSingleContinuationHistoryScore(position, ss, sd, move, scaledBonus, 1);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 2);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 4);
}





int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxHistory;
}

void updateHistory(board *position, SearchStack *ss, SearchData *sd, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = minimum(16 * depth * depth + 32 * depth + 16, 1200);
    int score = historyMoves[from][to];

    historyMoves[from][to] += scaledBonus(score, bonus);
    updateContinuationHistoryScore(position, ss, sd, bestMove, bonus);

    for (int index = 0; index < badQuiets->count; index++) {
        int move = badQuiets->moves[index];
        int badQuietFrom = getMoveSource(move);
        int badQuietTo = getMoveTarget(move);

        int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
        updateContinuationHistoryScore(position, ss, sd, move, -bonus);
    }
}

void clearHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            historyMoves[i][j] = 0;
        }
    }
}

void clearContinuationHistory(SearchData *sd) {
    memset(sd->continuationHistory, 0, sizeof(sd->continuationHistory));

}
