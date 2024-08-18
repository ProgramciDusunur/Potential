//
// Created by erena on 29.05.2024.
//

#include "board.h"
#include "move.h"
#include <stdio.h>

static inline void printAttackedSquares(int whichSide, board* position) {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (!file) {
                printf("  %d  ", 8 - rank);
            }
            printf("%d ", isSquareAttacked(square, whichSide, position) ? 1 : 0);

        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n\n");
}
// print move
static inline void printMove(int move) {
    if (getMovePromoted(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromoted(move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }
}


