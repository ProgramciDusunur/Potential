//
// Created by erena on 18.07.2024.
//

#include "bit_manipulation.h"


// random piece keys [piece][square]
extern U64 pieceKeys[12][64];
// random enpassant keys [square]
extern U64 enpassantKeys[64];
// random castling keys
extern U64 castleKeys[16];

extern void initRandomKeys();
