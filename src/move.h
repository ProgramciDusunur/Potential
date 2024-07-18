//
// Created by erena on 29.05.2024.
//
#pragma once

#include "board.h"


// move list structure
typedef struct {
    // moves
    int moves[256];

    // move count
    int count;
} moves;

struct copyposition {
    U64 bitboardsCopy[12];
    U64 occupanciesCopy[3];
    U64 hashKeyCopy;
    int sideCopy;
    int enpassantCopy;
    int castleCopy;
};

// encode move
#define encodeMove(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

// extract source square
#define getMoveSource(move) (move & 0x3f)
// extract target square
#define getMoveTarget(move) ((move & 0xfc0) >> 6)
// extract piece
#define getMovePiece(move) ((move & 0xf000) >> 12)
// extract promoted piece
#define getMovePromoted(move) ((move & 0xf0000) >> 16)
// extract double pawn push
#define getMoveCapture(move) (move & 0x100000)
// extract double pawn push flag
#define getMoveDouble(move) (move & 0x200000)
// extract enpassant flag
#define getMoveEnpassant(move) (move & 0x400000)
// extract castling flag
#define getMoveCastling(move) (move & 0x800000)


// Pawn attack masks pawnAttacks[side][square]
extern U64 pawnAttacks[2][64];

// Knight attack masks knightAttacks[square]
extern U64 knightAttacks[64];

// King attack masks kingAttacks[square]
extern U64 kingAttacks[64];

// Bishop attack table [square][occupancies]
extern U64 bishopAttacks[64][512];

// Rook attack table [square][occupancies]
extern U64 rookAttacks[64][4096];


void copyBoard(board *p, struct copyposition *cp);
void takeBack(board *p, struct copyposition *cp);
void addMoveToHistoryList(moves* list, int move);
void initSlidersAttacks(int bishop);
void initLeaperAttacks();
int isSquareAttacked(int square, int whichSide, board* position);
static inline int makeMove(int move, int moveFlag, board* position);
static inline void moveGenerator(moves *moveList, board* position);