//
// Created by erena on 13.09.2024.
//

#include "history.h"
// [square][square]
int quietMoveHistory[64][64];
// [side][square][square]
int captureMoveHistory[2][64][64];

int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxQuietHistory;
}

void updateQuietHistory(int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int score = quietMoveHistory[from][to];

    quietMoveHistory[from][to] += scaledBonus(score, bonus);

     for (int index = 0; index < badQuiets->count; index++) {
         int badQuietFrom = getMoveSource(badQuiets->moves[index]);
         int badQuietTo = getMoveTarget(badQuiets->moves[index]);

         int badQuietScore = quietMoveHistory[badQuietFrom][badQuietTo];

         if (badQuiets->moves[index] == bestMove) continue;

         quietMoveHistory[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
     }
}

void updateCaptureHistory(board *position, int bestMove, int depth, moves *noisyMoves) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int score = captureMoveHistory[position->side][from][to];

    captureMoveHistory[position->side][from][to] += scaledBonus(score, bonus);

    for (int index = 0; index < noisyMoves->count; index++) {
        int noisyFrom = getMoveSource(noisyMoves->moves[index]);
        int noisyTo = getMoveTarget(noisyMoves->moves[index]);

        int noisyMoveScore = quietMoveHistory[noisyFrom][noisyTo];

        if (noisyMoves->moves[index] == bestMove) continue;

        captureMoveHistory[position->side][from][to] += scaledBonus(noisyMoveScore, -bonus);
    }
}

void clearHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            quietMoveHistory[i][j] = 0;
        }
    }
}
