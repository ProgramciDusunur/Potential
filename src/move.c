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

void toggleHashesForPiece(board* position, int piece, int square) {
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

    // move piece
    popBit(position->bitboards[piece], sourceSquare);
    setBit(position->bitboards[piece], targetSquare);
    position->mailbox[sourceSquare] = NO_PIECE;
    position->mailbox[targetSquare] = piece;

    // hash piece
    toggleHashesForPiece(position, piece, sourceSquare);
    toggleHashesForPiece(position, piece, targetSquare);

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

        // remove it from corresponding bitboard
        popBit(position->bitboards[capturedPiece], targetSquare);

        // remove the piece from hash key
        toggleHashesForPiece(position, capturedPiece, targetSquare);
    }

    // handle enpassant captures
    if (enpass) {
        // reset fifty move rule counter
        position->fifty = 0;

        // white to move
        if (position->side == white) {
            // remove captured pawn
            popBit(position->bitboards[p], targetSquare + 8);
            position->mailbox[targetSquare + 8] = NO_PIECE;

            // remove pawn from hash key
            toggleHashesForPiece(position, p, targetSquare + 8);
        }

            // black to move
        else {
            // remove captured pawn
            popBit(position->bitboards[P], targetSquare - 8);
            position->mailbox[targetSquare - 8] = NO_PIECE;

            // remove pawn from hash key
            toggleHashesForPiece(position, P, targetSquare - 8);
        }

    }



    // handle pawn promotions
    if (promotedPiece) {
        // white to move
        if (position->side == white) {
            // erase the pawn from the target square
            popBit(position->bitboards[P], targetSquare);

            // remove pawn from hash key
            toggleHashesForPiece(position, P, targetSquare);
        }

            // black to move
        else {
            // erase the pawn from the target square
            popBit(position->bitboards[p], targetSquare);

            // remove pawn from hash key
            toggleHashesForPiece(position, p, targetSquare);
        }

        // set up promoted piece on chess board
        setBit(position->bitboards[promotedPiece], targetSquare);
        position->mailbox[targetSquare] = promotedPiece;

        // add promoted piece into the hash key
        toggleHashesForPiece(position, promotedPiece, targetSquare);
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
                // move H rook
                popBit(position->bitboards[R], h1);
                setBit(position->bitboards[R], f1);
                position->mailbox[h1] = NO_PIECE;
                position->mailbox[f1] = R;

                // hash rook
                toggleHashesForPiece(position, R, h1);
                toggleHashesForPiece(position, R, f1);
                break;

                // white castles queen side
            case (c1):
                // move A rook
                popBit(position->bitboards[R], a1);
                setBit(position->bitboards[R], d1);
                position->mailbox[a1] = NO_PIECE;
                position->mailbox[d1] = R;

                // hash rook
                toggleHashesForPiece(position, R, a1);
                toggleHashesForPiece(position, R, d1);
                break;

                // black castles king side
            case (g8):
                // move H rook
                popBit(position->bitboards[r], h8);
                setBit(position->bitboards[r], f8);
                position->mailbox[h8] = NO_PIECE;
                position->mailbox[f8] = r;

                // hash rook
                toggleHashesForPiece(position, r, h8);
                toggleHashesForPiece(position, r, f8);
                break;

                // black castles queen side
            case (c8):
                // move A rook
                popBit(position->bitboards[r], a8);
                setBit(position->bitboards[r], d8);
                position->mailbox[a8] = NO_PIECE;
                position->mailbox[d8] = r;

                // hash rook
                toggleHashesForPiece(position, r, a8);
                toggleHashesForPiece(position, r, d8);
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

bool legalityCheck(int moveFlag, int move, board *position) {
    struct copyposition copyPosition;
    // preserve board state
    copyBoard(position, &copyPosition);
    
    int isLegalCapture = getMoveCapture(move);
    if (moveFlag == onlyCaptures && !isLegalCapture) {
        // return illegal move
        return 0;
    }

     // make sure that king has not been exposed into a check
    if (isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[k]) : getLS1BIndex(position->bitboards[K]), position->side, position)) {        
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
