//
// Created by erena on 13.09.2024.
//

#include <stdbool.h>
#include "move.h"
#include "fen.h"

// Pawn attack masks pawnAttacks[side][square]
U64 pawnAttacks[2][64];
// Knight attack masks knightAttacks[square]
U64 knightAttacks[64];
// King attack masks kingAttacks[square]
U64 kingAttacks[64];
// Outer king ring masks
U64 outerKingRing[64];
// Bishop attack table [square][occupancies]
U64 bishopAttacks[64][512];
// Rook attack table [square][occupancies]
U64 rookAttacks[64][4096];

// Make sure the move isn't capture or promotion
bool isTactical(int move) {
    return getMoveCapture(move) || getMovePromoted(move);
}


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
    memcpy(cp->mailboxCopy, p->mailbox, 64);
    cp->hashKeyCopy = p->hashKey;
    cp->pawnKeyCopy = p->pawnKey;
    cp->minorKeyCopy = p->minorKey;
    cp->majorKeyCopy = p->majorKey;
    cp->whiteNonPawnKeyCopy = p->whiteNonPawnKey;
    cp->blackNonPawnKeyCopy = p->blackNonPawnKey;
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
    memcpy(p->mailbox, cp->mailboxCopy, 64);
    p->hashKey = cp->hashKeyCopy;
    p->pawnKey = cp->pawnKeyCopy;
    p->minorKey = cp->minorKeyCopy;
    p->majorKey = cp->majorKeyCopy;
    p->whiteNonPawnKey = cp->whiteNonPawnKeyCopy;
    p->blackNonPawnKey = cp->blackNonPawnKeyCopy;
    p->side = cp->sideCopy, p->enpassant = cp->enpassantCopy, p->castle = cp->castleCopy;
}


// add move to the move list
void addMove(moves *moveList, int move) {
    // store move
    moveList->moves[moveList->count] = move;
    // increment move count
    moveList->count++;
}

// get bishop attacks
U64 getBishopAttacks(int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishopMask[square];
    occupancy *= bishopMagic[square];
    occupancy >>= 64 - bishopRelevantBits[square];
    return bishopAttacks[square][occupancy];
}

// get rook attacks
U64 getRookAttacks(int square, U64 occupancy) {
    // get rook attacks assuming current board occupancy
    occupancy &= rookMask[square];
    occupancy *= rookMagic[square];
    occupancy >>= 64 - rookRelevantBits[square];
    return rookAttacks[square][occupancy];
}

// get queen attacks
U64 getQueenAttacks(int square, U64 occupancy) {
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

U64 getPawnAttacks(uint8_t side, int square) {
    return pawnAttacks[side][square];
}

U64 getKnightAttacks(int square) {
    return knightAttacks[square];
}

U64 getKingAttacks(int square) {
    return kingAttacks[square];
}

int isSquareAttacked(int square, int whichSide, board* position) {
    if ((whichSide == white) && (pawnAttacks[black][square] & position->bitboards[P])) {
        return 1;
    }
    if ((whichSide == black) && (pawnAttacks[white][square] & position->bitboards[p])) {
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

bool isMinor (int piece) {
    if (piece == K || piece == k || piece == B || piece == b || piece == N || piece == n) {
        return true;
    }
    return false;
}

bool isMajor (int piece) {
    if (piece == Q || piece == q || piece == R || piece == r) {
        return true;
    }
    return false;
}

// make move on chess board
int makeMove(int move, int moveFlag, board* position) {
    int isLegalCapture = getMoveCapture(move);
    if (moveFlag == onlyCaptures && !isLegalCapture) {
        return 0;
    }
    // quiet moves
    struct copyposition copyPosition;
    // preserve board state
    copyBoard(position, &copyPosition);

    // parse move
    int sourceSquare = getMoveSource(move);
    int targetSquare = getMoveTarget(move);
    int piece = getMovePiece(move);
    int promotedPiece = getMovePromoted(move);
    int capture = getMoveCapture(move);
    int doublePush = getMoveDouble(move);
    int enpass = getMoveEnpassant(move);
    int castling = getMoveCastling(move);

    // move piece
    popBit(position->bitboards[piece], sourceSquare);
    setBit(position->bitboards[piece], targetSquare);
    position->mailbox[sourceSquare] = NO_PIECE;
    position->mailbox[targetSquare] = piece;

    // hash piece
    position->hashKey ^= pieceKeys[piece][sourceSquare]; // remove piece from source square in hash key
    position->hashKey ^= pieceKeys[piece][targetSquare]; // set piece to the target square in hash key

    if (piece == P || piece == p) {
        position->pawnKey ^= pieceKeys[piece][sourceSquare];
        position->pawnKey ^= pieceKeys[piece][targetSquare];
    } else { // non pawn key

        if (position->side == white) {
            position->whiteNonPawnKey ^= pieceKeys[piece][sourceSquare];
            position->whiteNonPawnKey ^= pieceKeys[piece][targetSquare];
        } else {
            position->blackNonPawnKey ^= pieceKeys[piece][sourceSquare];
            position->blackNonPawnKey ^= pieceKeys[piece][targetSquare];
        }
    }

    if (isMinor(piece)) {
        position->minorKey ^= pieceKeys[piece][sourceSquare];
        position->minorKey ^= pieceKeys[piece][targetSquare];
    }

    if (isMajor(piece)) {
        position->majorKey ^= pieceKeys[piece][sourceSquare];
        position->majorKey ^= pieceKeys[piece][targetSquare];

    }


    // handling capture moves
    if (capture) {

        int startPiece, endPiece;
        if (position->side == white) {
            startPiece = p;
            endPiece = k;
        } else {
            startPiece = P;
            endPiece = K;
        }
        for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++) {
            if (getBit(position->bitboards[bbPiece], targetSquare)) {
                // remove it from corresponding bitboard
                popBit(position->bitboards[bbPiece], targetSquare);

                // remove the piece from hash key
                position->hashKey ^= pieceKeys[bbPiece][targetSquare];

                if (bbPiece == P || bbPiece ==  p) {

                    position->pawnKey ^= pieceKeys[bbPiece][targetSquare];

                } else { // non pawn key

                    if (position->side == white) {
                        position->blackNonPawnKey ^= pieceKeys[bbPiece][targetSquare];
                    } else {
                        position->whiteNonPawnKey ^= pieceKeys[bbPiece][targetSquare];
                    }
                }

                if (isMinor(bbPiece)) {
                    position->minorKey ^= pieceKeys[bbPiece][targetSquare];
                }

                if (isMajor(bbPiece)) {
                    position->majorKey ^= pieceKeys[bbPiece][targetSquare];
                }

                break;
            }
        }
    }

    // handle enpassant captures
    if (enpass) {
        // white to move
        if (position->side == white) {
            // remove captured pawn
            popBit(position->bitboards[p], targetSquare + 8);
            position->mailbox[targetSquare + 8] = NO_PIECE;

            // remove pawn from hash key
            position->hashKey ^= pieceKeys[p][targetSquare + 8];
            position->pawnKey ^= pieceKeys[p][targetSquare + 8];
        }

            // black to move
        else {
            // remove captured pawn
            popBit(position->bitboards[P], targetSquare - 8);
            position->mailbox[targetSquare - 8] = NO_PIECE;

            // remove pawn from hash key
            position->hashKey ^= pieceKeys[P][targetSquare - 8];
            position->pawnKey ^= pieceKeys[P][targetSquare - 8];
        }

    }



    // handle pawn promotions
    if (promotedPiece) {
        // white to move
        if (position->side == white) {
            // erase the pawn from the target square
            popBit(position->bitboards[P], targetSquare);

            // remove pawn from hash key
            position->hashKey ^= pieceKeys[P][targetSquare];
            position->pawnKey ^= pieceKeys[P][targetSquare];
            position->whiteNonPawnKey ^= pieceKeys[promotedPiece][targetSquare];
        }

            // black to move
        else {
            // erase the pawn from the target square
            popBit(position->bitboards[p], targetSquare);

            // remove pawn from hash key
            position->hashKey ^= pieceKeys[p][targetSquare];
            position->pawnKey ^= pieceKeys[p][targetSquare];
            position->blackNonPawnKey ^= pieceKeys[promotedPiece][targetSquare];
        }

        // set up promoted piece on chess board
        setBit(position->bitboards[promotedPiece], targetSquare);
        position->mailbox[targetSquare] = promotedPiece;

        // add promoted piece into the hash key
        position->hashKey ^= pieceKeys[promotedPiece][targetSquare];


        if (isMinor(promotedPiece)) {
            position->minorKey ^= pieceKeys[promotedPiece][targetSquare];
        }

        if (isMajor(promotedPiece)) {
            position->majorKey ^= pieceKeys[promotedPiece][targetSquare];

        }
    }


    // hash enpassant if available (remove enpassant square from hash key )
    if (position->enpassant != no_sq) position->hashKey ^= enpassantKeys[position->enpassant];

    // reset enpassant square
    position->enpassant = no_sq;

    // handle double pawn push
    if (doublePush) {        
        // set enpassant square
        position->enpassant = targetSquare + enPassantSquares[position->side];

        // hash enpassant
        position->hashKey ^= enpassantKeys[targetSquare + enPassantSquares[position->side]];

    }

    // handle castling moves
    if (castling) {
        switch (targetSquare) {
            // white castles king side
            case (g1):
                // move H rook
                popBit(position->bitboards[R], h1);
                setBit(position->bitboards[R], f1);
                position->mailbox[h1] = NO_PIECE;
                position->mailbox[f1] = R;

                // hash rook
                position->hashKey ^= pieceKeys[R][h1];  // remove rook from h1 from hash key
                position->hashKey ^= pieceKeys[R][f1];  // put rook on f1 into a hash key
                position->whiteNonPawnKey ^= pieceKeys[R][h1];
                position->whiteNonPawnKey ^= pieceKeys[R][f1];
                position->majorKey ^= pieceKeys[R][h1];
                position->majorKey ^= pieceKeys[R][f1];
                break;

                // white castles queen side
            case (c1):
                // move A rook
                popBit(position->bitboards[R], a1);
                setBit(position->bitboards[R], d1);
                position->mailbox[a1] = NO_PIECE;
                position->mailbox[d1] = R;

                // hash rook
                position->hashKey ^= pieceKeys[R][a1];  // remove rook from a1 from hash key
                position->hashKey ^= pieceKeys[R][d1];  // put rook on d1 into a hash key
                position->whiteNonPawnKey ^= pieceKeys[R][a1];
                position->whiteNonPawnKey ^= pieceKeys[R][d1];
                position->majorKey ^= pieceKeys[R][a1];
                position->majorKey ^= pieceKeys[R][d1];
                break;

                // black castles king side
            case (g8):
                // move H rook
                popBit(position->bitboards[r], h8);
                setBit(position->bitboards[r], f8);
                position->mailbox[h8] = NO_PIECE;
                position->mailbox[f8] = r;

                // hash rook
                position->hashKey ^= pieceKeys[r][h8];  // remove rook from h8 from hash key
                position->hashKey ^= pieceKeys[r][f8];  // put rook on f8 into a hash key
                position->blackNonPawnKey ^= pieceKeys[r][h8];
                position->blackNonPawnKey ^= pieceKeys[r][f8];
                position->majorKey ^= pieceKeys[r][h8];
                position->majorKey ^= pieceKeys[r][f8];
                break;

                // black castles queen side
            case (c8):
                // move A rook
                popBit(position->bitboards[r], a8);
                setBit(position->bitboards[r], d8);
                position->mailbox[a8] = NO_PIECE;
                position->mailbox[d8] = r;

                // hash rook
                position->hashKey ^= pieceKeys[r][a8];  // remove rook from a8 from hash key
                position->hashKey ^= pieceKeys[r][d8];  // put rook on d8 into a hash key
                position->blackNonPawnKey ^= pieceKeys[r][a8];
                position->blackNonPawnKey ^= pieceKeys[r][d8];
                position->majorKey ^= pieceKeys[r][a8];
                position->majorKey ^= pieceKeys[r][d8];
                break;
        }
    }


    // hash castling
    position->hashKey ^= castleKeys[position->castle];

    // update castling rights
    position->castle &= castlingRights[sourceSquare];
    position->castle &= castlingRights[targetSquare];

    // hash castling
    position->hashKey ^= castleKeys[position->castle];

    // reset occupancies
    position->occupancies[white] = 0LL;
    position->occupancies[black] = 0LL,
    position->occupancies[both] = 0LL;

    // loop over white pieces bitboards
    for (int bbPiece = P; bbPiece <= K; bbPiece++) {
        // update white occupancies
        position->occupancies[white] |= position->bitboards[bbPiece];
    }
    // loop over black pieces bitboards
    for (int bbPiece = p; bbPiece <= k; bbPiece++) {
        // update black occupancies
        position->occupancies[black] |= position->bitboards[bbPiece];
    }
    // update both side occupancies
    position->occupancies[both] |= position->occupancies[white];
    position->occupancies[both] |= position->occupancies[black];

    // change side
    position->side ^= 1;

    // hash side
    position->hashKey ^= sideKey;

    // make sure that king has not been exposed into a check
    if (isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[k]) : getLS1BIndex(position->bitboards[K]), position->side, position)) {
        // take move back
        takeBack(position, &copyPosition);
        // return illegal move
        return 0;
    }    

    return 1;
}



// pawn promotions
/*void generate_quiet_white_pawn_promotion(int source_square, int target_square, moves *moveList) {
    addMove(moveList, encodeMove(source_square, target_square, piece, Q, 0, 0, 0, 0));
    addMove(moveList, encodeMove(source_square, target_square, piece, R, 0, 0, 0, 0));
    addMove(moveList, encodeMove(source_square, target_square, piece, B, 0, 0, 0, 0));
    addMove(moveList, encodeMove(source_square, target_square, piece, N, 0, 0, 0, 0));
}

void generate_quiet_black_pawn_promotion(int source_square, int target_square, moves *moveList) {

}

// pawn push
void generate_one_pawn_push(int source_square, int target_square, moves *moveList) {
    addMove(moveList, encodeMove(source_square, target_square, piece, 0, 0, 0, 0, 0));
}

void generate_white_double_pawn_push(int source_square, int target_square, moves *moveList) {

}

void generate_black_double_pawn_push(int source_square, int target_square, moves *moveList) {

}*/

// castling
void generate_white_king_side_castling(board *position, moves *moveList) {
    // king side castling is available
    if (position->castle & wk) {
        // make sure square between king and king's rook are empty
        if (!(getBit(position->occupancies[both], f1)) && !(getBit(position->occupancies[both], g1))) {
            // make sure king and the f1 squares are not under attacks
            if (!isSquareAttacked(e1, black, position) && !isSquareAttacked(f1, black, position))
                addMove(moveList, encodeMove(e1, g1, K, 0, 0, 0, 0, 1));
        }
    }
}

void generate_white_queen_side_castling(board *position, moves *moveList) {
    // queen side castling is available
    if (position->castle & wq) {
        // make sure square between king and queen's rook are empty
        if (!(getBit(position->occupancies[both], d1)) && !(getBit(position->occupancies[both], c1)) &&
            !(getBit(position->occupancies[both], b1))) {
            // make sure king and the d1 squares are not under attacks
            if (!isSquareAttacked(e1, black, position) && !isSquareAttacked(d1, black, position))
                addMove(moveList, encodeMove(e1, c1, K, 0, 0, 0, 0, 1));
        }
    }
}

void generate_black_king_side_castling(board *position, moves *moveList) {
    // king side castling is available
    if (position->castle & bk) {
        // make sure square between king and king's rook are empty
        if (!(getBit(position->occupancies[both], f8)) && !(getBit(position->occupancies[both], g8))) {
            // make sure king and the f8 squares are not under attacks
            if (!isSquareAttacked(e8, white, position) && !isSquareAttacked(f8, white, position))
                addMove(moveList, encodeMove(e8, g8, k, 0, 0, 0, 0, 1));
        }
    }
}

void generate_black_queen_side_castling(board *position, moves *moveList) {
    // queen side castling is available
    if (position->castle & bq) {
        // make sure square between king and queen's rook are empty
        if (!(getBit(position->occupancies[both], d8)) && !(getBit(position->occupancies[both], c8)) &&
            !(getBit(position->occupancies[both], b8))) {
            // make sure king and the d8 squares are not under attacks
            if (!isSquareAttacked(e8, white, position) && !isSquareAttacked(d8, white, position))
                addMove(moveList, encodeMove(e8, c8, k, 0, 0, 0, 0, 1));
        }
    }
}

// enpassant
void generate_white_enpassant(board *position, int source_square, moves *moveList) {
    if (position->enpassant != no_sq) {
        // lookup pawn attacks and bitwise AND with enpassant square (bit)
        U64 enpassant_attacks = pawnAttacks[position->side][source_square] & (1ULL << position->enpassant);

        // make sure enpassant capture available
        if (enpassant_attacks) {
            // init enpassant capture target square
            int target_enpassant = getLS1BIndex(enpassant_attacks);
            addMove(moveList, encodeMove(source_square, target_enpassant, P, 0, 1, 0, 1, 0));
        }
    }
}

void generate_black_enpassant(board *position, int source_square, moves *moveList) {
    if (position->enpassant != no_sq) {
        // lookup pawn attacks and bitwise AND with enpassant square (bit)
        U64 enpassant_attacks = pawnAttacks[position->side][source_square] & (1ULL << position->enpassant);

        // make sure enpassant capture available
        if (enpassant_attacks) {
            // init enpassant capture target square
            int target_enpassant = getLS1BIndex(enpassant_attacks);
            addMove(moveList, encodeMove(source_square, target_enpassant, p, 0, 1, 0, 1, 0));
        }
    }
}




// generate all captures and promotions
void noisyGenerator(moves *moveList, board* position) {
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
                    // init pawn attacks bitboard
                    attacks = pawnAttacks[position->side][source_square] & position->occupancies[black];
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
                    generate_white_enpassant(position, source_square, moveList);
                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
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
                    // init pawn attacks bitboard
                    attacks = pawnAttacks[position->side][source_square] & position->occupancies[white];
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
                    generate_black_enpassant(position, source_square, moveList);
                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
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
                    // capture move
                    if (getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square))
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
                    // capture move
                    if (getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square))
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
                    // capture move
                    if (getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square))
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
                    // capture move
                    if (getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square))
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
                    // capture move
                    if (getBit(((position->side == white) ? position->occupancies[black] : position->occupancies[white]), target_square))
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



// generate all moves
void moveGenerator(moves *moveList, board* position) {
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
                    if (target_square >= a8 && !(getBit(position->occupancies[both], target_square))) {
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
                    attacks = pawnAttacks[position->side][source_square] & position->occupancies[black];

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
                    generate_white_enpassant(position, source_square, moveList);

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == K) {
                generate_white_king_side_castling(position, moveList);
                generate_white_queen_side_castling(position, moveList);
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
                    if (target_square <= h1 && !(getBit(position->occupancies[both], target_square))) {
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
                    attacks = pawnAttacks[position->side][source_square] & position->occupancies[white];

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
                    generate_black_enpassant(position, source_square, moveList);

                    // pop ls1b from piece bitboard copy
                    popBit(bitboard, source_square);
                }
            }

            // castling moves
            if (piece == k) {
                generate_black_king_side_castling(position, moveList);
                generate_black_queen_side_castling(position, moveList);
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

// init slider piece's attack tables
void initSlidersAttacks(int bishop) {
    // init bishop & rook masks
    for (int square = 0; square < 64; square++) {
        bishopMask[square] = maskBishopAttacks(square);
        rookMask[square] = maskRookAttacks(square);

        // init current mask
        U64 attackMask = bishop ? bishopMask[square] : rookMask[square];

        // init relevant occupancy bit count
        int relevantBitsCount = countBits(attackMask);

        // init occupancyIndicies
        int occupancIndicies = 1 << relevantBitsCount;

        // loop over occupancy indicies
        for (int index = 0; index < occupancIndicies; index++) {
            // bishop
            if (bishop) {
                // int current occupancy variation
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * bishopMagic[square]) >> (64 - bishopRelevantBits[square]);

                // init bishop attacks
                bishopAttacks[square][magicIndex] = bishopAttack(square, occupancy);
            }
                // rook
            else {
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * rookMagic[square]) >> (64 - rookRelevantBits[square]);

                // init bishop attacks
                rookAttacks[square][magicIndex] = rookAttack(square, occupancy);
            }
        }
    }
}

void initLeaperAttacks(void) {
    for (int square = 0; square < 64; square++) {
        // init pawn attacks
        pawnAttacks[white][square] = maskPawnAttacks(white, square);
        pawnAttacks[black][square] = maskPawnAttacks(black, square);

        // init knight attacks
        knightAttacks[square] = maskKnightAttacks(square);

        // init king attacks
        kingAttacks[square] = maskKingAttacks(square);
    }
}


void addMoveToHistoryList(moves* list, int move) {
    // don't pass move list border
    if (list->count < 256) {
        list->moves[list->count] = move;
        list->count += 1;
    }
}
