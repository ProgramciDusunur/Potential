//
// Created by erena on 13.09.2024.
//

#include "history.h"

// quietHistory[fromSquare][toSquare]
int quietHistory[64][64];
// rootHistory[side to move][fromSquare][toSquare]
int rootHistory[2][64][64];

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateRootHistory(board *position, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = myMIN(16 * depth * depth + 32 * depth + 16, 8192);
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

void updateQuietMoveHistory(int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = myMIN(16 * depth * depth + 32 * depth + 16, 1024);
    int score = quietHistory[from][to];

    quietHistory[from][to] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);

        int badQuietScore = quietHistory[badQuietFrom][badQuietTo];

        if (badQuiets->moves[index] == bestMove) continue;

        quietHistory[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus, maxQuietHistory);
    }
}


void clearQuietHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            quietHistory[i][j] = 0;
        }
    }
}
