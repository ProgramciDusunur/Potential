//
// Created by erena on 29.05.2024.
//


#include "mask.h"

// Rook attack masks rookMask[square]
U64 rookMask[64];
// BishopMask[square]
U64 bishopMask[64];

// file masks [square]
U64 fileMasks[64];

// rank masks [square]
U64 rankMasks[64];

// isolated pawn masks [square]
U64 isolatedMasks[64];

// white passed pawn masks [square]
U64 whitePassedMasks[64];

// black passed pawn masks [square]
U64 blackPassedMasks[64];






