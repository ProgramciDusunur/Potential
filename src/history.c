//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxHistory;
}

void updateHistory(int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = depth * depth;
    int score = historyMoves[from][to];

    historyMoves[from][to] += scaledBonus(score, bonus);

     for (int index = 0; index < badQuiets->count; index++) {
         int badQuietFrom = getMoveSource(badQuiets->moves[index]);
         int badQuietTo = getMoveTarget(badQuiets->moves[index]);

         int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

         if (badQuiets->moves[index] == bestMove) continue;

         historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
     }
}

void clearHistory() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            historyMoves[i][j] = 0;
        }
    }
}
