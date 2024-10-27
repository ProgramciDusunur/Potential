//
// Created by erena on 13.09.2024.
//

#include "history.h"

int historyMoves[64][64];

int getCaptureHistoryScore(const SearchData *sd, const int move) {
    int capturedPiece = getMoveEnpassant(move) ? P : getMovePiece(move);
    return sd->captureHistory[getPieceTo(move)][capturedPiece];
}

void updateCaptureHistoryScore(SearchData *sd, const int move, int bonus) {
    const int scaledBonus = bonus - getCaptureHistoryScore(sd, move) * myAbs(bonus) / captureHistMax;
    int capturedPiece = getMoveEnpassant(move) ? P : getMovePiece(move);
    sd->captureHistory[getPieceTo(move)][capturedPiece] += scaledBonus;
}

int scaledBonus(int score, int bonus) {
    return bonus - score * myAbs(bonus) / maxHistory;
}

void updateHistory(SearchData *sd, int bestMove, int depth, moves *badQuiets) {
    int from = getMoveSource(bestMove);
    int to = getMoveTarget(bestMove);
    int bonus = 16 * depth * depth + 32 * depth + 16;
    if (!getMoveCapture(bestMove)) {

        int score = historyMoves[from][to];

        historyMoves[from][to] += scaledBonus(score, bonus);

        for (int index = 0; index < badQuiets->count; index++) {
            int badQuietFrom = getMoveSource(badQuiets->moves[index]);
            int badQuietTo = getMoveTarget(badQuiets->moves[index]);

            int badQuietScore = historyMoves[badQuietFrom][badQuietTo];

            if (badQuiets->moves[index] == bestMove) continue;

            historyMoves[badQuietFrom][badQuietTo] += scaledBonus(badQuietScore, -bonus);
        }
    } else {
        updateCaptureHistoryScore(sd, bestMove, bonus);
    }
    /*for (int index = 0; index < badCaptures->count; index++) {
        if (badCaptures->moves[index] == bestMove) continue;
        updateCaptureHistoryScore(sd, badQuiets->moves[index], -bonus);
    }*/


}

void clearHistory(void) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            historyMoves[i][j] = 0;
        }
    }
}

void clearCaptureHistory(SearchData *sd) {
    memset(sd->captureHistory, 0, sizeof(sd->captureHistory));
}
