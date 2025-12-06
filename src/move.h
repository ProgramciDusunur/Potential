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
#include <stdbool.h>
#include <stdint.h>

enum {
    mf_normal = 0x0000,
    mf_double = 0x1000,
    mf_castling = 0x2000,
    mf_promo_n = 0x4000,
    mf_promo_b = 0x5000,
    mf_promo_r = 0x6000,
    mf_promo_q = 0x7000,
    mf_capture = 0x8000,
    mf_enpassant = 0x9000,
    mf_cap_promo_n = 0xC000,
    mf_cap_promo_b = 0xD000,
    mf_cap_promo_r = 0xE000,
    mf_cap_promo_q = 0xF000,
};

// encode move
#define encodeMove(source, target, mf) (source) | (target << 6) | (mf)

// extract source square
#define getMoveSource(move) (move & 0x3f)
// extract target square
#define getMoveTarget(move) ((move & 0xfc0) >> 6)
// extract move is promote
#define getMovePromote(move) (move & 0x4000)
// extract promoted piece
#define getMovePromotedPiece(stm, move) (((move & 0x3000) >> 12) + (stm == white ? N : n))
// extract double pawn push
#define getMoveCapture(move) (move & 0x8000)
// extract double pawn push flag
#define getMoveDouble(move) ((move & 0xF000) == mf_double)
// extract enpassant flag
#define getMoveEnpassant(move) ((move & 0xF000) == mf_enpassant)
// extract castling flag
#define getMoveCastling(move) ((move & 0xF000) == mf_castling)

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
void addMove(moves *moveList, uint16_t move);
bool isTactical(uint16_t move);
U64 getBishopAttacks(int square, U64 occupancy);
U64 getRookAttacks(int square, U64 occupancy);
U64 getQueenAttacks(int square, U64 occupancy);
U64 getKingAttacks(int square);
U64 getPawnAttacks(uint8_t side, int square);
U64 getKnightAttacks(int square);
int isSquareAttacked(int square, int whichSide, board* position);
int makeMove(uint16_t move, int moveFlag, board* position);
void moveGenerator(moves *moveList, board* position);
void noisyGenerator(moves *moveList, board* position);
void initSlidersAttacks(int bishop);
void initLeaperAttacks();
void addMoveToHistoryList(moves* list, uint16_t move);
U64 pawn_threats(U64 pawnBitboard, int side);
U64 knight_threats (U64 knightBB);

#endif //POTENTIAL_MOVE_H
