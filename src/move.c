//
// Created by erena on 13.09.2024.
//

#include <assert.h>
#include <stdbool.h>
#include "move.h"
#include "fen.h"

#if __AVX512VBMI2__
#include <immintrin.h>
#endif

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
bool isTactical(uint16_t move) {
    return getMoveCapture(move) || getMovePromote(move);
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
void addMove(moves *moveList, uint16_t move) {
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
    if (getRookAttacks(square, position->occupancies[both]) & (position->bitboards[whichSide == white ? R : r] | position->bitboards[whichSide == white ? Q : q])) {
        return 1;
    }
    if (getBishopAttacks(square, position->occupancies[both]) & (position->bitboards[whichSide == white ? B : b] | position->bitboards[whichSide == white ? Q : q])) {
        return 1;
    }
    if (kingAttacks[square] & position->bitboards[whichSide == white ? K : k]) {
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
    position->mosaicKey ^= get_rng_hash(piece, square);
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
int makeMove(uint16_t move, int moveFlag, board* position) {
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
    int promote = getMovePromote(move);
    int promotedPiece = getMovePromotedPiece(position->side, move);
    int capture = getMoveCapture(move);
    int doublePush = getMoveDouble(move);
    int enpass = getMoveEnpassant(move);
    int castling = getMoveCastling(move);
    int piece = position->mailbox[sourceSquare];
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
    if (promote) {
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
                addMove(moveList, encodeMove(e1, g1, mf_castling));
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
                addMove(moveList, encodeMove(e1, c1, mf_castling));
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
                addMove(moveList, encodeMove(e8, g8, mf_castling));
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
                addMove(moveList, encodeMove(e8, c8, mf_castling));
        }
    }
}

#if __AVX512VBMI2__

inline static __m512i SPLAT_TEMPLATE_TARGET_HALF() {
    return _mm512_set_epi16(
        encodeMove(0, 31, 0), encodeMove(0, 30, 0), encodeMove(0, 29, 0), encodeMove(0, 28, 0), encodeMove(0, 27, 0), encodeMove(0, 26, 0), encodeMove(0, 25, 0), encodeMove(0, 24, 0),
        encodeMove(0, 23, 0), encodeMove(0, 22, 0), encodeMove(0, 21, 0), encodeMove(0, 20, 0), encodeMove(0, 19, 0), encodeMove(0, 18, 0), encodeMove(0, 17, 0), encodeMove(0, 16, 0),
        encodeMove(0, 15, 0), encodeMove(0, 14, 0), encodeMove(0, 13, 0), encodeMove(0, 12, 0), encodeMove(0, 11, 0), encodeMove(0, 10, 0), encodeMove(0, 9, 0),  encodeMove(0, 8, 0),
        encodeMove(0, 7, 0),  encodeMove(0, 6, 0),  encodeMove(0, 5, 0),  encodeMove(0, 4, 0),  encodeMove(0, 3, 0),  encodeMove(0, 2, 0),  encodeMove(0, 1, 0),  encodeMove(0, 0, 0)
    );
}

inline static __m512i SPLAT_TEMPLATE_SOURCE_CENTER() {
    return _mm512_set_epi16(
        encodeMove(47, 0, 0), encodeMove(46, 0, 0), encodeMove(45, 0, 0), encodeMove(44, 0, 0), encodeMove(43, 0, 0), encodeMove(42, 0, 0), encodeMove(41, 0, 0), encodeMove(40, 0, 0),
        encodeMove(39, 0, 0), encodeMove(38, 0, 0), encodeMove(37, 0, 0), encodeMove(36, 0, 0), encodeMove(35, 0, 0), encodeMove(34, 0, 0), encodeMove(33, 0, 0), encodeMove(32, 0, 0),
        encodeMove(31, 0, 0), encodeMove(30, 0, 0), encodeMove(29, 0, 0), encodeMove(28, 0, 0), encodeMove(27, 0, 0), encodeMove(26, 0, 0), encodeMove(25, 0, 0), encodeMove(24, 0, 0),
        encodeMove(23, 0, 0), encodeMove(22, 0, 0), encodeMove(21, 0, 0), encodeMove(20, 0, 0), encodeMove(19, 0, 0), encodeMove(18, 0, 0), encodeMove(17, 0, 0), encodeMove(16, 0, 0)
    );
}

inline static __m512i SPLAT_TEMPLATE_BOTH_HALF() {
    return _mm512_set_epi16(
        encodeMove(31, 31, 0), encodeMove(30, 30, 0), encodeMove(29, 29, 0), encodeMove(28, 28, 0), encodeMove(27, 27, 0), encodeMove(26, 26, 0), encodeMove(25, 25, 0), encodeMove(24, 24, 0),
        encodeMove(23, 23, 0), encodeMove(22, 22, 0), encodeMove(21, 21, 0), encodeMove(20, 20, 0), encodeMove(19, 19, 0), encodeMove(18, 18, 0), encodeMove(17, 17, 0), encodeMove(16, 16, 0),
        encodeMove(15, 15, 0), encodeMove(14, 14, 0), encodeMove(13, 13, 0), encodeMove(12, 12, 0), encodeMove(11, 11, 0), encodeMove(10, 10, 0), encodeMove(9, 9, 0),   encodeMove(8, 8, 0),
        encodeMove(7, 7, 0),   encodeMove(6, 6, 0),   encodeMove(5, 5, 0),   encodeMove(4, 4, 0),   encodeMove(3, 3, 0),   encodeMove(2, 2, 0),   encodeMove(1, 1, 0),   encodeMove(0, 0, 0)
    );
}

inline static __m128i SPLAT_TEMPLATE_BOTH_RANK() {
    return _mm_set_epi16(
        encodeMove(7, 7, 0), encodeMove(6, 6, 0), encodeMove(5, 5, 0), encodeMove(4, 4, 0), encodeMove(3, 3, 0), encodeMove(2, 2, 0), encodeMove(1, 1, 0), encodeMove(0, 0, 0)
    );
}

inline static __m512i SPLAT_TEMPLATE_BOTH_RANK_PROMO() {
    return _mm512_set_epi16(
        encodeMove(7, 7, mf_promo_n), encodeMove(7, 7, mf_promo_b), encodeMove(7, 7, mf_promo_r), encodeMove(7, 7, mf_promo_q),
        encodeMove(6, 6, mf_promo_n), encodeMove(6, 6, mf_promo_b), encodeMove(6, 6, mf_promo_r), encodeMove(6, 6, mf_promo_q),
        encodeMove(5, 5, mf_promo_n), encodeMove(5, 5, mf_promo_b), encodeMove(5, 5, mf_promo_r), encodeMove(5, 5, mf_promo_q),
        encodeMove(4, 4, mf_promo_n), encodeMove(4, 4, mf_promo_b), encodeMove(4, 4, mf_promo_r), encodeMove(4, 4, mf_promo_q),
        encodeMove(3, 3, mf_promo_n), encodeMove(3, 3, mf_promo_b), encodeMove(3, 3, mf_promo_r), encodeMove(3, 3, mf_promo_q),
        encodeMove(2, 2, mf_promo_n), encodeMove(2, 2, mf_promo_b), encodeMove(2, 2, mf_promo_r), encodeMove(2, 2, mf_promo_q),
        encodeMove(1, 1, mf_promo_n), encodeMove(1, 1, mf_promo_b), encodeMove(1, 1, mf_promo_r), encodeMove(1, 1, mf_promo_q),
        encodeMove(0, 0, mf_promo_n), encodeMove(0, 0, mf_promo_b), encodeMove(0, 0, mf_promo_r), encodeMove(0, 0, mf_promo_q)
    );
}

inline static void splat16(moves *moveList, uint32_t k, __m512i a, __m512i b) {
    int count = __builtin_popcount(k);
    __m512i to_write = _mm512_maskz_compress_epi16(k, _mm512_add_epi16(a, b));
    _mm512_storeu_si512(&moveList->moves[moveList->count], to_write);
    moveList->count += count;
}

inline static void splat64(moves *moveList, uint8_t k, __m512i a, __m512i b) {
    int count = __builtin_popcount(k);
    __m512i to_write = _mm512_maskz_compress_epi64(k, _mm512_add_epi16(a, b));
    _mm512_storeu_si512(&moveList->moves[moveList->count], to_write);
    moveList->count += count * 4;
}

inline static void splatPawnSingleMoves(moves *moveList, U64 sourceBitboard, int shift, int capture) {
    if (!sourceBitboard) return;

    __m512i extra0 = _mm512_set1_epi16(encodeMove(0, shift, capture ? mf_capture : mf_normal));
    __m512i extra1 = _mm512_set1_epi16(encodeMove(32, 32 + shift, capture ? mf_capture : mf_normal));

    splat16(moveList, (uint32_t)(sourceBitboard >> 0), SPLAT_TEMPLATE_BOTH_HALF(), extra0);
    splat16(moveList, (uint32_t)(sourceBitboard >> 32), SPLAT_TEMPLATE_BOTH_HALF(), extra1);
}

inline static void splatPawnDoubleMoves(moves *moveList, U64 sourceBitboard, int shift, int color) {
    if (!sourceBitboard) return;

    int offset = color == white ? 48 : 8;
    __m128i extra = _mm_set1_epi16(encodeMove(offset, shift + offset, mf_double));
    int count = countBits(sourceBitboard);
    __m128i to_write = _mm_maskz_compress_epi16(sourceBitboard >> offset, _mm_add_epi16(SPLAT_TEMPLATE_BOTH_RANK(), extra));
    _mm_storeu_si128((__m128i*)&moveList->moves[moveList->count], to_write);
    moveList->count += count;
}

inline static void splatPawnPromoMoves(moves *moveList, U64 sourceBitboard, int shift, int color, int capture) {
    if (!sourceBitboard) return;

    int offset = color == white ? 8 : 48;
    __m512i extra = _mm512_set1_epi16(encodeMove(offset, shift + offset, capture ? mf_capture : mf_normal));
    splat64(moveList, (uint8_t)(sourceBitboard >> offset), SPLAT_TEMPLATE_BOTH_RANK_PROMO(), extra);
}

inline static void splatEnpassant(moves *moveList, U64 sourceBitboard, int enpassantSquare) {
    if (!sourceBitboard) return;

    __m512i extra = _mm512_set1_epi16(encodeMove(0, enpassantSquare, mf_enpassant));

    splat16(moveList, (uint32_t)(sourceBitboard >> 16), SPLAT_TEMPLATE_SOURCE_CENTER(), extra);
}

inline static void splatNormalMoves(moves *moveList, int sourceSquare, U64 targetBitboard, int mf) {
    if (!targetBitboard) return;

    __m512i extra0 = _mm512_set1_epi16(encodeMove(sourceSquare, 0, mf));
    __m512i extra1 = _mm512_set1_epi16(encodeMove(sourceSquare, 32, mf));

    splat16(moveList, (uint32_t)(targetBitboard >> 0), SPLAT_TEMPLATE_TARGET_HALF(), extra0);
    splat16(moveList, (uint32_t)(targetBitboard >> 32), SPLAT_TEMPLATE_TARGET_HALF(), extra1);
}

#else

inline static void splatPawnMoves(moves *moveList, U64 sourceBitboard, int shift, int mf) {
    while (sourceBitboard) {
        int sourceSquare = getLS1BIndex(sourceBitboard);
        int targetSquare = sourceSquare + shift;

        addMove(moveList, encodeMove(sourceSquare, targetSquare, mf));

        popBit(sourceBitboard, sourceSquare);
    }
}

inline static void splatPawnSingleMoves(moves *moveList, U64 sourceBitboard, int shift, int capture) {
    splatPawnMoves(moveList, sourceBitboard, shift, capture ? mf_capture : mf_normal);
}

inline static void splatPawnDoubleMoves(moves *moveList, U64 sourceBitboard, int shift, int color) {
    (void) color;
    splatPawnMoves(moveList, sourceBitboard, shift, mf_double);
}

inline static void splatPawnPromoMoves(moves *moveList, U64 sourceBitboard, int shift, int color, int capture) {
    (void) color;
    while (sourceBitboard) {
        int sourceSquare = getLS1BIndex(sourceBitboard);
        int targetSquare = sourceSquare + shift;

        addMove(moveList, encodeMove(sourceSquare, targetSquare, capture ? mf_cap_promo_q : mf_promo_q));
        addMove(moveList, encodeMove(sourceSquare, targetSquare, capture ? mf_cap_promo_r : mf_promo_r));
        addMove(moveList, encodeMove(sourceSquare, targetSquare, capture ? mf_cap_promo_b : mf_promo_b));
        addMove(moveList, encodeMove(sourceSquare, targetSquare, capture ? mf_cap_promo_n : mf_promo_n));

        popBit(sourceBitboard, sourceSquare);
    }
}

inline static void splatEnpassant(moves *moveList, U64 sourceBitboard, int enpassantSquare) {
    while (sourceBitboard) {
        int sourceSquare = getLS1BIndex(sourceBitboard);

        addMove(moveList, encodeMove(sourceSquare, enpassantSquare, mf_enpassant));

        popBit(sourceBitboard, sourceSquare);
    }
}

inline static void splatNormalMoves(moves *moveList, int sourceSquare, U64 targetBitboard, int mf) {
    while (targetBitboard) {
        int targetSquare = getLS1BIndex(targetBitboard);

        addMove(moveList, encodeMove(sourceSquare, targetSquare, mf));

        popBit(targetBitboard, targetSquare);
    }
}

#endif

// generate all moves
void moveGenerator(moves *moveList, board* position) {
    // init move count
    moveList->count = 0;

    int piece;
    U64 bitboard;

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

        splatPawnSingleMoves(moveList, singlePush, -8, 0);
        splatPawnDoubleMoves(moveList, doublePush, -16, white);
        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000;
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000;
        U64 lPromotions = lEnemy & 0x000000000000FF00;
        U64 rPromotions = rEnemy & 0x000000000000FF00;

        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[black][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant);
        }
    } else {
        bitboard = position->bitboards[p];

        U64 emptyAhead = bitboard & (empty >> 8);
        U64 singlePush = emptyAhead & 0x0000FFFFFFFFFF00;
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16);
        U64 promotions = emptyAhead & 0x00FF000000000000;

        splatPawnSingleMoves(moveList, singlePush, +8, 0);
        splatPawnDoubleMoves(moveList, doublePush, +16, black);
        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00;
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00;
        U64 lPromotions = lEnemy & 0x00FF000000000000;
        U64 rPromotions = rEnemy & 0x00FF000000000000;

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[white][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant);
        }
    }

    // Knight moves
    piece = position->side == white ? N : n;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = knightAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = position->side == white ? B : b;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = position->side == white ? R : r;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getRookAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = position->side == white ? Q : q;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // King moves
    piece = position->side == white ? K : k;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = kingAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

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

void noisyGenerator(moves *moveList, board* position) {
    // init move count
    moveList->count = 0;

    int piece;
    U64 bitboard;

    U64 enemy = position->occupancies[position->side == white ? black : white];
    U64 blockers = position->occupancies[both];
    U64 empty = ~blockers;

    // Pawn moves
    if (position->side == white) {
        bitboard = position->bitboards[P];

        U64 emptyAhead = bitboard & (empty << 8);
        U64 promotions = emptyAhead & 0x000000000000FF00;

        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000;
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000;
        U64 lPromotions = lEnemy & 0x000000000000FF00;
        U64 rPromotions = rEnemy & 0x000000000000FF00;

        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[black][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant);
        }
    } else {
        bitboard = position->bitboards[p];

        U64 emptyAhead = bitboard & (empty >> 8);
        U64 promotions = emptyAhead & 0x00FF000000000000;

        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00;
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00;
        U64 lPromotions = lEnemy & 0x00FF000000000000;
        U64 rPromotions = rEnemy & 0x00FF000000000000;

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);

        if (position->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[white][position->enpassant];
            splatEnpassant(moveList, attackers, position->enpassant);
        }
    }

    // Knight moves
    piece = position->side == white ? N : n;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = knightAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = position->side == white ? B : b;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = position->side == white ? R : r;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getRookAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = position->side == white ? Q : q;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers);

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // King moves
    piece = position->side == white ? K : k;
    bitboard = position->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = kingAttacks[sourceSquare];

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
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


void addMoveToHistoryList(moves* list, uint16_t move) {
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
