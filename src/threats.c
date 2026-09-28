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
