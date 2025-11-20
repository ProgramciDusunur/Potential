#include "generate_fen.h"

// Default FEN generation settings
const int how_many_ply = 8;

void default_fen_generation(board *pos) {
    moves moveList[1];
    moveGenerator(moveList, pos);

    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(pos, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, pos) == 0) {
            // skip to the next move
            continue;
        }
        
        takeBack(pos, &copyPosition);
    }
}