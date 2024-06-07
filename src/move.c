//
// Created by erena on 29.05.2024.
//

#include "move.h"
#include "board.h"
#include "bit_manipulation.h"
#include "magic.h"
#include "mask.c"



void copyBoard(board *p, struct copyposition *cp) {
    cp->bitboardsCopy[0] = p->bitboards[0];
    cp->bitboardsCopy[1] = p->bitboards[1];
    cp->bitboardsCopy[2] = p->bitboards[2];
    cp->bitboardsCopy[3] = p->bitboards[3];
    cp->bitboardsCopy[4] = p->bitboards[4];
    cp->bitboardsCopy[5] = p->bitboards[5];
    cp->bitboardsCopy[6] = p->bitboards[6];
    cp->bitboardsCopy[7] = p->bitboards[7];
    cp->bitboardsCopy[8] = p->bitboards[8];
    cp->bitboardsCopy[9] = p->bitboards[9];
    cp->bitboardsCopy[10] = p->bitboards[10];
    cp->bitboardsCopy[11] = p->bitboards[11];
    cp->occupanciesCopy[0] = p->occupancies[0];
    cp->occupanciesCopy[1] = p->occupancies[1];
    cp->occupanciesCopy[2] = p->occupancies[2];
    cp->hashKeyCopy = p->hashKey;
    cp->sideCopy = p->side, cp->enpassantCopy = p->enpassant, cp->castleCopy = p->castle;
}

void takeBack(board *p, struct copyposition *cp) {
    p->bitboards[0] = cp->bitboardsCopy[0];
    p->bitboards[1] = cp->bitboardsCopy[1];
    p->bitboards[2] = cp->bitboardsCopy[2];
    p->bitboards[3] = cp->bitboardsCopy[3];
    p->bitboards[4] = cp->bitboardsCopy[4];
    p->bitboards[5] = cp->bitboardsCopy[5];
    p->bitboards[6] = cp->bitboardsCopy[6];
    p->bitboards[7] = cp->bitboardsCopy[7];
    p->bitboards[8] = cp->bitboardsCopy[8];
    p->bitboards[9] = cp->bitboardsCopy[9];
    p->bitboards[10] = cp->bitboardsCopy[10];
    p->bitboards[11] = cp->bitboardsCopy[11];
    p->occupancies[0] = cp->occupanciesCopy[0];
    p->occupancies[1] = cp->occupanciesCopy[1];
    p->occupancies[2] = cp->occupanciesCopy[2];
    p->hashKey = cp->hashKeyCopy;
    p->side = cp->sideCopy, p->enpassant = cp->enpassantCopy, p->castle = cp->castleCopy;
}

// Pawn attack masks pawnAttacks[side][square]
U64 pawnAtacks[2][64];
// Knight attack masks knightAttacks[square]
U64 knightAttacks[64];
// King attack masks kingAttacks[square]
U64 kingAttacks[64];
// Bishop attack table [square][occupancies]
U64 bishopAttacks[64][512];
// Rook attack table [square][occupancies]
U64 rookAttacks[64][4096];

// add move to the move list
static inline void addMove(moves *moveList, int move) {
    // store move
    moveList->moves[moveList->count] = move;
    // increment move count
    moveList->count++;
}

// get bishop attacks
static inline U64 getBishopAttacks(int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishopMask[square];
    occupancy *= bishopMagic[square];
    occupancy >>= 64 - bishopRelevantBits[square];
    return bishopAttacks[square][occupancy];
}

// get rook attacks
static inline U64 getRookAttacks(int square, U64 occupancy) {
    // get rook attacks assuming current board occupancy
    occupancy &= rookMask[square];
    occupancy *= rookMagic[square];
    occupancy >>= 64 - rookRelevantBits[square];
    return rookAttacks[square][occupancy];
}

// get queen attacks
static inline U64 getQueenAttacks(int square, U64 occupancy) {
    // get queen attacks assuming current board occupancy
    U64 queenAttacks;
    U64 bishopOccupancy = occupancy;
    U64 rookOccupancy = occupancy;
    bishopOccupancy &= bishopMask[square];
    bishopOccupancy *= bishopMagic[square];
    bishopOccupancy >>= 64 - bishopRelevantBits[square];
    queenAttacks = bishopAttacks[square][bishopOccupancy];

    rookOccupancy &= rookMask[square];
    rookOccupancy *= rookMagic[square];
    rookOccupancy >>= 64 - rookRelevantBits[square];
    queenAttacks |= rookAttacks[square][rookOccupancy];
    return queenAttacks;
}

static inline int isSquareAttacked(int square, int whichSide, board* position) {
    if ((whichSide == white) && (pawnAtacks[black][square] & position->bitboards[P])) {
        return 1;
    }
    if ((whichSide == black) && (pawnAtacks[white][square] & position->bitboards[p])) {
        return 1;
    }
    if (knightAttacks[square] & ((whichSide == white) ? position->bitboards[N] : position->bitboards[n])) {
        return 1;
    }
    if (getBishopAttacks(square, position->occupancies[both]) & ((whichSide == white) ? position->bitboards[B] : position->bitboards[b])) {
        return 1;
    }
    if (kingAttacks[square] & ((whichSide == white) ? position->bitboards[K] : position->bitboards[k])) {
        return 1;
    }
    if (getQueenAttacks(square, position->occupancies[both]) & ((whichSide == white) ? position->bitboards[Q] : position->bitboards[q])) {
        return 1;
    }
    if (getRookAttacks(square, position->occupancies[both]) & ((whichSide == white) ? position->bitboards[R] : position->bitboards[r])) {
        return 1;
    }
    return 0;
}




// generate all moves
static inline void moveGenerator(moves *moveList, board* position) {
    // init move count
    moveList->count = 0;


    // define source & target squares
    int source_square, target_square;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++) {
        // init piece bitboard copy
        bitboard = position->bitboards[piece];

        // generate white pawns & white king castling moves
        if (position->side == white) {
            // pick up white pawn bitboards index
            if (piece == P) {
                // loop over white pawns within white pawn bitboard
                while (bitboard) {
                    // init source square
                    source_square = getLS1BIndex(bitboard);

                    // init target square
                    target_square = source_square - 8;

                    // generate quiet pawn moves
                    if (!(target_square < a8) && !(getBit(position->occupancies[both], target_square))) {
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, R, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, B, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, N, 0, 0, 0, 0));
                        } else {
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) &&
                                !(getBit(position->occupancies[both], (target_square - 8))))
                                addMove(moveList, encodeMove(source_square, (target_square - 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawnAtacks[position->side][source_square] & position->occupancies[black];

                    // generate pawn captures
                    while (attacks) {
                        // init target square
                        target_square = getLS1BIndex(attacks);

                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, R, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, B, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, N, 1, 0, 0, 0));
                        } else
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        popBit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (position->enpassant != no_sq) {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawnAtacks[position->side][source_square] & (1ULL << position->enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks) {
                            // init enpassant capture target square
                            int target_enpassant = getLS1BIndex(enpassant_attacks);
                            addMove(moveList, encodeMove(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == K) {
                // king side castling is available
                if (position->castle & wk) {
                    // make sure square between king and king's rook are empty
                    if (!(getBit(position->occupancies[both], f1)) && !(getBit(position->occupancies[both], g1))) {
                        // make sure king and the f1 squares are not under attacks
                        if (!isSquareAttacked(e1, black, position) && !isSquareAttacked(f1, black, position))
                            addMove(moveList, encodeMove(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (position->castle & wq) {
                    // make sure square between king and queen's rook are empty
                    if (!(getBit(position->occupancies[both], d1)) && !(getBit(position->occupancies[both], c1)) &&
                        !(getBit(position->occupancies[both], b1))) {
                        // make sure king and the d1 squares are not under attacks
                        if (!isSquareAttacked(e1, black, position) && !isSquareAttacked(d1, black, position))
                            addMove(moveList, encodeMove(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

            // generate black pawns & black king castling moves
        else {
            // pick up black pawn bitboards index
            if (piece == p) {
                // loop over white pawns within white pawn bitboard
                while (bitboard) {
                    // init source square
                    source_square = getLS1BIndex(bitboard);

                    // init target square
                    target_square = source_square + 8;

                    // generate quiet pawn moves
                    if (!(target_square > h1) && !(getBit(position->occupancies[both], target_square))) {
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, q, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, r, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, b, 0, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, n, 0, 0, 0, 0));
                        } else {
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) &&
                                !(getBit(position->occupancies[both], (target_square + 8))))
                                addMove(moveList, encodeMove(source_square, (target_square + 8), piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // init pawn attacks bitboard
                    attacks = pawnAtacks[position->side][source_square] & position->occupancies[white];

                    // generate pawn captures
                    while (attacks) {
                        // init target square
                        target_square = getLS1BIndex(attacks);

                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2) {
                            addMove(moveList, encodeMove(source_square, target_square, piece, q, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, r, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, b, 1, 0, 0, 0));
                            addMove(moveList, encodeMove(source_square, target_square, piece, n, 1, 0, 0, 0));
                        } else
                            // one square ahead pawn move
                            addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                        // pop ls1b of the pawn attacks
                        popBit(attacks, target_square);
                    }

                    // generate enpassant captures
                    if (position->enpassant != no_sq) {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawnAtacks[position->side][source_square] & (1ULL << position->enpassant);

                        // make sure enpassant capture available
                        if (enpassant_attacks) {
                            // init enpassant capture target square
                            int target_enpassant = getLS1BIndex(enpassant_attacks);
                            addMove(moveList, encodeMove(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == k) {
                // king side castling is available
                if (position->castle & bk) {
                    // make sure square between king and king's rook are empty
                    if (!(getBit(position->occupancies[both], f8)) && !(getBit(position->occupancies[both], g8))) {
                        // make sure king and the f8 squares are not under attacks
                        if (!isSquareAttacked(e8, white, position) && !isSquareAttacked(f8, white, position))
                            addMove(moveList, encodeMove(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }

                // queen side castling is available
                if (position->castle & bq) {
                    // make sure square between king and queen's rook are empty
                    if (!(getBit(position->occupancies[both], d8)) && !(getBit(position->occupancies[both], c8)) &&
                        !(getBit(position->occupancies[both], b8))) {
                        // make sure king and the d8 squares are not under attacks
                        if (!isSquareAttacked(e8, white, position) && !isSquareAttacked(d8, white, position))
                            addMove(moveList, encodeMove(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        // genarate knight moves
        if ((position->side == white) ? piece == N : piece == n) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = knightAttacks[source_square] & ((position->side == white) ? ~position->occupancies[white] : ~position->occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate bishop moves
        if ((position->side == white) ? piece == B : piece == b) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getBishopAttacks(source_square, position->occupancies[both]) &
                          ((position->side == white) ? ~position->occupancies[white] : ~position->occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate rook moves
        if ((position->side == white) ? piece == R : piece == r) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getRookAttacks(source_square, position->occupancies[both]) &
                          ((position->side == white) ? ~position->occupancies[white] : ~position->occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }

        // generate queen moves
        if ((position->side == white) ? piece == Q : piece == q) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = getQueenAttacks(source_square, position->occupancies[both]) &
                          ((position->side == white) ? ~position->occupancies[white] : ~position->occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }


                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }
        // generate king moves
        if ((position->side == white) ? piece == K : piece == k) {
            // loop over source squares of piece bitboard copy
            while (bitboard) {
                // init source square
                source_square = getLS1BIndex(bitboard);

                // init piece attacks in order to get set of target squares
                attacks = kingAttacks[source_square] & ((position->side == white) ? ~position->occupancies[white] : ~position->occupancies[black]);

                // loop over target squares available from generated attacks
                while (attacks) {
                    // init target square
                    target_square = getLS1BIndex(attacks);

                    // quiet move
                    if (!(getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square)))
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));

                    else
                        // capture move
                        addMove(moveList, encodeMove(source_square, target_square, piece, 0, 1, 0, 0, 0));

                    // pop ls1b in current attacks set
                    popBit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                popBit(bitboard, source_square);
            }
        }
    }
}


