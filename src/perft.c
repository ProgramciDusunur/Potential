//
// Created by erena on 13.09.2024.
//

#include "perft.h"

U64 perftNodes = 0;
U64 variant = 0;


void perftChild(int depth, board* position) {
    if (depth == 0) {
        perftNodes++;
        variant++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }

        // call perft driver recursively
        perftChild(depth - 1, position);
        takeBack(position, &copyPosition);
    }
}


void perftRoot(int depth, board* position) {
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perftChild(depth - 1, position);
        printf("%s%s%c %llu \n", squareToCoordinates[getMoveSource(moveList->moves[moveCount])],
               squareToCoordinates[getMoveTarget(moveList->moves[moveCount])],
               promotedPieces[getMovePromoted(moveList->moves[moveCount])], variant);
        variant = 0;
        takeBack(position, &copyPosition);
    }
    printf("\n");

}

void perft(int depth, board* position) {
    if (depth == 0) {
        perftNodes++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perft(depth - 1, position);
        takeBack(position, &copyPosition);
    }
}
