//
// Created by erena on 13.09.2024.
//

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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
// PEXT
U64 bishopAttacksPEXT[64][512];
U64 rookAttacksPEXT[64][4096];
// Helper bitboards for pinned pieces
uint64_t lineBB[64][64];
uint64_t rayBB[64][64];
uint64_t ray_pass[64][64];
uint64_t line_pass[64][64];

// Make sure the move isn't capture or promotion
bool isTactical(uint16_t move) {
    return getMoveCapture(move) || getMovePromote(move);
}


void copyBoard(board *p, struct copyposition *cp) {
    memcpy(cp->bitboards, p->bitboards, sizeof(cp->bitboards));
    memcpy(cp->occupancies, p->occupancies, sizeof(cp->occupancies));
    cp->hashKey = p->hashKey;
    cp->pawnKey = p->pawnKey;
    cp->minorKey = p->minorKey;
    cp->majorKey = p->majorKey;
    cp->whiteNonPawnKey = p->whiteNonPawnKey;
    cp->blackNonPawnKey = p->blackNonPawnKey;
    cp->krpKey = p->krpKey;
    memcpy(cp->mailbox, p->mailbox, sizeof(cp->mailbox));
    cp->side = p->side;
    cp->castle = p->castle;
    cp->enpassant = p->enpassant;
    cp->fifty = p->fifty;
    cp->full_moves = p->full_moves;
    cp->phase_score = p->phase_score;
    cp->pinned[0] = p->pinned[0];
    cp->pinned[1] = p->pinned[1];
    cp->pieceThreats = p->pieceThreats;
}

void takeBack(board *p, struct copyposition *cp) {
    memcpy(p->bitboards, cp->bitboards, sizeof(p->bitboards));
    memcpy(p->occupancies, cp->occupancies, sizeof(p->occupancies));
    p->hashKey = cp->hashKey;
    p->pawnKey = cp->pawnKey;
    p->minorKey = cp->minorKey;
    p->majorKey = cp->majorKey;
    p->whiteNonPawnKey = cp->whiteNonPawnKey;
    p->blackNonPawnKey = cp->blackNonPawnKey;
    p->krpKey = cp->krpKey;
    memcpy(p->mailbox, cp->mailbox, sizeof(p->mailbox));
    p->side = cp->side;
    p->castle = cp->castle;
    p->enpassant = cp->enpassant;
    p->fifty = cp->fifty;
    p->full_moves = cp->full_moves;
    p->phase_score = cp->phase_score;
    p->pinned[0] = cp->pinned[0];
    p->pinned[1] = cp->pinned[1];
    p->pieceThreats = cp->pieceThreats;
}

// add move to the move list
void addMove(moves *moveList, uint16_t move) {
    // store move
    moveList->moves[moveList->count] = move;
    // increment move count
    moveList->count++;
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

int isSquareAttacked(int square, int whichSide, board* pos) {
    if (pawnAttacks[whichSide == white ? black : white][square] & pos->bitboards[whichSide == white ? P : p]) {
        return 1;
    }
    if (knightAttacks[square] & pos->bitboards[whichSide == white ? N : n]) {
        return 1;
    }
    if (getRookAttacks(square, pos->occupancies[both]) & (pos->bitboards[whichSide == white ? R : r] | pos->bitboards[whichSide == white ? Q : q])) {
        return 1;
    }
    if (getBishopAttacks(square, pos->occupancies[both]) & (pos->bitboards[whichSide == white ? B : b] | pos->bitboards[whichSide == white ? Q : q])) {
        return 1;
    }
    if (kingAttacks[square] & pos->bitboards[whichSide == white ? K : k]) {
        return 1;
    }
    return 0;
}

U64 get_checkers(board* pos, uint8_t stm_king_square) {
    U64 checkers = 0;    

    checkers |= getPawnAttacks(pos->side, stm_king_square) & pos->bitboards[pos->side == white ? p : P];
    checkers |= getKnightAttacks(stm_king_square) & pos->bitboards[pos->side == white ? n : N];
    checkers |= getRookAttacks(stm_king_square, pos->occupancies[both]) & (pos->bitboards[pos->side == white ? r : R] | pos->bitboards[pos->side == white ? q : Q]);
    checkers |= getBishopAttacks(stm_king_square, pos->occupancies[both]) & (pos->bitboards[pos->side == white ? b : B] | pos->bitboards[pos->side == white ? q : Q]);    

    return checkers;
}

U64 attacked_bb(board *pos) {    
    U64 occ = pos->occupancies[both] ^ pos->bitboards[pos->side == white ? K : k];
    uint8_t king_square = getLS1BIndex(pos->bitboards[pos->side == white ? K : k]);
    uint8_t threat_side = pos->side == white ? black : white;

    U64 attacked_bb = get_threats_bb(threat_side, pos);
        

    return attacked_bb;
}

bool move_gives_check(uint16_t move, board* pos) {
    // attacker piece source square
    uint8_t sourceSquare = getMoveSource(move);
    // attacker piece target square
    uint8_t targetSquare = getMoveTarget(move);
    // attacker piece type
    uint8_t piece_type = getMovePromote(move) ? getMovePromotedPiece(pos->side, move) : pos->mailbox[sourceSquare];
    // opponent king square
    uint8_t opponent_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? k : K]);
    
    if (getMoveEnpassant(move)) {        
        if (pawnAttacks[pos->side][targetSquare] & (1ULL << opponent_king_square)) {            
            return true;
        }
    }

    if (getMoveCastling(move)) {
        switch (targetSquare) {
            // white castles king side
            case (g1):
                return getRookAttacks(f1, pos->occupancies[both]) & (1ULL << opponent_king_square);
                break;

            // white castles queen side
            case (c1):                
                return getRookAttacks(d1, pos->occupancies[both]) & (1ULL << opponent_king_square);
                break;

            // black castles king side
            case (g8):                
                return getRookAttacks(f8, pos->occupancies[both]) & (1ULL << opponent_king_square);
                break;

            // black castles queen side
            case (c8):                
                return getRookAttacks(d8, pos->occupancies[both]) & (1ULL << opponent_king_square);
                break;
        }        
    }

    if (piece_type == K || piece_type == k) return false; // King moves cannot give check

    switch (piece_type) {
        case P:
        case p:
            return pawnAttacks[pos->side][targetSquare] & (1ULL << opponent_king_square);
            break;
        case N:
        case n:
            return knightAttacks[targetSquare] & (1ULL << opponent_king_square);
            break;
        case B:
        case b:
            return getBishopAttacks(targetSquare, pos->occupancies[both]) & (1ULL << opponent_king_square);
            break;
        case R:
        case r:
            return getRookAttacks(targetSquare, pos->occupancies[both]) & (1ULL << opponent_king_square);
            break;
        case Q:
        case q:
            return (getBishopAttacks(targetSquare, pos->occupancies[both]) | getRookAttacks(targetSquare, pos->occupancies[both])) & (1ULL << opponent_king_square);
            break;
    }
        
    return false;
}

/*void update_check_squares(board* pos) {
    // Check for checks against the opponent's king
    int opponent_side = pos->side == white ? black : white;
    int king_square = getLS1BIndex(pos->bitboards[opponent_side == white ? K : k]);
    
    if (is_square_checked(king_square, pos->side, pos)) {
        pos->check_squares[0] |= (1ULL << king_square);
    }
}*/

bool isMinor(int piece) {
    return piece == K || piece == k || piece == B || piece == b || piece == N || piece == n;
}

bool isMajor(int piece) {
    return piece == Q || piece == q || piece == R || piece == r;
}

bool isKRP(int piece) {
    return piece == K || piece == k || piece == R || piece == r || piece == P || piece == p;
}

inline static void toggleHashesForPiece(board* position, int piece, int square) {
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

inline static void addPiece(board* position, int piece, int square) {    
    position->phase_score += get_piece_phase_score(piece);
    setBit(position->bitboards[piece], square);
    setBit(position->occupancies[pieceColor(piece)], square);
    setBit(position->occupancies[both], square);    
    position->mailbox[square] = piece;
    toggleHashesForPiece(position, piece, square);    
}

inline static void removePiece(board* position, int piece, int square) {
    assert(position->mailbox[square] == piece);    
    position->phase_score -= get_piece_phase_score(piece);
    popBit(position->bitboards[piece], square);
    popBit(position->occupancies[pieceColor(piece)], square);
    popBit(position->occupancies[both], square);    
    position->mailbox[square] = NO_PIECE;
    toggleHashesForPiece(position, piece, square);
    
}

bool is_pseudo_legal(uint16_t move, board *pos) {
    if (move == 0) return false; // no move
    uint16_t capture = getMoveCapture(move);
    uint16_t promote = getMovePromote(move);
    uint16_t castling = getMoveCastling(move);
    uint16_t double_push = getMoveDouble(move);    
    uint16_t enpassant = getMoveEnpassant(move);
    uint16_t quiet = !capture && !promote && !castling;

    uint16_t source_square = getMoveSource(move);
    uint16_t target_square = getMoveTarget(move);

    uint16_t piece = pos->mailbox[source_square];
    uint16_t target_piece = pos->mailbox[target_square];

    uint16_t piece_color = pieceColor(piece);
    int target_piece_color = target_piece != NO_PIECE ? pieceColor(target_piece) : -1;    

    // 1) the move must contain a piece
    // 2) the piece must belong to the side to move
    if (piece == NO_PIECE || piece_color != pos->side) {
        return false;
    }

    // 3) we can't capture our own pieces 
    if (target_piece != NO_PIECE && piece_color == target_piece_color) {
        return false;
    }

    // 4) we can't capture the king
    if (target_piece == K || target_piece == k) {
        return false;
    }

    // 5) if the move is enpassant, there must be an enpassant square available
    if (enpassant) {
        if (pos->enpassant == no_sq || target_square != pos->enpassant) return false;
        // enpassant can only be performed by pawns!
        if (pos->side == white ? piece != P : piece != p) return false;
    }

    // 6) if the move is capture then target piece must be available
    if (capture && !enpassant && target_piece == NO_PIECE) {
        return false;
    }

    // 7) if the move is double pawn push, there must be no piece on the target square and the square behind it
    if (double_push) {
        if (target_piece != NO_PIECE || (pos->side == white ? piece != P : piece != p)) return false;

        uint16_t behind_target_square = target_square + (pos->side == white ? 8 : -8);

        if (pos->mailbox[behind_target_square] != NO_PIECE) {
            return false;
        }

        uint16_t really_double_push = (pos->side == white ? target_square == source_square - 16 : target_square == source_square + 16);
        if (!really_double_push) {
            return false;
        }

        if (pos->side == black ? get_rank[source_square] != 6 : get_rank[source_square] != 1) {
            return false;
        }
    }

    // 8) handle quiet moves
    if (quiet && target_piece != NO_PIECE) {
        return false;
    }

    // 9) handle promotions
    if (promote) {
        if (pos->side == white ? piece != P : piece != p) return false;  // the piece must be a pawn
        // the source square must be on the 2nd or 7th rank
        if (pos->side == white ? get_rank[source_square] != 6 : get_rank[source_square] != 1) {            
            return false;
        }
        // a quiet promotion must land on an empty square
        if (!capture && target_piece != NO_PIECE) {
            return false;
        }
    }

    // 10) handle castling moves
    if (castling) {
        switch (target_square) {
            // white castles king side
            case g1:
                if (!(pos->castle & wk) // castling right must be available
                    || !(!(getBit(pos->occupancies[both], f1)) && !(getBit(pos->occupancies[both], g1))) // squares between king and rook must be empty
                    || !(!isSquareAttacked(e1, black, pos) && !isSquareAttacked(f1, black, pos) && !isSquareAttacked(g1, black, pos))) { // king, f1 and g1 squares must not be under attack
                    return false;
                }
                break;                
            // white castles queen side
            case c1:
                if (!(pos->castle & wq) // castling right must be available
                    || !(!(getBit(pos->occupancies[both], d1)) && !(getBit(pos->occupancies[both], c1))) // squares between king and rook must be empty
                    || !(!isSquareAttacked(e1, black, pos) && !isSquareAttacked(d1, black, pos) && !isSquareAttacked(c1, black, pos))) { // king, d1 and c1 squares must not be under attack
                    return false;
                }
                break;

            // black castles king side
            case g8:
                if (!(pos->castle & bk) // castling right must be available
                    || !(!(getBit(pos->occupancies[both], f8)) && !(getBit(pos->occupancies[both], g8))) // squares between king and rook must be empty
                    || !(!isSquareAttacked(e8, white, pos) && !isSquareAttacked(f8, white, pos) && !isSquareAttacked(g8, white, pos))) { // king, f8 and g8 squares must not be under attack
                    return false;
                }
                break;

            // black castles queen side
            case c8:
                if (!(pos->castle & bq) // castling right must be available
                    || !(!(getBit(pos->occupancies[both], d8)) && !(getBit(pos->occupancies[both], c8))) // squares between king and rook must be empty
                    || !(!isSquareAttacked(e8, white, pos) && !isSquareAttacked(d8, white, pos) && !isSquareAttacked(c8, white, pos))) { // king, d8 and c8 squares must not be under attack
                    return false;
                }
                break;
        }
        // if all checks pass, move is legal
        return true;
    }
    
    switch (piece) {
        case P:
        case p:            
            if (!promote && (pos->side == white ? get_rank[target_square] == 7 : get_rank[target_square] == 0)) {
                return false;
            }
            if (capture) {                
                if (target_square != source_square + (pos->side == white ? -7 : 9) && target_square != source_square + (pos->side == white ? -9 : 7)) {
                    return false;
                }
            } 
            else {
                int push_dir = (pos->side == white) ? -8 : 8;
                if (!double_push && target_square != source_square + push_dir) {
                    return false;
                }
            }
            
            break;
        case N:
        case n:
            if (!(knightAttacks[source_square] & (1ULL << target_square))) {
                return false;
            }
            break;
        case B:
        case b:
            if (!(getBishopAttacks(source_square, pos->occupancies[both]) & (1ULL << target_square))) {
                return false;
            }
            break;
        case R:
        case r:
            if (!(getRookAttacks(source_square, pos->occupancies[both]) & (1ULL << target_square))) {
                return false;
            }
            break;
        case Q:
        case q:
            if (!((getBishopAttacks(source_square, pos->occupancies[both]) | getRookAttacks(source_square, pos->occupancies[both])) & (1ULL << target_square))) {
                return false;
            }
            break;
        case K:
        case k:
            if (!(kingAttacks[source_square] & (1ULL << target_square))) {
                return false;
            }
            break;
    }
    
    /*if (capture && target_piece != NO_PIECE && piece_color == target_piece_color) {
        printBoard(pos);
        printf("the move is capture and target capture is friendly piece! ");
        printMove(move);
        printf("\n");
    }*/
    
    return true;
}

// castling
void generate_white_king_side_castling(board *position, moves *moveList) {
    // make sure square between king and king's rook are empty
    U64 w_short_castle_occupancy = position->occupancies[both] & w_short_castle_mask;
    // make sure the f1, g1 squares are not under attacks
    U64 w_short_castle_threats = position->pieceThreats.stmThreats[black] & w_short_castle_mask;

    // king side castling is available
    if (position->castle & wk && !w_short_castle_occupancy && !w_short_castle_threats) {        
        addMove(moveList, encodeMove(e1, g1, mf_castling));
    }
}

void generate_white_queen_side_castling(board *position, moves *moveList) {
    // make sure square between king and queen's rook are empty
    U64 w_long_castle_occupancy = position->occupancies[both] & w_long_castle_occupancy_mask;
    // make sure the d1, c1 squares are not under attacks
    U64 w_long_castle_threats = position->pieceThreats.stmThreats[black] & w_long_castle_threat_mask;

    // queen side castling is available
    if (position->castle & wq && !w_long_castle_occupancy && !w_long_castle_threats) {
        addMove(moveList, encodeMove(e1, c1, mf_castling));
    }
}

void generate_black_king_side_castling(board *position, moves *moveList) {
    // make sure square between king and king's rook are empty
    U64 b_short_castle_occupancy = position->occupancies[both] & b_short_castle_mask;
    // make sure the f8, g8 squares are not under attacks
    U64 b_short_castle_threats = position->pieceThreats.stmThreats[white] & b_short_castle_mask;

    // king side castling is available
    if (position->castle & bk && !b_short_castle_occupancy && !b_short_castle_threats) {
        addMove(moveList, encodeMove(e8, g8, mf_castling));
    }
}

void generate_black_queen_side_castling(board *position, moves *moveList) {
    // make sure square between king and queen's rook are empty
    U64 b_long_castle_occupancy = position->occupancies[both] & b_long_castle_occupancy_mask;
    // make sure the d8, c8 squares are not under attacks
    U64 b_long_castle_threats = position->pieceThreats.stmThreats[white] & b_long_castle_threat_mask;

    // queen side castling is available
    if (position->castle & bq && !b_long_castle_occupancy && !b_long_castle_threats) {
        addMove(moveList, encodeMove(e8, c8, mf_castling));
    }
}

// Filter out en passant captures that would expose the king to a horizontal
inline static U64 ep_pin_filter(const board *pos, U64 attackers, int king_sq, int captured_sq) {
    if (get_rank[king_sq] != get_rank[captured_sq]) return attackers;

    U64 enemy_rq = pos->side == white
        ? (pos->bitboards[r] | pos->bitboards[q])
        : (pos->bitboards[R] | pos->bitboards[Q]);
    // remove captured pawn + all attackers from occupancy
    U64 occ = pos->occupancies[both] ^ (1ULL << captured_sq) ^ attackers;

    return (getRookAttacks(king_sq, occ) & rankMasks[king_sq] & enemy_rq) ? 0 : attackers;
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

    __m512i extra0 = _mm512_set1_epi16(encodeMove(0, (unsigned)shift, capture ? mf_capture : mf_normal));
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
    __m512i extra = _mm512_set1_epi16(encodeMove(offset, (unsigned)(shift + offset), capture ? mf_capture : mf_normal));
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

void legal_make_move(uint16_t move, board* position) {    

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
        // en passant horizontal pin check is now handled in move generation

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

    // increment full moves counter
    position->full_moves += position->side == black;

    init_threats(position);
}

void legal_move_generator(moves *moveList, board* pos) {
    // init move list count
    moveList->count = 0;

    uint8_t stm_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? K : k]);    
    uint8_t enemy_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? k : K]);

    U64 piece, bitboard;
    U64 enemy = pos->occupancies[pos->side == white ? black : white];
    // remove enemy king from enemy bitboard
    popBit(enemy, enemy_king_square);

    U64 threats = pos->pieceThreats.stmThreats[!pos->side];    
    U64 blockers = pos->occupancies[both];
    U64 friendly = pos->occupancies[pos->side];
    U64 empty = ~blockers;    

    U64 checkers = get_checkers(pos, stm_king_square);

    uint8_t checker_count = countBits(checkers);
    uint8_t checker_square = checker_count == 1 ? getLS1BIndex(checkers) : 0;


    // in double check we can't create any move except king moves
    if (checker_count >= 2) {
        // King moves
        piece = pos->side == white ? K : k;
        bitboard = pos->bitboards[piece];
        U64 targetBitboard = kingAttacks[stm_king_square] & ~threats;

        splatNormalMoves(moveList, stm_king_square, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, stm_king_square, targetBitboard & enemy, mf_capture);

        // return because we can't create any move except king moves
        return;
    }

    U64 evasion_mask = checker_count == 1 ? (1ULL << checker_square) | lineBB[stm_king_square][checker_square] | rayBB[stm_king_square][checker_square] : ~0ULL;
    update_pinned(pos);
    U64 pinned = pos->pinned[pos->side];


    // Pawn moves
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & ~pinned;

        U64 emptyAhead = bitboard & (empty << 8);
        // evasion_mask is in target-square space; shift to source-square space
        U64 singlePush = emptyAhead & 0x00FFFFFFFFFF0000 & (evasion_mask << 8);
        U64 doublePush = emptyAhead & 0x00FF000000000000 & (empty << 16) & (evasion_mask << 16);
        U64 promotions = emptyAhead & 0x000000000000FF00 & (evasion_mask << 8);

        splatPawnSingleMoves(moveList, singlePush, -8, 0);
        splatPawnDoubleMoves(moveList, doublePush, -16, white);
        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 9);
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 7);
        U64 lPromotions = lEnemy & 0x000000000000FF00 & (evasion_mask << 9);
        U64 rPromotions = rEnemy & 0x000000000000FF00 & (evasion_mask << 7);

        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);

        if (pos->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[black][pos->enpassant];
            int capturedPawnSq = pos->enpassant + 8; // the pawn being captured
            // In check: ep is legal if the ep target square is in evasion_mask,
            // OR if the captured pawn IS the checker (checker_square = ep_target ± 8)
            if (checker_count == 1) {
                if (!((1ULL << pos->enpassant) & evasion_mask) && capturedPawnSq != checker_square)
                    attackers = 0;
            }
            // Filter out horizontal clearance pins
            attackers = ep_pin_filter(pos, attackers, stm_king_square, capturedPawnSq);
            splatEnpassant(moveList, attackers, pos->enpassant);
        }
    } else {
        bitboard = pos->bitboards[p] & ~pinned;

        U64 emptyAhead = bitboard & (empty >> 8);
        U64 singlePush = emptyAhead & 0x0000FFFFFFFFFF00 & (evasion_mask >> 8);
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16) & (evasion_mask >> 16);
        U64 promotions = emptyAhead & 0x00FF000000000000 & (evasion_mask >> 8);

        splatPawnSingleMoves(moveList, singlePush, +8, 0);
        splatPawnDoubleMoves(moveList, doublePush, +16, black);
        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 7);
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 9);
        U64 lPromotions = lEnemy & 0x00FF000000000000 & (evasion_mask >> 7);
        U64 rPromotions = rEnemy & 0x00FF000000000000 & (evasion_mask >> 9);

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);

        if (pos->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[white][pos->enpassant];
            int capturedPawnSq = pos->enpassant - 8; // the pawn being captured
            // In check: ep is legal if the ep target square is in evasion_mask,
            // OR if the captured pawn IS the checker (checker_square = ep_target - 8)
            if (checker_count == 1) {
                if (!((1ULL << pos->enpassant) & evasion_mask) && capturedPawnSq != checker_square)
                    attackers = 0;
            }
            // Filter out horizontal clearance pins
            attackers = ep_pin_filter(pos, attackers, stm_king_square, capturedPawnSq);
            splatEnpassant(moveList, attackers, pos->enpassant);
        }
    }    

    /* UNPINNED PIECE MOVEMENTS */

    // Knight moves
    piece = pos->side == white ? N : n;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = knightAttacks[sourceSquare] & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

     // King moves
    piece = pos->side == white ? K : k;
    bitboard = pos->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = kingAttacks[sourceSquare] & ~threats;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    /* PINNED PIECE MOVEMENTS */

    // Pinned Bishop Moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Pinned Rook Moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Pinned Queen Moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);
        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }


    // Pinned Pawn Moves
    U64 kingFileBB = 0x0101010101010101ULL << (stm_king_square % 8);
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & pinned;
        U64 emptyAhead = bitboard & (empty << 8) & kingFileBB & (evasion_mask << 8);
        U64 single_push = emptyAhead;
        U64 doublePush = emptyAhead & 0x00FF000000000000 & (empty << 16) & (evasion_mask << 16);
        U64 promotions = emptyAhead & 0x000000000000FF00 & (evasion_mask << 8);

        splatPawnSingleMoves(moveList, single_push, NORTH, 0);
        splatPawnDoubleMoves(moveList, doublePush, -16, white);
        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 9) & king_anti_diag_mask[1][stm_king_square];
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 7) & king_anti_diag_mask[0][stm_king_square];
        U64 lPromotions = lEnemy & 0x000000000000FF00 & (evasion_mask << 9) & king_anti_diag_mask[1][stm_king_square];
        U64 rPromotions = rEnemy & 0x000000000000FF00 & (evasion_mask << 7) & king_anti_diag_mask[0][stm_king_square];

        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);


    } 
    else {
        bitboard = pos->bitboards[p] & pinned;
        U64 emptyAhead = bitboard & (empty >> 8) & kingFileBB & (evasion_mask >> 8);
        U64 single_push = emptyAhead;
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16) & (evasion_mask >> 16);
        U64 promotions = emptyAhead & 0x00FF000000000000 & (evasion_mask >> 8);

        splatPawnSingleMoves(moveList, single_push, SOUTH, 0);
        splatPawnDoubleMoves(moveList, doublePush, +16, black);
        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 7) & king_anti_diag_mask[0][stm_king_square];
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 9) & king_anti_diag_mask[1][stm_king_square];
        U64 lPromotions = lEnemy & 0x00FF000000000000 & (evasion_mask >> 7) & king_anti_diag_mask[0][stm_king_square];
        U64 rPromotions = rEnemy & 0x00FF000000000000 & (evasion_mask >> 9) & king_anti_diag_mask[1][stm_king_square];

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);
    }

    // Castling moves
    if (checker_count == 0) { 
        if (pos->side == white) {
            generate_white_king_side_castling(pos, moveList);
            generate_white_queen_side_castling(pos, moveList);
        } else {
            generate_black_king_side_castling(pos, moveList);
            generate_black_queen_side_castling(pos, moveList);
        }
    }
}

void legal_noisy_generator(moves *moveList, board* pos) {
    // init move list count
    moveList->count = 0;    

    uint8_t stm_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? K : k]);
    uint8_t enemy_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? k : K]);

    U64 piece, bitboard;
    U64 enemy = pos->occupancies[pos->side == white ? black : white];
    // remove enemy king from enemy bitboard
    popBit(enemy, enemy_king_square);

    U64 threats = pos->pieceThreats.stmThreats[!pos->side];    
    U64 blockers = pos->occupancies[both];
    U64 friendly = pos->occupancies[pos->side];
    U64 empty = ~blockers;



    U64 checkers = get_checkers(pos, stm_king_square);

    uint8_t checker_count = countBits(checkers);
    uint8_t checker_square = checker_count == 1 ? getLS1BIndex(checkers) : 0;


    // in double check we can't create any move except king moves
    if (checker_count >= 2) {
        // King moves
        piece = pos->side == white ? K : k;
        bitboard = pos->bitboards[piece];
        U64 targetBitboard = kingAttacks[stm_king_square] & ~threats;

        splatNormalMoves(moveList, stm_king_square, targetBitboard & enemy, mf_capture);

        // return because we can't create any move except king moves
        return;
    }

    U64 evasion_mask = checker_count == 1 ? (1ULL << checker_square) | lineBB[stm_king_square][checker_square] | rayBB[stm_king_square][checker_square] : ~0ULL;
    update_pinned(pos);
    U64 pinned = pos->pinned[pos->side];


    // Pawn moves
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & ~pinned;

        U64 emptyAhead = bitboard & (empty << 8);
        // evasion_mask is in target-square space; shift to source-square space        
        U64 promotions = emptyAhead & 0x000000000000FF00 & (evasion_mask << 8);

        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 9);
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 7);
        U64 lPromotions = lEnemy & 0x000000000000FF00 & (evasion_mask << 9);
        U64 rPromotions = rEnemy & 0x000000000000FF00 & (evasion_mask << 7);

        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);

        if (pos->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[black][pos->enpassant];
            int capturedPawnSq = pos->enpassant + 8; // the pawn being captured
            // In check: ep is legal if the ep target square is in evasion_mask,
            // OR if the captured pawn IS the checker (checker_square = ep_target ± 8)
            if (checker_count == 1) {
                if (!((1ULL << pos->enpassant) & evasion_mask) && capturedPawnSq != checker_square)
                    attackers = 0;
            }
            // Filter out horizontal clearance pins
            attackers = ep_pin_filter(pos, attackers, stm_king_square, capturedPawnSq);
            splatEnpassant(moveList, attackers, pos->enpassant);
        }
    } else {
        bitboard = pos->bitboards[p] & ~pinned;

        U64 emptyAhead = bitboard & (empty >> 8);        
        U64 promotions = emptyAhead & 0x00FF000000000000 & (evasion_mask >> 8);

        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 7);
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 9);
        U64 lPromotions = lEnemy & 0x00FF000000000000 & (evasion_mask >> 7);
        U64 rPromotions = rEnemy & 0x00FF000000000000 & (evasion_mask >> 9);

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);

        if (pos->enpassant != no_sq) {
            U64 attackers = bitboard & pawnAttacks[white][pos->enpassant];
            int capturedPawnSq = pos->enpassant - 8; // the pawn being captured
            // In check: ep is legal if the ep target square is in evasion_mask,
            // OR if the captured pawn IS the checker (checker_square = ep_target - 8)
            if (checker_count == 1) {
                if (!((1ULL << pos->enpassant) & evasion_mask) && capturedPawnSq != checker_square)
                    attackers = 0;
            }
            // Filter out horizontal clearance pins
            attackers = ep_pin_filter(pos, attackers, stm_king_square, capturedPawnSq);
            splatEnpassant(moveList, attackers, pos->enpassant);
        }
    }    

    /* UNPINNED PIECE MOVEMENTS */

    // Knight moves
    piece = pos->side == white ? N : n;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);
        U64 targetBitboard = knightAttacks[sourceSquare] & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);
        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);
        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);
        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

     // King moves
    piece = pos->side == white ? K : k;
    bitboard = pos->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);
        U64 targetBitboard = kingAttacks[sourceSquare] & ~threats;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    /* PINNED PIECE MOVEMENTS */

    // Pinned Bishop Moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Pinned Rook Moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }

    // Pinned Queen Moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & enemy, mf_capture);

        popBit(bitboard, sourceSquare);
    }


    // Pinned Pawn Moves
    U64 kingFileBB = 0x0101010101010101ULL << (stm_king_square % 8);
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & pinned;
        U64 emptyAhead = bitboard & (empty << 8) & kingFileBB & (evasion_mask << 8);        
        U64 promotions = emptyAhead & 0x000000000000FF00 & (evasion_mask << 8);

        splatPawnPromoMoves(moveList, promotions, -8, white, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy << 9);
        U64 rEnemy = bitboard & not_h_file & (enemy << 7);
        U64 lSingleCapt = lEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 9) & king_anti_diag_mask[1][stm_king_square];
        U64 rSingleCapt = rEnemy & 0x00FFFFFFFFFF0000 & (evasion_mask << 7) & king_anti_diag_mask[0][stm_king_square];
        U64 lPromotions = lEnemy & 0x000000000000FF00 & (evasion_mask << 9) & king_anti_diag_mask[1][stm_king_square];
        U64 rPromotions = rEnemy & 0x000000000000FF00 & (evasion_mask << 7) & king_anti_diag_mask[0][stm_king_square];

        
        splatPawnSingleMoves(moveList, lSingleCapt, -9, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, -7, 1);
        splatPawnPromoMoves(moveList, lPromotions, -9, white, 1);
        splatPawnPromoMoves(moveList, rPromotions, -7, white, 1);


    } 
    else {
        bitboard = pos->bitboards[p] & pinned;
        U64 emptyAhead = bitboard & (empty >> 8) & kingFileBB & (evasion_mask >> 8);        
        U64 promotions = emptyAhead & 0x00FF000000000000 & (evasion_mask >> 8);

        splatPawnPromoMoves(moveList, promotions, +8, black, 0);

        U64 lEnemy = bitboard & not_a_file & (enemy >> 7);
        U64 rEnemy = bitboard & not_h_file & (enemy >> 9);
        U64 lSingleCapt = lEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 7) & king_anti_diag_mask[0][stm_king_square];
        U64 rSingleCapt = rEnemy & 0x0000FFFFFFFFFF00 & (evasion_mask >> 9) & king_anti_diag_mask[1][stm_king_square];
        U64 lPromotions = lEnemy & 0x00FF000000000000 & (evasion_mask >> 7) & king_anti_diag_mask[0][stm_king_square];
        U64 rPromotions = rEnemy & 0x00FF000000000000 & (evasion_mask >> 9) & king_anti_diag_mask[1][stm_king_square];

        splatPawnSingleMoves(moveList, lSingleCapt, +7, 1);
        splatPawnSingleMoves(moveList, rSingleCapt, +9, 1);
        splatPawnPromoMoves(moveList, lPromotions, +7, black, 1);
        splatPawnPromoMoves(moveList, rPromotions, +9, black, 1);
    }
}

void legal_quiet_generator(moves *moveList, board* pos) {
    // init move list count
    moveList->count = 0;    

    U64 piece, bitboard;
    U64 enemy = pos->occupancies[pos->side == white ? black : white];
    U64 threats = pos->pieceThreats.stmThreats[!pos->side];    
    U64 blockers = pos->occupancies[both];
    U64 friendly = pos->occupancies[pos->side];
    U64 empty = ~blockers;

    uint8_t stm_king_square = getLS1BIndex(pos->bitboards[pos->side == white ? K : k]);    

    U64 checkers = get_checkers(pos, stm_king_square);

    uint8_t checker_count = countBits(checkers);
    uint8_t checker_square = checker_count == 1 ? getLS1BIndex(checkers) : 0;


    // in double check we can't create any move except king moves
    if (checker_count >= 2) {
        // King moves
        piece = pos->side == white ? K : k;
        bitboard = pos->bitboards[piece];
        U64 targetBitboard = kingAttacks[stm_king_square] & ~threats;

        splatNormalMoves(moveList, stm_king_square, targetBitboard & empty, mf_normal);        

        // return because we can't create any move except king moves
        return;
    }

    U64 evasion_mask = checker_count == 1 ? (1ULL << checker_square) | lineBB[stm_king_square][checker_square] | rayBB[stm_king_square][checker_square] : ~0ULL;
    update_pinned(pos);
    U64 pinned = pos->pinned[pos->side];


    // Pawn moves
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & ~pinned;

        U64 emptyAhead = bitboard & (empty << 8);
        // evasion_mask is in target-square space; shift to source-square space
        U64 singlePush = emptyAhead & 0x00FFFFFFFFFF0000 & (evasion_mask << 8);
        U64 doublePush = emptyAhead & 0x00FF000000000000 & (empty << 16) & (evasion_mask << 16);    

        splatPawnSingleMoves(moveList, singlePush, -8, 0);
        splatPawnDoubleMoves(moveList, doublePush, -16, white);

    } else {
        bitboard = pos->bitboards[p] & ~pinned;

        U64 emptyAhead = bitboard & (empty >> 8);
        U64 singlePush = emptyAhead & 0x0000FFFFFFFFFF00 & (evasion_mask >> 8);
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16) & (evasion_mask >> 16);        

        splatPawnSingleMoves(moveList, singlePush, +8, 0);
        splatPawnDoubleMoves(moveList, doublePush, +16, black);                
    }    

    /* UNPINNED PIECE MOVEMENTS */

    // Knight moves
    piece = pos->side == white ? N : n;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = knightAttacks[sourceSquare] & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    // Bishop moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    // Rook moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    // Queen moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & ~pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

     // King moves
    piece = pos->side == white ? K : k;
    bitboard = pos->bitboards[piece];
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);

        U64 targetBitboard = kingAttacks[sourceSquare] & ~threats;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    /* PINNED PIECE MOVEMENTS */

    // Pinned Bishop Moves
    piece = pos->side == white ? B : b;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getBishopAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    // Pinned Rook Moves
    piece = pos->side == white ? R : r;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getRookAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }

    // Pinned Queen Moves
    piece = pos->side == white ? Q : q;
    bitboard = pos->bitboards[piece] & pinned;
    while (bitboard) {
        int sourceSquare = getLS1BIndex(bitboard);        
        U64 pinLine = ray_pass[stm_king_square][sourceSquare];
        U64 targetBitboard = getQueenAttacks(sourceSquare, blockers) & pinLine & ~friendly & evasion_mask;

        splatNormalMoves(moveList, sourceSquare, targetBitboard & empty, mf_normal);        

        popBit(bitboard, sourceSquare);
    }


    // Pinned Pawn Moves
    U64 kingFileBB = 0x0101010101010101ULL << (stm_king_square % 8);
    if (pos->side == white) {
        bitboard = pos->bitboards[P] & pinned;
        U64 emptyAhead = bitboard & (empty << 8) & kingFileBB & (evasion_mask << 8);
        U64 single_push = emptyAhead;
        U64 doublePush = emptyAhead & 0x00FF000000000000 & (empty << 16) & (evasion_mask << 16);        

        splatPawnSingleMoves(moveList, single_push, NORTH, 0);
        splatPawnDoubleMoves(moveList, doublePush, -16, white);        

    } 
    else {
        bitboard = pos->bitboards[p] & pinned;
        U64 emptyAhead = bitboard & (empty >> 8) & kingFileBB & (evasion_mask >> 8);
        U64 single_push = emptyAhead;
        U64 doublePush = emptyAhead & 0x000000000000FF00 & (empty >> 16) & (evasion_mask >> 16);        

        splatPawnSingleMoves(moveList, single_push, SOUTH, 0);
        splatPawnDoubleMoves(moveList, doublePush, +16, black);        
    }

    // Castling moves
    if (checker_count == 0) { 
        if (pos->side == white) {
            generate_white_king_side_castling(pos, moveList);
            generate_white_queen_side_castling(pos, moveList);
        } else {
            generate_black_king_side_castling(pos, moveList);
            generate_black_queen_side_castling(pos, moveList);
        }
    }
}

void initSlidersAttacks(int bishop) {
    for (int square = 0; square < 64; square++) {
        bishopMask[square] = maskBishopAttacks(square);
        rookMask[square] = maskRookAttacks(square);

        U64 attackMask = bishop ? bishopMask[square] : rookMask[square];
        int relevantBitsCount = countBits(attackMask);
        int occupancyIndices = 1 << relevantBitsCount;

        for (int index = 0; index < occupancyIndices; index++) {
            U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

            if (bishop) {
                // Traditional Magics
                int magicIndex = (occupancy * bishopMagic[square]) >> (64 - bishopRelevantBits[square]);
                bishopAttacks[square][magicIndex] = bishopAttack(square, occupancy);

                // PEXT Tables
                #if defined(__BMI2__)                
                bishopAttacksPEXT[square][index] = bishopAttacks[square][magicIndex];
                #endif
            }
            else {
                // Traditional Magics
                int magicIndex = (occupancy * rookMagic[square]) >> (64 - rookRelevantBits[square]);
                rookAttacks[square][magicIndex] = rookAttack(square, occupancy);

                // PEXT Tables
                #if defined(__BMI2__)
                rookAttacksPEXT[square][index] = rookAttacks[square][magicIndex];
                #endif
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

// print move
void printMove(uint16_t move) {
    if (getMovePromote(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromotedPiece(black, move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }
}

