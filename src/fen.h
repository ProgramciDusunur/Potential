//
// Created by erena on 29.05.2024.
//
#pragma once


#include "board.h"
#include "board_constants.h"
#include "board.h"
#include "bit_manipulation.h"
#include "table.h"
#include <string.h>


#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define emptyBoard "8/8/8/8/8/8/8/8 w - - 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40"

// parse FEN string
inline void parseFEN(char *fen, board* position) {
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
