//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int getSingleContinuationHistoryScore(const board *position, const SearchStack *ss, const int move, const int offSet) {
    const int previousMove = (ss)->move[position->ply - offSet];
    return previousMove ? ss->continuationHistory[getMoveTarget(previousMove)][getMoveTarget(move)] : 0;
}

// Returns the history score of a move
int getContinuationHistoryScore(const board *position, const SearchStack *ss, const int move) {
    return getSingleContinuationHistoryScore(position, ss, move, 1) +
    getSingleContinuationHistoryScore(position, ss, move, 2);
    //+ GetSingleCHScore(sd, ss, move, 4);
}

void updateSingleContinuationHistoryScore(const board *position, SearchStack *ss, const int move, const int bonus, const int offSet) {
    if (position->ply >= offSet) {
        const int previousMove = (ss)->move[position->ply - offSet];
        const int scaledBonus = bonus - getSingleContinuationHistoryScore(position, ss, move, offSet) * abs(bonus) / 32768;
        ss->continuationHistory[getMoveTarget(previousMove)][getMoveTarget(move)] += scaledBonus;
    }
}

void updateContinuationHistoryScore(board *position, SearchStack *ss, const int move, const int bonus) {
    const int scaledBonus = bonus - getContinuationHistoryScore(position, ss, move) * abs(bonus) / 16384;
    updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 1);
    updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 2);
    //updateSingleContinuationHistoryScore(position, ss, move, scaledBonus, 4);
}





int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxHistory;
}

void updateHistory(board *position, SearchStack *ss, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int score = historyMoves[from][to];

    historyMoves[from][to] += scaledBonus(score, bonus);
    updateContinuationHistoryScore(position, ss, bestMove, bonus);

     for (int index = 0; index < badQuiets->count; index++) {
         int move = badQuiets->moves[index];
         int badQuietFrom = getMoveSource(move);
         int badQuietTo = getMoveTarget(move);

         int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

         if (badQuiets->moves[index] == bestMove) continue;

         historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
         updateContinuationHistoryScore(position, ss, move, -bonus);
     }
}

void clearHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            historyMoves[i][j] = 0;
        }
    }
}

void clearContinuationHistory(SearchStack *ss) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            ss->continuationHistory[i][j] = 0;
        }
    }
}
