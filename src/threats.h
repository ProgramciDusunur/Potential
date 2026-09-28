#include "board_constants.h"

// Threat Input Features
#define WHITE_TI_SIZE_PAWN   504
#define WHITE_TI_SIZE_KNIGHT 3360
#define WHITE_TI_SIZE_BISHOP 4480
#define WHITE_TI_SIZE_ROOK   7168
#define WHITE_TI_SIZE_QUEEN  14560

// Same as the sum of the white pieces like above, but for black pieces
#define BLACK_TI_SIZE 30072

#define THREAT_INPUTS (WHITE_TI_SIZE_PAWN + WHITE_TI_SIZE_KNIGHT + WHITE_TI_SIZE_BISHOP + \
                       WHITE_TI_SIZE_ROOK + WHITE_TI_SIZE_QUEEN + BLACK_TI_SIZE)

#define TI_OFFSET_WHITE_PAWN   0
#define TI_OFFSET_WHITE_KNIGHT 504
#define TI_OFFSET_WHITE_BISHOP 3864
#define TI_OFFSET_WHITE_ROOK   8344
#define TI_OFFSET_WHITE_QUEEN  15512





int8_t get_white_pawn_target_id(uint8_t target_piece);
int8_t get_white_knight_target_id(uint8_t target_piece);
int8_t get_white_bishop_target_id(uint8_t target_piece);
int8_t get_white_rook_target_id(uint8_t target_piece);
int8_t get_white_queen_target_id(uint8_t target_piece);
