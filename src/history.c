//
// Created by erena on 13.09.2024.
//

#include "history.h"

// quietHistory[side to move][fromSquare][toSquare]
int quietHistory[2][64][64];
// rootHistory[side to move][fromSquare][toSquare]
int rootHistory[2][64][64];
// continuationHistory[previousPiece][previousTargetSq][currentPiece][currentTargetSq]
int continuationHistory[12][64][12][64];

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

void updateQuietMoveHistory(int bestMove, int side, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);

    int bonus = 16 * depth * depth + 32 * depth + 16;
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
    int ply = pos->ply - offSet;
    return ply >= 0 ? continuationHistory[pos->piece[ply]][getMoveTarget(pos->move[ply])]
                              [pos->mailbox[getMoveSource(move)]][getMoveTarget(move)] : 0;
}


void updateContinuationHistory(board *pos, int bestMove, int depth, moves *badQuiets) {
    int prev_piece = pos->piece[pos->ply];
    int prev_target = getMoveTarget(pos->move[pos->ply]);
    int piece = pos->mailbox[getMoveSource(bestMove)];
    int target = getMoveTarget(bestMove);

    int score = continuationHistory[prev_piece][prev_target][piece][target];
    int bonus = 16 * depth * depth + 32 * depth + 16;

    continuationHistory[prev_piece][prev_target][piece][target] += scaledBonus(score, bonus, maxQuietHistory);

    for (int index = 0; index < badQuiets->count; index++) {

        if (badQuiets->moves[index] == bestMove) continue;


        int badQuietPiece = pos->mailbox[getMoveSource(badQuiets->moves[index])];
        int badQuietTarget = getMoveTarget(badQuiets->moves[index]);



        int badQuietScore = continuationHistory[prev_piece][prev_target][badQuietPiece][badQuietTarget];



        continuationHistory[prev_piece][prev_target][badQuietPiece][badQuietTarget] += scaledBonus(badQuietScore, -bonus, maxQuietHistory);
    }

}



void clearQuietHistory(void) {
    memset(quietHistory, 0, sizeof(quietHistory));
}
