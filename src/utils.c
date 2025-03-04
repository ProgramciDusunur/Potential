//
// Created by erena on 2.02.2025.
//

#include "utils.h"




int myMAX(int x, int y) {
    return (x > y) ? x : y;
}

int myMIN(int x, int y) {
    return (x < y) ? x : y;
}

int clamp(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

void pBoard(board* position) {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (!file) {
                printf(" %d ", 8 - rank);
            }
            int piece = -1;
            for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                if (getBit(position->bitboards[bbPiece], square)) {
                    piece = bbPiece;
                }
            }
#ifdef WIN64
            printf(" %c", (piece == -1) ? '.' : asciiPieces[piece]);
#else
            printf(" %c", (piece == -1) ? '.' : asciiPieces[piece]);
#endif
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");

    printf("    Side:     %s\n", !position->side ? "white" : "black");

    printf("    Enpassant: %s\n", (position->enpassant != no_sq) ? squareToCoordinates[position->enpassant] : "  no");

    printf("    Castling:  %c%c%c%c\n\n", (position->castle & wk) ? 'K' : '-',
           (position->castle & wq) ? 'Q' : '-',
           (position->castle & bk) ? 'k' : '-',
           (position->castle & bq) ? 'q' : '-');

    // print hash key
    printf("    Hash key:  %llx\n\n", position->hashKey);
}
