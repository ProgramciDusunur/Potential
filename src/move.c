//
// Created by erena on 13.09.2024.
//

#include <assert.h>
#include <stdbool.h>
#include "move.h"
#include "fen.h"

// Pawn attack masks pawnAttacks[side][square]
U64 pawnAttacks[2][64];
// Knight attack masks knightAttacks[square]
U64 knightAttacks[64];
// King attack masks kingAttacks[square]
U64 kingAttacks[64];
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
    cp->fiftyCopy = p->fifty;
    memcpy(cp->mailboxCopy, p->mailbox, 64);
    cp->hashKeyCopy = p->hashKey;
    cp->pawnKeyCopy = p->pawnKey;
    cp->minorKeyCopy = p->minorKey;
    cp->majorKeyCopy = p->majorKey;
    cp->whiteNonPawnKeyCopy = p->whiteNonPawnKey;
    cp->blackNonPawnKeyCopy = p->blackNonPawnKey;
    cp->krpKeyCopy = p->krpKey;
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
    p->fifty = cp->fiftyCopy;
    memcpy(p->mailbox, cp->mailboxCopy, 64);
    p->hashKey = cp->hashKeyCopy;
    p->pawnKey = cp->pawnKeyCopy;
    p->minorKey = cp->minorKeyCopy;
    p->majorKey = cp->majorKeyCopy;
    p->whiteNonPawnKey = cp->whiteNonPawnKeyCopy;
    p->blackNonPawnKey = cp->blackNonPawnKeyCopy;
    p->krpKey = cp->krpKeyCopy;
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
    return getBishopAttacks(square, occupancy) | getRookAttacks(square, occupancy);
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
    if (pawnAttacks[whichSide == white ? black : white][square] & position->bitboards[whichSide == white ? P : p]) {
        return 1;
    }
    if (knightAttacks[square] & position->bitboards[whichSide == white ? N : n]) {
        return 1;
    }
    if (getBishopAttacks(square, position->occupancies[both]) & position->bitboards[whichSide == white ? B : b]) {
        return 1;
    }
    if (kingAttacks[square] & position->bitboards[whichSide == white ? K : k]) {
        return 1;
    }
    if (getQueenAttacks(square, position->occupancies[both]) & position->bitboards[whichSide == white ? Q : q]) {
        return 1;
    }
    if (getRookAttacks(square, position->occupancies[both]) & position->bitboards[whichSide == white ? R : r]) {
        return 1;
    }
    return 0;
}

bool isMinor(int piece) {
    return piece == K || piece == k || piece == B || piece == b || piece == N || piece == n;
}

bool isMajor(int piece) {
    return piece == Q || piece == q || piece == R || piece == r;
}

bool isKRP(int piece) {
    return piece == K || piece == k || piece == R || piece == r;
}

inline void toggleHashesForPiece(board* position, int piece, int square) {
    position->hashKey ^= pieceKeys[piece][square];
    if (piece == P || piece == p) {
        position->pawnKey ^= pieceKeys[piece][square];
        position->krpKey ^= pieceKeys[piece][square];
    } else {
        if (pieceColor(piece) == white) {
            position->whiteNonPawnKey ^= pieceKeys[piece][square];
        } else {
            position->blackNonPawnKey ^= pieceKeys[piece][square];
        }
    }
    if (isMinor(piece)) {
        position->minorKey ^= pieceKeys[piece][square];
    }
    if (isMajor(piece)) {
        position->majorKey ^= pieceKeys[piece][square];
    }
    if (isKRP(piece)) {
        position->krpKey ^= pieceKeys[piece][square];
    }
}

inline void addPiece(board* position, int piece, int square) {
    setBit(position->bitboards[piece], square);
    setBit(position->occupancies[pieceColor(piece)], square);
    setBit(position->occupancies[both], square);
    position->mailbox[square] = piece;
    toggleHashesForPiece(position, piece, square);
}

inline void removePiece(board* position, int piece, int square) {
    assert(position->mailbox[square] == piece);
    popBit(position->bitboards[piece], square);
    popBit(position->occupancies[pieceColor(piece)], square);
    popBit(position->occupancies[both], square);
    position->mailbox[square] = NO_PIECE;
    toggleHashesForPiece(position, piece, square);
}

// make move on chess board
int makeMove(int move, int moveFlag, board* position) {
    int isLegalCapture = getMoveCapture(move);
    if (moveFlag == onlyCaptures && !isLegalCapture) {
        return 0;
    }    
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
    int capturedPiece = position->mailbox[targetSquare];

    // increment fifty move rule counter
    position->fifty++;

    if (piece == P || piece == p) {
        position->fifty = 0; // reset fifty move rule counter
    }

    // handling capture moves
    if (capture && !enpass) {
        assert(capturedPiece != NO_PIECE);

        // reset fifty move rule counter
        position->fifty = 0;

        removePiece(position, capturedPiece, targetSquare);
    }

    // move piece
    removePiece(position, piece, sourceSquare);
    addPiece(position, piece, targetSquare);

    // handle enpassant captures
    if (enpass) {
        // reset fifty move rule counter
        position->fifty = 0;

        if (position->side == white) {
            removePiece(position, p, targetSquare + 8);
        } else {
            removePiece(position, P, targetSquare - 8);
        }
    }

    // handle pawn promotions
    if (promotedPiece) {
        if (position->side == white) {
            removePiece(position, P, targetSquare);
        } else {
            removePiece(position, p, targetSquare);
        }
        addPiece(position, promotedPiece, targetSquare);
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
        position->hashKey ^= enpassantKeys[position->enpassant];
    }

    // handle castling moves
    if (castling) {
        switch (targetSquare) {
            // white castles king side
            case (g1):
                removePiece(position, R, h1);
                addPiece(position, R, f1);
                break;

            // white castles queen side
            case (c1):
                removePiece(position, R, a1);
                addPiece(position, R, d1);
                break;

            // black castles king side
            case (g8):
                removePiece(position, r, h8);
                addPiece(position, r, f8);
                break;

            // black castles queen side
            case (c8):
                removePiece(position, r, a8);
                addPiece(position, r, d8);
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

inline void splatPawnMoves(moves *moveList, U64 sourceBitboard, int shift, int piece, int promoPiece, int capture, int doublePush) {
    while (sourceBitboard) {
        int sourceSquare = getLS1BIndex(sourceBitboard);
        int targetSquare = sourceSquare + shift;

        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, promoPiece, capture, doublePush, 0, 0));

        popBit(sourceBitboard, sourceSquare);
    }
}

inline void splatEnpassant(moves *moveList, U64 sourceBitboard, int enpassantSquare, int piece) {
    while (sourceBitboard) {
        int sourceSquare = getLS1BIndex(sourceBitboard);

        addMove(moveList, encodeMove(sourceSquare, enpassantSquare, piece, 0, 1, 0, 1, 0));

        popBit(sourceBitboard, sourceSquare);
    }
}

inline void splatNormalMoves(moves *moveList, int sourceSquare, U64 targetBitboard, int piece, int capture) {
    while (targetBitboard) {
        int targetSquare = getLS1BIndex(targetBitboard);

        addMove(moveList, encodeMove(sourceSquare, targetSquare, piece, 0, capture, 0, 0, 0));

        popBit(targetBitboard, targetSquare);
    }
}

// generate all moves
void moveGenerator(moves *moveList, board* position) {
    // init move count
    moveList->count = 0;

    // define source & target squares
    int source_square, target_square;
    int piece;

    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;

    U64 enemy = position->occupancies[position->side == white ? black : white];
    U64 blockers = position->occupancies[both];
    U64 empty = ~blockers;

    // Pawn moves
    if (position->side == white) {
        bitboard = position->bitboards[P];

        U64 emptyAhead = bitboard & (empty << 8);
        U64 singlePush = emptyAhead & 0x00FFFFFFFFFF0000;
        U64 doublePush = emptyAhead & 0x00FF000000000000 & (empty << 16);
        U64 promotions = emptyAhead & 0x000000000000FF00;

        splatPawnMoves(moveList, singlePush, -8, P, 0, 0, 0);
        splatPawnMoves(moveList, doublePush, -16, P, 0, 0, 1);
        if (promotions) {
            splatPawnMoves(moveList, promotions, -8, P, Q, 0, 0);
            splatPawnMoves(moveList, promotions, -8, P, R, 0, 0);
            splatPawnMoves(moveList, promotions, -8, P, B, 0, 0);
            splatPawnMoves(moveList, promotions, -8, P, N, 0, 0);
        }

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000;
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000;
        U64 lPromotions = lEnemy & 0x000000000000FF00;
        U64 rPromotions = rEnemy & 0x000000000000FF00;

        splatPawnMoves(moveList, lSingleCapt, -9, P, 0, 1, 0);
        splatPawnMoves(moveList, rSingleCapt, -7, P, 0, 1, 0);
        if (lPromotions) {
            splatPawnMoves(moveList, lPromotions, -9, P, Q, 1, 0);
            splatPawnMoves(moveList, lPromotions, -9, P, R, 1, 0);
            splatPawnMoves(moveList, lPromotions, -9, P, B, 1, 0);
            splatPawnMoves(moveList, lPromotions, -9, P, N, 1, 0);
        }
        if (rPromotions) {
            splatPawnMoves(moveList, rPromotions, -7, P, Q, 1, 0);
            splatPawnMoves(moveList, rPromotions, -7, P, R, 1, 0);
            splatPawnMoves(moveList, rPromotions, -7, P, B, 1, 0);
            splatPawnMoves(moveList, rPromotions, -7, P, N, 1, 0);
        }

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[black][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant, P);
        }
    } else {
        bitboard = position->bitboards[p];

        U64 emptyAhead = bitboard & (empty >> 8);
        U64 singlePush = emptyAhead & 0x0000FFFFFFFFFF00;
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16);
        U64 promotions = emptyAhead & 0x00FF000000000000;

        splatPawnMoves(moveList, singlePush, +8, p, 0, 0, 0);
        splatPawnMoves(moveList, doublePush, +16, p, 0, 0, 1);
        if (promotions) {
            splatPawnMoves(moveList, promotions, +8, p, q, 0, 0);
            splatPawnMoves(moveList, promotions, +8, p, r, 0, 0);
            splatPawnMoves(moveList, promotions, +8, p, b, 0, 0);
            splatPawnMoves(moveList, promotions, +8, p, n, 0, 0);
        }

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00;
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00;
        U64 lPromotions = lEnemy & 0x00FF000000000000;
        U64 rPromotions = rEnemy & 0x00FF000000000000;

        splatPawnMoves(moveList, lSingleCapt, +7, p, 0, 1, 0);
        splatPawnMoves(moveList, rSingleCapt, +9, p, 0, 1, 0);
        if (lPromotions) {
            splatPawnMoves(moveList, lPromotions, +7, p, q, 1, 0);
            splatPawnMoves(moveList, lPromotions, +7, p, r, 1, 0);
            splatPawnMoves(moveList, lPromotions, +7, p, b, 1, 0);
            splatPawnMoves(moveList, lPromotions, +7, p, n, 1, 0);
        }
        if (rPromotions) {
            splatPawnMoves(moveList, rPromotions, +9, p, q, 1, 0);
            splatPawnMoves(moveList, rPromotions, +9, p, r, 1, 0);
            splatPawnMoves(moveList, rPromotions, +9, p, b, 1, 0);
            splatPawnMoves(moveList, rPromotions, +9, p, n, 1, 0);
        }

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[white][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant, p);
        }
    }

    // Knight moves
    piece = position->side == white ? N : n;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = knightAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, piece, 0);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, piece, 1);

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = position->side == white ? B : b;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, piece, 0);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, piece, 1);

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = position->side == white ? R : r;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getRookAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, piece, 0);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, piece, 1);

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = position->side == white ? Q : q;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, piece, 0);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, piece, 1);

        popBit(bitboard, sourceSquare);
    }

    // King moves
    piece = position->side == white ? K : k;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = kingAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, piece, 0);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, piece, 1);

        popBit(bitboard, sourceSquare);
    }

    // Castling moves
    if (position->side == white) {
        generate_white_king_side_castling(position, moveList);
        generate_white_queen_side_castling(position, moveList);
    } else {
        generate_black_king_side_castling(position, moveList);
        generate_black_queen_side_castling(position, moveList);
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

U64 pawn_threats(U64 pawnBitboard, int side) {    
    U64 forward_pawns = side == white ? (pawnBitboard >> 8) : (pawnBitboard << 8);
    U64 left_attacks =  forward_pawns << 1 & not_a_file;
    U64 right_attacks = forward_pawns >> 1 & not_h_file;

    return left_attacks | right_attacks;
}

U64 knight_threats (U64 knightBB) {
    U64 attacks =  (knightBB & not8And7RankHFile)  >> 15 |
                   (knightBB & not1And2RanksAFile) << 15 |
                   (knightBB & not8And7RankAFile)  >> 17 |
                   (knightBB & not1And2RankHFile)  << 17 |
                   (knightBB & not8RankAndABFile)  >> 10 |
                   (knightBB & not1RankAndGHFile)  << 10 |
                   (knightBB & not8RankAndGHFile)  >> 6  |
                   (knightBB & not1RankAndABFile)  << 6  ;    
    return attacks;
}
