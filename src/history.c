//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int getSingleContinuationHistoryScore(const SearchStack *ss, const int move, const int offSet) {
    const int previousMove = (ss - offSet)->move;
    return previousMove ? (*((ss - offSet)->contHistEntry))[getMovePiece(move)][getMoveTarget(move)] : 0;
}

// Returns the history score of a move
int getContinuationHistoryScore(const SearchStack *ss, const int move) {
    return getSingleContinuationHistoryScore(ss, move, 1);
    //getSingleContinuationHistoryScore(ss, move, 2);
    //+ GetSingleCHScore(sd, ss, move, 4);
}

void updateSingleContinuationHistoryScore(SearchStack *ss, const int move, const int bonus, const int offSet) {
    if ((ss - offSet)->move) {
        const int scaledBonus = bonus - getSingleContinuationHistoryScore(ss, move, offSet) * abs(bonus) / 16384;
        (*((ss - offSet)->contHistEntry))[getMovePiece(move)][getMoveTarget(move)] += scaledBonus;
    }
}

void updateContinuationHistoryScore(SearchStack *ss, const int move, const int bonus) {
    const int scaledBonus = bonus - getContinuationHistoryScore(ss, move) * abs(bonus) / 8192;
    updateSingleContinuationHistoryScore(ss, move, scaledBonus, 1);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 2);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 4);
}





int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxHistory;
}

void updateHistory(SearchStack *ss, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = minimum(16 * depth * depth + 32 * depth + 16, 1200);
    int score = historyMoves[from][to];

    historyMoves[from][to] += scaledBonus(score, bonus);
    updateContinuationHistoryScore(ss, bestMove, bonus);

    for (int index = 0; index < badQuiets->count; index++) {
        int move = badQuiets->moves[index];
        int badQuietFrom = getMoveSource(move);
        int badQuietTo = getMoveTarget(move);

        int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
        updateContinuationHistoryScore(ss, move, -bonus);
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
