//
// Created by erena on 13.09.2024.
//

#include "board_constants.h"


// random piece keys [piece][square]
U64 pieceKeys[12][64];
// random enpassant keys [square]
U64 enpassantKeys[64];
// random castling keys
U64 castleKeys[16];

int8_t enPassantSquares[] = {
        8, -8
};


char *squareToCoordinates[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "no"
};

char asciiPieces[12] = "PNBRQKpnbrqk";

char promotedPieces[] = {
        [Q] = 'q',
        [R] = 'r',
        [B] = 'b',
        [N] = 'n',
        [q] = 'q',
        [r] = 'r',
        [b] = 'b',
        [n] = 'n'
};

char *unicodePieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔",
                           "♟", "♞", "♝", "♜", "♛", "♚"};

int charPieces[] = {
        ['P'] = P,
        ['N'] = N,
        ['B'] = B,
        ['R'] = R,
        ['Q'] = Q,
        ['K'] = K,
        ['p'] = p,
        ['n'] = n,
        ['b'] = b,
        ['r'] = r,
        ['q'] = q,
        ['k'] = k
};

int mvvLva[12][12] = {
        {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
        {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
        {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
        {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
        {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
        {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600},

        {105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
        {104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
        {103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
        {102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
        {101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
        {100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}
};

int minorPieces[6] = {B, b, N, n, K, k};
int majorPieces[4] = {R, r, Q, q};
int whiteNonPawnPieces[5] = { R, N, B, Q, K };
int blackNonPawnPieces[5] = {r, n, b, q, k };


const U64 notHFile = 9187201950435737471ULL;
const U64 notAFile = 18374403900871474942ULL;
const U64 not8Rank = 18446744073709551360ULL;
const U64 not1Rank = 72057594037927935ULL;
const U64 not8RankAndABFile = 18229723555195321344ULL;
const U64 not1RankAndGHFile = 17802464409370431ULL;
const U64 not1And2RankHFile = 140185576636287ULL;
const U64 not8And7RankAFile = 18374403900871409664ULL;
const U64 not8And7RankHFile = 9187201950435704832ULL;
const U64 not1And2RanksAFile = 280371153272574ULL;
const U64 not1RankAndABFile = 71209857637481724ULL;
const U64 not8RankAndGHFile = 4557430888798830336ULL;
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 notAFileAndHRank = 35887507618889599ULL;
const U64 not8RankAndAFile = 18374403900871474688ULL;
const U64 not8RankAndHFile = 9187201950435737344ULL;
const U64 not1RankAndAFile = 71775015237779198ULL;
// masks for the outer king ring
const U64 outerKingNorthMask = 18446744073709486080ULL;
const U64 outerKingSouthMask = 281474976710655ULL;
const U64 outerKingSouthEastMask = 69540876599103ULL;
const U64 outerKingNorthWestMask = 18229723555195256832ULL;
const U64 outerKingNorthEastMask = 4557430888798814208ULL;
const U64 outerKingSouthWestMask = 278163506396412ULL;
const U64 outerKingWestMask = 18229723555195321596ULL;
const U64 outerKingEastMask = 4557430888798830399ULL;

int get_rank[64] = {
        7, 7, 7, 7, 7, 7, 7, 7,
        6, 6, 6, 6, 6, 6, 6, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0
};

