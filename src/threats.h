#include "board_constants.h"

#define WHITE_TI_SIZE_PAWN   336
#define WHITE_TI_SIZE_KNIGHT 3360
#define WHITE_TI_SIZE_BISHOP 4480
#define WHITE_TI_SIZE_ROOK   7168
#define WHITE_TI_SIZE_QUEEN  14560

#define BLACK_TI_SIZE 29904

#define THREAT_INPUTS (WHITE_TI_SIZE_PAWN + WHITE_TI_SIZE_KNIGHT + WHITE_TI_SIZE_BISHOP + \
                       WHITE_TI_SIZE_ROOK + WHITE_TI_SIZE_QUEEN + BLACK_TI_SIZE)

#define TI_OFFSET_WHITE_PAWN   0
#define TI_OFFSET_WHITE_KNIGHT 336
#define TI_OFFSET_WHITE_BISHOP 3696
#define TI_OFFSET_WHITE_ROOK   8176
#define TI_OFFSET_WHITE_QUEEN  15344





int8_t get_white_pawn_target_id(uint8_t target_piece);
int8_t get_white_knight_target_id(uint8_t target_piece);
int8_t get_white_bishop_target_id(uint8_t target_piece);
int8_t get_white_rook_target_id(uint8_t target_piece);
int8_t get_white_queen_target_id(uint8_t target_piece);
