//
// Created by erena on 13.09.2024.
//

#ifndef POTENTIAL_MOVE_H
#define POTENTIAL_MOVE_H

#pragma once

#include "structs.h"
#include "bit_manipulation.h"
#include "mask.h"
#include "magic.h"
#include "table.h"

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

void copyBoard(board *p, struct copyposition *cp);
void takeBack(board *p, struct copyposition *cp);

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
void addMove(moves *moveList, int move);
U64 getBishopAttacks(int square, U64 occupancy);
U64 getRookAttacks(int square, U64 occupancy);
U64 getQueenAttacks(int square, U64 occupancy);
U64 getKingAttacks(int square);
U64 getPawnAttacks(uint8_t side, int square);
U64 getKnightAttacks(int square);
int isSquareAttacked(int square, int whichSide, board* position);
int makeMove(int move, int moveFlag, board* position);
void moveGenerator(moves *moveList, board* position);
void noisyGenerator(moves *moveList, board* position);
void initSlidersAttacks(int bishop);
void initLeaperAttacks();
void addMoveToHistoryList(moves* list, int move);

#endif //POTENTIAL_MOVE_H
