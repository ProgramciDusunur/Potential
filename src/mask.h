//
// Created by erena on 29.05.2024.
//
#ifndef MASK_H
#define MASK_H

#pragma once
#include "board_constants.c"
#include "bit_manipulation.h"
#include "board.h"

// file masks [square]
extern U64 fileMasks[64];

// rank masks [square]
extern U64 rankMasks[64];

// isolated pawn masks [square]
extern U64 isolatedMasks[64];

// white passed pawn masks [square]
extern U64 whitePassedMasks[64];

// black passed pawn masks [square]
extern U64 blackPassedMasks[64];


// Rook attack masks rookMask[square]
extern U64 rookMask[64];
// BishopMask[square]
extern U64 bishopMask[64];

static inline U64 maskPawnAttacks(int isWhite, int square) {
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

    // set piece on board
    setBit(bitboard, square);

    // white pawns
    if (!isWhite) {
        // generate pawn attacks
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
        // black pawns
    } else {
        // generate pawn attacks
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    }
    // return attack map
    return attacks;
}

static inline U64 maskKnightAttacks(int square) {
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    setBit(bitboard, square);

    if (bitboard & not8RankAndABFile) { attacks |= bitboard >> 10; }

    if (bitboard & not1RankAndGHFile) { attacks |= bitboard << 10; }

    if (bitboard & not1And2RankHFile) { attacks |= bitboard << 17; }

    if (bitboard & not8And7RankAFile) { attacks |= bitboard >> 17; }

    if (bitboard & not8And7RankHFile) { attacks |= bitboard >> 15; }

    if (bitboard & not1And2RanksAFile) { attacks |= bitboard << 15; }

    if (bitboard & not1RankAndABFile) { attacks |= bitboard << 6; }

    if (bitboard & not8RankAndGHFile) { attacks |= bitboard >> 6; }

    return attacks;
}

static inline U64 maskKingAttacks(int square) {
    U64 attacks = 0ULL;

    U64 bitboard = 0ULL;

    setBit(bitboard, square);

    if (bitboard & not8Rank) { attacks |= bitboard >> 8; }

    if (bitboard & not1Rank) { attacks |= bitboard << 8; }

    if (bitboard & notAFileAndHRank) { attacks |= bitboard << 9; }

    if (bitboard & not8RankAndAFile) { attacks |= bitboard >> 9; }

    if (bitboard & not8RankAndHFile) { attacks |= bitboard >> 7; }

    if (bitboard & not1RankAndAFile) { attacks |= bitboard << 7; }

    if (bitboard & notAFile) { attacks |= bitboard >> 1; }

    if (bitboard & notHFile) { attacks |= bitboard << 1; }

    return attacks;
}

static inline U64 maskBishopAttacks(int square) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1, f0 = file + 1; r0 < 7 && f0 < 7; r0++, f0++) {
        attacks |= (1ULL << (r0 * 8 + f0));
    }
    for (int r1 = rank - 1, f1 = file - 1; r1 > 0 && f1 > 0; r1--, f1--) {
        attacks |= (1ULL << (r1 * 8 + f1));
    }
    for (int r2 = rank + 1, f2 = file - 1; r2 < 7 && f2 > 0; r2++, f2--) {
        attacks |= (1ULL << (r2 * 8 + f2));
    }
    for (int r3 = rank - 1, f3 = file + 1; r3 > 0 && f3 < 7; r3--, f3++) {
        attacks |= (1ULL << (r3 * 8 + f3));
    }
    return attacks;
}

static inline U64 bishopAttack(int square, U64 block) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1, f0 = file + 1; r0 < 8 && f0 < 8; r0++, f0++) {
        attacks |= (1ULL << (r0 * 8 + f0));
        if ((1ULL << (r0 * 8 + f0)) & block) { break; }
    }
    for (int r1 = rank - 1, f1 = file - 1; r1 > -1 && f1 > -1; r1--, f1--) {
        attacks |= (1ULL << (r1 * 8 + f1));
        if ((1ULL << (r1 * 8 + f1)) & block) { break; }
    }
    for (int r2 = rank + 1, f2 = file - 1; r2 < 8 && f2 > -1; r2++, f2--) {
        attacks |= (1ULL << (r2 * 8 + f2));
        if ((1ULL << (r2 * 8 + f2)) & block) { break; }
    }
    for (int r3 = rank - 1, f3 = file + 1; r3 > -1 && f3 < 8; r3--, f3++) {
        attacks |= (1ULL << (r3 * 8 + f3));
        if ((1ULL << (r3 * 8 + f3)) & block) { break; }
    }
    return attacks;
}

static inline U64 maskRookAttacks(int square) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1; r0 < 7; r0++) {
        attacks |= 1ULL << (r0 * 8 + file);
    }
    for (int f0 = file + 1; f0 < 7; f0++) {
        attacks |= 1ULL << (rank * 8 + f0);
    }
    for (int r1 = rank - 1; r1 > 0; r1--) {
        attacks |= 1ULL << (r1 * 8 + file);
    }
    for (int f1 = file - 1; f1 > 0; f1--) {
        attacks |= 1ULL << (rank * 8 + f1);
    }
    return attacks;
}

static inline U64 rookAttack(int square, U64 block) {
    U64 attacks = 0ULL;

    int rank = square / 8, file = square % 8;

    for (int r0 = rank + 1; r0 < 8; r0++) {
        attacks |= 1ULL << (r0 * 8 + file);
        if ((1ULL << (r0 * 8 + file)) & block) { break; }
    }
    for (int f0 = file + 1; f0 < 8; f0++) {
        attacks |= 1ULL << (rank * 8 + f0);
        if ((1ULL << (rank * 8 + f0)) & block) { break; }
    }
    for (int r1 = rank - 1; r1 > -1; r1--) {
        attacks |= 1ULL << (r1 * 8 + file);
        if ((1ULL << (r1 * 8 + file)) & block) { break; }
    }
    for (int f1 = file - 1; f1 > -1; f1--) {
        attacks |= 1ULL << (rank * 8 + f1);
        if ((1ULL << (rank * 8 + f1)) & block) { break; }
    }
    return attacks;
}

static inline U64 get_attackers(const board *pos, int square, int side) {
    U64 attacks = 0ULL;

    attacks |= (maskPawnAttacks(side, square) & pos->bitboards[P]) | (maskPawnAttacks(!side, square) & pos->bitboards[p]);
    attacks |= (maskKnightAttacks(square) & (pos->bitboards[N] | pos->bitboards[n]));
    attacks |= (maskKingAttacks(square) & (pos->bitboards[K] | pos->bitboards[k]));
    attacks |= (bishopAttack(square,pos->occupancies[both]) & (pos->bitboards[B] | pos->bitboards[b] | pos->bitboards[Q] | pos->bitboards[q]));
    attacks |= (rookAttack(square, pos->occupancies[both]) & (pos->bitboards[R] | pos->bitboards[r] | pos->bitboards[Q] | pos->bitboards[q]));
    return attacks;
}

static inline U64 setFileRankMask(int file_number, int rank_number) {
    // file or rank mask
    U64 mask = 0ULL;

    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            if (file_number != -1)
            {
                // on file match
                if (file == file_number)
                    // set bit on mask
                    mask |= setBit(mask, square);
            }

            else if (rank_number != -1)
            {
                // on rank match
                if (rank == rank_number)
                    // set bit on mask
                    mask |= setBit(mask, square);
            }
        }
    }

    // return mask
    return mask;
}


static inline void initEvaluationMasks() {
    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            // init file mask for a current square
            fileMasks[square] |= setFileRankMask(file, -1);
        }
    }

    /******** Init rank masks ********/

    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            // init rank mask for a current square
            rankMasks[square] |= setFileRankMask(-1, rank);
        }
    }

    /******** Init isolated masks ********/

    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            // init isolated pawns masks for a current square
            isolatedMasks[square] |= setFileRankMask(file - 1, -1);
            isolatedMasks[square] |= setFileRankMask(file + 1, -1);
        }
    }

    /******** White passed masks ********/

    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            // init white passed pawns mask for a current square
            whitePassedMasks[square] |= setFileRankMask(file - 1, -1);
            whitePassedMasks[square] |= setFileRankMask(file, -1);
            whitePassedMasks[square] |= setFileRankMask(file + 1, -1);

            // loop over redudant ranks
            for (int i = 0; i < (8 - rank); i++)
                // reset redudant bits
                whitePassedMasks[square] &= ~rankMasks[(7 - i) * 8 + file];
        }
    }

    /******** Black passed masks ********/

    // loop over ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over files
        for (int file = 0; file < 8; file++)
        {
            // init square
            int square = rank * 8 + file;

            // init black passed pawns mask for a current square
            blackPassedMasks[square] |= setFileRankMask(file - 1, -1);
            blackPassedMasks[square] |= setFileRankMask(file, -1);
            blackPassedMasks[square] |= setFileRankMask(file + 1, -1);

            // loop over redudant ranks
            for (int i = 0; i < rank + 1; i++)
                // reset redudant bits
                blackPassedMasks[square] &= ~rankMasks[i * 8 + file];
        }
    }
}

#endif // MASK_H
