#ifndef POTENTIAL_THREAT_GEO_H
#define POTENTIAL_THREAT_GEO_H

#include <stdint.h>
#include "threats.h"

extern int pawn_geo[64][64];
extern int knight_geo[64][64];
extern int bishop_geo[64][64];
extern int rook_geo[64][64];
extern int queen_geo[64][64];

extern const int8_t ti_target_ids[5][12];
extern const int ti_max_geo[5];
extern const int ti_type_offset[5];
extern const int (*const ti_geo[5])[64];

void init_threat_geometry();

#endif // POTENTIAL_THREAT_GEO_H