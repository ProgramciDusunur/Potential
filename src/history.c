//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateQuietMoveHistory(int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    int clampedBonus =
            clamp(bonus, -QUIET_HISTORY_BONUS_MIN, QUIET_HISTORY_BONUS_MAX);
    int clampedMalus =
            clamp(bonus, -QUIET_HISTORY_MALUS_MIN, QUIET_HISTORY_MALUS_MAX);

    int adjust = bestMove ? clampedBonus : clampedMalus;
    int score = historyMoves[from][to];

    historyMoves[from][to] += scaledBonus(score, adjust, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, adjust, maxQuietHistory);
    }
}

void clearHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            historyMoves[i][j] = 0;
        }
    }
}
