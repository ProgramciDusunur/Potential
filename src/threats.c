#include "threats.h"
#include "threat_geo.h"
#include "bit_manipulation.h"
#include "move.h"
#include "nnue.h"
#include "simd.h"

#define TILE_VECS 12

extern uint64_t lineBB[64][64];
extern uint64_t rayBB[64][64];
extern uint64_t ray_pass[64][64];

int get_threat_feature(int side, int attacker, int target, int from_sq, int to_sq, int ksq) {
    int rel_attacker, rel_target, flip_mask, offset;

    if (side == white) {
        rel_attacker = attacker;
        rel_target   = target;
        flip_mask    = ((ksq & 7) > 3) ? 7 : 0;
        offset       = (attacker >= 6) ? BLACK_TI_SIZE : 0;
    } else {
        rel_attacker = (attacker >= 6) ? (attacker - 6) : (attacker + 6);
        rel_target   = (target >= 6)   ? (target - 6)   : (target + 6);
        flip_mask    = (((ksq & 7) > 3) ? 7 : 0) ^ 56;
        offset       = (attacker < 6)  ? BLACK_TI_SIZE : 0;
    }

    int base_piece = rel_attacker % 6;
    if (base_piece == 5) return -1; // Kings cannot be threat attackers

    int target_id = ti_target_ids[base_piece][rel_target];
    if (target_id < 0) return -1;   // Invalid target or King

    int mapped_from   = from_sq ^ flip_mask;
    int mapped_target = to_sq   ^ flip_mask;

    // Symmetry deduplication for identical non-pawn piece types
    if (base_piece > 0 && ((rel_target % 6) == base_piece) &&
        ((mapped_target ^ 56) > (mapped_from ^ 56))) {
        return -1;
    }

    int geo = ti_geo[base_piece][mapped_from][mapped_target];
    if (geo < 0) return -1;

    return offset + ti_type_offset[base_piece] + (target_id * ti_max_geo[base_piece]) + geo;
}

void record_threat_pair(ThreatUpdates *u, int attacker, int target, int from_sq, int to_sq,
                        int w_ksq, int b_ksq, bool is_add) {
    int wf = get_threat_feature(white, attacker, target, from_sq, to_sq, w_ksq);
    if (wf >= 0) {
        if (is_add) u->white_adds[u->num_white_adds++] = wf;
        else        u->white_subs[u->num_white_subs++] = wf;
    }

    int bf = get_threat_feature(black, attacker, target, from_sq, to_sq, b_ksq);
    if (bf >= 0) {
        if (is_add) u->black_adds[u->num_black_adds++] = bf;
        else        u->black_subs[u->num_black_subs++] = bf;
    }
}

static U64 pseudo_rook_rays[64];
static U64 pseudo_bishop_rays[64];

void init_threat_sliders(void) {
    for (int sq = 0; sq < 64; sq++) {
        pseudo_rook_rays[sq]   = getRookAttacks(sq, 0ULL);
        pseudo_bishop_rays[sq] = getBishopAttacks(sq, 0ULL);
    }
}

void update_piece_threats_buffer(ThreatUpdates *u, board *pos, int piece, int sq,
                                 bool is_add, bool discovery, U64 discoveryMask,
                                 int blocked_sq) {
    U64 occ   = pos->occupancies[both];
    U64 kings = pos->bitboards[K] | pos->bitboards[k];
    U64 valid_targets = occ & ~kings;

    int w_ksq = getLS1BIndex(pos->bitboards[K]);
    int b_ksq = getLS1BIndex(pos->bitboards[k]);

    U64 rooks_queens   = pos->bitboards[R] | pos->bitboards[r] | pos->bitboards[Q] | pos->bitboards[q];
    U64 bishops_queens = pos->bitboards[B] | pos->bitboards[b] | pos->bitboards[Q] | pos->bitboards[q];

    U64 r_att = 0ULL;
    U64 b_att = 0ULL;
    bool r_computed = false;
    bool b_computed = false;

    // 1. Outgoing threats from 'piece' at 'sq' (if non-King)
    if (piece != K && piece != k) {
        U64 outgoing = 0ULL;
        switch (piece) {
            case P: outgoing = getPawnAttacks(white, sq); break;
            case p: outgoing = getPawnAttacks(black, sq); break;
            case N: case n: outgoing = getKnightAttacks(sq); break;
            case B: case b:
                b_att = getBishopAttacks(sq, occ);
                b_computed = true;
                outgoing = b_att;
                break;
            case R: case r:
                r_att = getRookAttacks(sq, occ);
                r_computed = true;
                outgoing = r_att;
                break;
            case Q: case q:
                r_att = getRookAttacks(sq, occ);
                b_att = getBishopAttacks(sq, occ);
                r_computed = true;
                b_computed = true;
                outgoing = r_att | b_att;
                break;
        }
        outgoing &= valid_targets;
        while (outgoing) {
            int to_sq = getLS1BIndex(outgoing);
            popBit(outgoing, to_sq);

            int target_piece = pos->mailbox[to_sq];
            record_threat_pair(u, piece, target_piece, sq, to_sq, w_ksq, b_ksq, is_add);
        }
    }

    // Fast ray pre-filtering: only compute magic attacks if a slider exists on that line
    U64 sliders = 0ULL;
    if (pseudo_rook_rays[sq] & rooks_queens) {
        if (!r_computed) {
            r_att = getRookAttacks(sq, occ);
            r_computed = true;
        }
        sliders |= (r_att & rooks_queens);
    }
    if (pseudo_bishop_rays[sq] & bishops_queens) {
        if (!b_computed) {
            b_att = getBishopAttacks(sq, occ);
            b_computed = true;
        }
        sliders |= (b_att & bishops_queens);
    }

    // 2. Incoming threats to 'piece' at 'sq' (if 'piece' is non-King)
    if (piece != K && piece != k) {
        U64 incoming = (getPawnAttacks(black, sq) & pos->bitboards[P])
                     | (getPawnAttacks(white, sq) & pos->bitboards[p])
                     | (getKnightAttacks(sq) & (pos->bitboards[N] | pos->bitboards[n]))
                     | sliders;

        while (incoming) {
            int from_sq = getLS1BIndex(incoming);
            popBit(incoming, from_sq);

            // If a slider's attack to 'sq' passed through 'blocked_sq' (which was occupied before the move),
            // then that slider was blocked before the move and was NOT an incoming attacker to 'sq'!
            if (blocked_sq >= 0 && ((lineBB[from_sq][sq] | rayBB[from_sq][sq]) & (1ULL << blocked_sq))) {
                continue;
            }

            int attacker_piece = pos->mailbox[from_sq];
            record_threat_pair(u, attacker_piece, piece, from_sq, sq, w_ksq, b_ksq, is_add);
        }
    }

    // 3. Discovered / Blocked slider rays passing through 'sq'
    // When is_add == true  (piece placed on 'sq'), rays passing through 'sq' are BLOCKED (!is_add = false).
    // When is_add == false (piece removed from 'sq'), rays passing through 'sq' are OPENED (!is_add = true).
    if (discovery && sliders) {
        U64 slider_bb = sliders;
        while (slider_bb) {
            int slider_sq = getLS1BIndex(slider_bb);
            popBit(slider_bb, slider_sq);

            // Skip false discovery if the moving piece slid along this exact slider's ray
            if (discoveryMask && ((ray_pass[slider_sq][sq] & discoveryMask) == discoveryMask)) {
                continue;
            }

            U64 ray = ray_pass[slider_sq][sq] & ~(1ULL << sq) & ~lineBB[slider_sq][sq] & ~rayBB[slider_sq][sq];
            U64 discovered = ray & (r_att | b_att) & valid_targets;

            if (discovered) {
                int target_sq    = getLS1BIndex(discovered);
                int slider_piece = pos->mailbox[slider_sq];
                int target_piece = pos->mailbox[target_sq];
                record_threat_pair(u, slider_piece, target_piece, slider_sq, target_sq, w_ksq, b_ksq, !is_add);
            }
        }
    }
}

static inline void apply_perspective_threat_updates(const v16u *restrict src,
                                                    v16u *restrict dst,
                                                    const int *adds, int num_adds,
                                                    const int *subs, int num_subs) {
    if (num_adds == 0 && num_subs == 0) {
        if (src != dst) {
            for (int k = 0; k < HIDDEN_VECS; k++) dst[k] = src[k];
        }
        return;
    }

    for (int tile = 0; tile < HIDDEN_VECS; tile += TILE_VECS) {
        v16 regs[TILE_VECS];
        for (int k = 0; k < TILE_VECS; k++)
            regs[k] = src[tile + k];

        for (int i = 0; i < num_adds; i++) {
            const v16u *w = (const v16u *)weights->ft_threat_weights[adds[i]];
            for (int k = 0; k < TILE_VECS; k++)
                regs[k] += w[tile + k];
        }
        for (int i = 0; i < num_subs; i++) {
            const v16u *w = (const v16u *)weights->ft_threat_weights[subs[i]];
            for (int k = 0; k < TILE_VECS; k++)
                regs[k] -= w[tile + k];
        }

        for (int k = 0; k < TILE_VECS; k++)
            dst[tile + k] = regs[k];
    }
}

void apply_threat_updates(board *pos, int prev_ply, int cur_ply, const ThreatUpdates *u) {
    apply_perspective_threat_updates((const v16u *)pos->threat_accum_white[prev_ply],
                                     (v16u *)pos->threat_accum_white[cur_ply],
                                     u->white_adds, u->num_white_adds,
                                     u->white_subs, u->num_white_subs);
    apply_perspective_threat_updates((const v16u *)pos->threat_accum_black[prev_ply],
                                     (v16u *)pos->threat_accum_black[cur_ply],
                                     u->black_adds, u->num_black_adds,
                                     u->black_subs, u->num_black_subs);
}
