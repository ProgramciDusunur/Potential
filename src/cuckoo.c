#include "cuckoo.h"
#include "board_constants.h"
#include "table.h"
#include "move.h"
#include "mask.h"
#include <stdlib.h>
#include <assert.h>

uint64_t cuckoo_keys[CUCKOO_TABLE_SIZE] = {0};
uint16_t cuckoo_moves[CUCKOO_TABLE_SIZE] = {0};

static bool can_attack(int ptype, int sq0, int sq1) {
    if (sq0 == sq1) {
        return false;
    }
    
    int rank0 = get_rank[sq0];
    int file0 = sq0 % 8;
    int rank1 = get_rank[sq1];
    int file1 = sq1 % 8;

    int file_diff = file1 - file0;
    int rank_diff = rank1 - rank0;
    int abs_file_diff = abs(file_diff);
    int abs_rank_diff = abs(rank_diff);

    switch (ptype) {
    case N:
    case n:
        return (abs_file_diff == 2 && abs_rank_diff == 1)
            || (abs_file_diff == 1 && abs_rank_diff == 2);

    case B:
    case b:
        return abs_file_diff == abs_rank_diff && abs_file_diff > 0;

    case R:
    case r:
        return (file_diff == 0 && rank_diff != 0) || (file_diff != 0 && rank_diff == 0);

    case Q:
    case q:
        return (file_diff == 0 && rank_diff != 0) || (file_diff != 0 && rank_diff == 0)
            || (abs_file_diff == abs_rank_diff && abs_file_diff > 0);

    case K:
    case k:
        return abs_file_diff <= 1 && abs_rank_diff <= 1;

    default:
        return false;
    }
}

void cuckoo_init(void) {
    int count = 0;

    uint64_t temp_keys[CUCKOO_TABLE_SIZE] = {0};
    uint16_t temp_moves[CUCKOO_TABLE_SIZE] = {0};

    int ptypes[] = {N, B, R, Q, K};

    for (int color = white; color <= black; color++) {
        for (int p_idx = 0; p_idx < 5; p_idx++) {
            int ptype = ptypes[p_idx];
            // Adjust piece type for black
            if (color == black) {
                ptype += 6; // N -> n, B -> b, R -> r, Q -> q, K -> k
            }

            for (int sq0 = 0; sq0 < 64; sq0++) {
                for (int sq1 = sq0 + 1; sq1 < 64; sq1++) {
                    
                    if (!can_attack(ptype, sq0, sq1)) {
                        continue;
                    }

                    uint64_t key = pieceKeys[ptype][sq0]
                                 ^ pieceKeys[ptype][sq1]
                                 ^ sideKey;

                    uint16_t mv = encodeMove(sq0, sq1, mf_normal);

                    uint32_t slot = cuckoo_h1(key);
                    while (true) {
                        // Swap temp_keys[slot] and key
                        uint64_t tk = temp_keys[slot];
                        temp_keys[slot] = key;
                        key = tk;

                        // Swap temp_moves[slot] and mv
                        uint16_t tm = temp_moves[slot];
                        temp_moves[slot] = mv;
                        mv = tm;

                        if (mv == 0) {
                            break; // 0 is mf_normal with A1 to A1 which is no move
                        }

                        slot = (slot == cuckoo_h1(key)) ? cuckoo_h2(key) : cuckoo_h1(key);
                    }
                    count++;
                }
            }
        }
    }

    // Copy to final arrays
    for (int i = 0; i < CUCKOO_TABLE_SIZE; i++) {
        cuckoo_keys[i] = temp_keys[i];
        cuckoo_moves[i] = temp_moves[i];
    }
}

bool has_game_cycle(const board* pos, int ply) {
    int end = pos->fifty < pos->repetitionIndex ? pos->fifty : pos->repetitionIndex;

    if (end < 3) {
        return false;
    }

    uint64_t occ = pos->occupancies[both];
    uint64_t original_key = pos->hashKey;
    
    uint64_t other = ~(original_key ^ pos->repetitionTable[pos->repetitionIndex - 2]);

    for (int i = 3; i <= end; i += 2) {
        uint64_t curr_key = pos->repetitionTable[pos->repetitionIndex - (i + 1)];

        other ^= ~(curr_key ^ pos->repetitionTable[pos->repetitionIndex - i]);
        
        if (other != 0) {
            continue;
        }

        uint64_t diff = original_key ^ curr_key;

        uint32_t slot = cuckoo_h1(diff);

        if (diff != cuckoo_keys[slot]) {
            slot = cuckoo_h2(diff);
        }

        if (diff != cuckoo_keys[slot]) {
            continue;
        }

        uint16_t mv = cuckoo_moves[slot];
        int from = getMoveSource(mv);
        int to = getMoveTarget(mv);

        // Check if there are pieces along the ray between from and to.
        uint64_t between = 0;
        if (get_rank[from] == get_rank[to] || (from % 8) == (to % 8)) {
            between = lineBB[from][to];
        } else {
            between = rayBB[from][to];
        }

        if ((occ & between) == 0ULL) {
            if (ply > i) {
                return true;
            }

            int piece = pos->mailbox[from];
            if (piece == NO_PIECE) {
                piece = pos->mailbox[to];
            }

            return pieceColor(piece) == pos->side;
        }
    }

    return false;
}