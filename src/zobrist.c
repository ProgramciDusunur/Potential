//
// Created by erena on 18.07.2024.
//

#include "zobrist.h"




// random piece keys [piece][square]
U64 pieceKeys[12][64];
// random enpassant keys [square]
U64 enpassantKeys[64];
// random castling keys
U64 castleKeys[16];
