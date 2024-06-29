//
// Created by erena on 29.06.2024.
//

#include "history.h"

int historyBonus(int score, int bonus) {
    return bonus - score * abs(bonus) / maxHistory;
}


void updateHistory(int movePiece, int moveTo, int depth) {
    int piece = movePiece;
    int to = moveTo;
    int score = historyMoves[piece][to];
    historyMoves[piece][to] += historyBonus(score, depth * depth);
}

