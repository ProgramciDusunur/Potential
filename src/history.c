//
// Created by erena on 13.09.2024.
//

#include "history.h"

// quietHistory[fromSquare][toSquare]
int quietHistory[64][64][2][2];
// rootHistory[side to move][fromSquare][toSquare]
int rootHistory[2][64][64];

int scaledBonus(int score, int bonus, int gravity) {
    return bonus - score * myAbs(bonus) / gravity;
}

void updateRootHistory(board *position, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
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

void updateQuietMoveHistory(int bestMove, int depth, moves *badQuiets, uint64_t threats) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
    bool threatFrom = getBit(threats, from);
    bool threatTo = getBit(threats, to);
    int score = quietHistory[from][to][threatFrom][threatTo];

    quietHistory[from][to][threatFrom][threatTo] +=
            scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {
        int badQuietFrom = getMoveSource(badQuiets->moves[index]);
        int badQuietTo = getMoveTarget(badQuiets->moves[index]);
        bool badThreatFrom = getBit(threats, badQuietFrom);
        bool badThreatTo = getBit(threats, badQuietTo);

        int badQuietScore =
                quietHistory[badQuietFrom][badQuietTo][badThreatFrom][badThreatTo];

        if (badQuiets->moves[index] == bestMove) continue;

        quietHistory[badQuietFrom][badQuietTo][badQuietFrom][badQuietTo] +=
                scaledBonus(badQuietScore, -bonus, maxQuietHistory);
    }
}

void clearQuietHistory(void) {
    memset(quietHistory, 0, sizeof(quietHistory));;
}
