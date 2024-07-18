//
// Created by erena on 29.05.2024.
//
#include "fen.h"



// parse FEN string
void parseFEN(char *fen, board* position) {
    // reset board position (bitboards)
    memset(position->bitboards, 0ULL, sizeof(position->bitboards));
    // reset board occupancies (bitboards)
    memset(position->occupancies, 0ULL, sizeof(position->occupancies));

    // reset game state variables
    position->side = 0;
    position->enpassant = no_sq;
    position->castle = 0;

    // reset repetition index
    position->repetitionIndex = 0;

    // reset repetition table
    memset(position->repetitionTable, 0ULL, sizeof(position->repetitionTable));

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                int piece = charPieces[*fen];
                setBit(position->bitboards[piece], square);
                fen++;
            }
            if (*fen >= '0' && *fen <= '9') {
                int offset = *fen - '0';
                int piece = -1;
                for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                    if (getBit(position->bitboards[bbPiece], square)) {
                        piece = bbPiece;
                    }
                }
                if (piece == -1) {
                    file--;
                }
                file += offset;
                fen++;
            }
            if (*fen == '/') {
                fen++;
            }
        }
    }
    fen++;
    (*fen == 'w') ? (position->side = white) : (position->side = black);
    fen += 2;
    while (*fen != ' ') {
        switch (*fen) {
            case 'K':
                position->castle |= wk;
                break;
            case 'Q':
                position->castle |= wq;
                break;
            case 'k':
                position->castle |= bk;
                break;
            case 'q':
                position->castle |= bq;
                break;
            case '-':
                break;
        }
        fen++;
    }
    fen++;
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        position->enpassant = rank * 8 + file;
    } else {
        position->enpassant = no_sq;
    }
    for (int piece = P; piece <= K; piece++) {
        position->occupancies[white] |= position->bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++) {
        position->occupancies[black] |= position->bitboards[piece];
    }
    position->occupancies[both] |= position->occupancies[white];
    position->occupancies[both] |= position->occupancies[black];

    // init hash key
    position->hashKey = generateHashKey(position);
}


