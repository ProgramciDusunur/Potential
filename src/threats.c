#include "threats.h"

int8_t get_white_pawn_target_id(uint8_t target_piece) {
    // Friendly Pieces
    if (target_piece == P) return 0;
    if (target_piece == N) return 1;
    if (target_piece == R) return 2;

    // Enemy Pieces
    if (target_piece == p) return 3;
    if (target_piece == n) return 4;
    if (target_piece == r) return 5;

    // Invalid target (Bishop, Queen, King, or empty square)
    return -1; 
}

int8_t get_white_knight_target_id(uint8_t target_piece) {
    // Friendly Pieces
    if (target_piece == P) return 0;
    if (target_piece == N) return 1;
    if (target_piece == B) return 2;
    if (target_piece == R) return 3;
    if (target_piece == Q) return 4;

    // Enemy Pieces
    if (target_piece == p) return 5;
    if (target_piece == n) return 6;
    if (target_piece == b) return 7;
    if (target_piece == r) return 8;
    if (target_piece == q) return 9;

    // Invalid target (King, or empty square)
    return -1; 
}

int8_t get_white_bishop_target_id(uint8_t target_piece) {
    // Friendly Pieces
    if (target_piece == P) return 0;
    if (target_piece == N) return 1;
    if (target_piece == B) return 2;
    if (target_piece == R) return 3;

    // Enemy Pieces
    if (target_piece == p) return 4;
    if (target_piece == n) return 5;
    if (target_piece == b) return 6;
    if (target_piece == r) return 7;

    // Invalid target (Queen, King, or empty square)
    return -1; 
}

int8_t get_white_rook_target_id(uint8_t target_piece) {
    // Friendly Pieces
    if (target_piece == P) return 0;
    if (target_piece == N) return 1;
    if (target_piece == B) return 2;
    if (target_piece == R) return 3;    

    // Enemy Pieces
    if (target_piece == p) return 4;
    if (target_piece == n) return 5;
    if (target_piece == b) return 6;
    if (target_piece == r) return 7;    

    // Invalid target (Queen, King, or empty square)
    return -1; 
}

int8_t get_white_queen_target_id(uint8_t target_piece) {
    // Friendly Pieces
    if (target_piece == P) return 0;
    if (target_piece == N) return 1;
    if (target_piece == B) return 2;
    if (target_piece == R) return 3;
    if (target_piece == Q) return 4;

    // Enemy Pieces
    if (target_piece == p) return 5;
    if (target_piece == n) return 6;
    if (target_piece == b) return 7;
    if (target_piece == r) return 8;
    if (target_piece == q) return 9;

    // Invalid target (King, or empty square)
    return -1; 
}

U64 get_threat_feature_index(int8_t piece, int8_t target_piece) {
    // feature_index = color_offset + type_offset + (target_id × max_geo) + geo

    // Every color and piece type has their own offset in the feature matrix
    int16_t color_offset = (piece >= 6) ? BLACK_TI_SIZE : 0;
    int16_t type_offset = ti_type_offset[piece % 6];
    int w_target_id = ti_target_ids[base_piece][w_rel_target];
    int8_t target_id = 0;

    U64 feature_index = color_offset + type_offset;
}

// TODO: this is can be done as pre-computed table (my guess xd)
U64 get_piece_attacks(int piece, int square, U64 occ) {
    switch(piece) {
        case P: return getPawnAttacks(0, square);
        case p: return getPawnAttacks(1, square);
        case N: case n: return getKnightAttacks(square);
        case B: case b: return getBishopAttacks(square, occ);
        case R: case r: return getRookAttacks(square, occ);
        case Q: case q: return getQueenAttacks(square, occ);
        default: return 0;
    }
}

void remove_piece_threats(int8_t piece, int8_ t square, U64 occ) {
    U64 attacks = get_piece_attacks(piece, square, occ);
    
    

}

void add_piece_threats() {

}
