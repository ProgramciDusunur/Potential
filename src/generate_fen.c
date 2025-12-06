#include "generate_fen.h"

// Default FEN generation settings
const int how_many_ply = 8;

void extract_and_print_fen(board *pos) {
    char fen_string[100];    

    // Extract board
    for (int square = 0; square < 64; square++) {

        // If the target square has a piece
        if (pos->mailbox[square] != NO_PIECE) {
            fen_string[square] = asciiPieces[pos->mailbox[square]];
            continue;
        }

        if (square % 8 == 1) {
            fen_string[square] = '/';
            continue;;
        }
    }

    printf("\nFEN: %s\n", fen_string);
}

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

        pBoard(pos);
        
        takeBack(pos, &copyPosition);
    }
}