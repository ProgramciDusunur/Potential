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

int clamp(const int d, const int min, const int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

double my_max_double(double x, double y) {
    return (x > y) ? x : y;
}

double my_min_double(double x, double y) {
    return (x < y) ? x : y;
}

double clamp_double(const double d, const double min, const double max) {
    const double t = d < min ? min : d;
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

    // fifty move rule counter
    printf("    Fifty move: %d\n\n", position->fifty);

    // print full move counter
    printf("    Full moves counter: %d\n\n", position->full_moves);
}

void printMailbox(const board *position) {
    printf("\n");
    for (int y = 0, coordinate = 8; y < 8; y++, coordinate--) {
        printf("%d  ", coordinate);
        for (int x = 0; x < 8; x++) {
            const int index = y * 8 + x;
            const int piece = position->mailbox[index];
            switch (piece) {
                case P:
                    printf("P ");
                break;
                case N:
                    printf("N ");
                break;
                case B:
                    printf("B ");
                break;
                case R:
                    printf("R ");
                break;
                case Q:
                    printf("Q ");
                break;
                case K:
                    printf("K ");
                break;

                case p:
                    printf("p ");
                break;
                case n:
                    printf("n ");
                break;
                case b:
                    printf("b ");
                break;
                case r:
                    printf("r ");
                break;
                case q:
                    printf("q ");
                break;
                case k:
                    printf("k ");
                break;
                default:
                    printf("- ");
                break;
            }

        }
        printf(" \n");
    }
    printf("\n   a b c d e f g h \n");


    printf("\n");

}

double clampDecimalValue(double d, const double min, const double max) {
    const double t = d < min ? min : d;
    return t > max ? max : t;
}

bool is_mate_score(int score) {
    return (score >= mateFound && score <= mateValue) || 
    (score <= -mateFound && score >= -mateValue);
}
