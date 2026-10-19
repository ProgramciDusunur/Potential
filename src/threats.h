#ifndef THREATS_H
#define THREATS_H

#include <stdint.h>
#include <stdbool.h>
#include "board_constants.h"
#include "structs.h"

// Threat Input Features
#define WHITE_TI_SIZE_PAWN   504
#define WHITE_TI_SIZE_KNIGHT 3360
#define WHITE_TI_SIZE_BISHOP 4480
#define WHITE_TI_SIZE_ROOK   7168
#define WHITE_TI_SIZE_QUEEN  14560

#define BLACK_TI_SIZE 30072

#define THREAT_INPUTS (WHITE_TI_SIZE_PAWN + WHITE_TI_SIZE_KNIGHT + WHITE_TI_SIZE_BISHOP + \
                       WHITE_TI_SIZE_ROOK + WHITE_TI_SIZE_QUEEN + BLACK_TI_SIZE)

#define TI_OFFSET_WHITE_PAWN   0
#define TI_OFFSET_WHITE_KNIGHT 504
#define TI_OFFSET_WHITE_BISHOP 3864
#define TI_OFFSET_WHITE_ROOK   8344
#define TI_OFFSET_WHITE_QUEEN  15512

#define MAX_THREAT_UPDATES 256

typedef struct {
    int white_adds[MAX_THREAT_UPDATES];
    int white_subs[MAX_THREAT_UPDATES];
    int black_adds[MAX_THREAT_UPDATES];
    int black_subs[MAX_THREAT_UPDATES];
    int num_white_adds;
    int num_white_subs;
    int num_black_adds;
    int num_black_subs;
} ThreatUpdates;

static inline void init_threat_updates(ThreatUpdates *u) {
    u->num_white_adds = 0;
    u->num_white_subs = 0;
    u->num_black_adds = 0;
    u->num_black_subs = 0;
}

int get_threat_feature(int side, int attacker, int target, int from_sq, int to_sq, int ksq);

void record_threat_pair(ThreatUpdates *u, int attacker, int target, int from_sq, int to_sq,
                        int w_ksq, int b_ksq, bool is_add);

void update_piece_threats_buffer(ThreatUpdates *u, board *pos, int piece, int sq,
                                 bool is_add, bool discovery, U64 discoveryMask,
                                 int blocked_sq);

void init_threat_sliders(void);

void apply_threat_updates(board *pos, int prev_ply, int cur_ply, const ThreatUpdates *u);

#endif // THREATS_H