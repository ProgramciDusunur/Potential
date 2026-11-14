#include "structs.h"

// macros for converting viri move

#define viri_from(vf_move) ((vf_move) & 0x3F)
#define viri_to(vf_move) (((vf_move) >> 6) & 0x3F)
#define viri_prom(vf_move) (((vf_move) >> 12) & 0x03)
#define viri_move_type(vf_move) ((vf_move & 0xC000))

enum {
    VIRI_NORMAL = 0x0000,
    VIRI_EN_PASSANT = 0x4000,
    VIRI_CASTLE = 0x8000,
    VIRI_PROMOTION = 0xC000
};