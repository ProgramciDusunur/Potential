#ifndef POTENTIAL_CUCKOO_H
#define POTENTIAL_CUCKOO_H

#include <stdint.h>
#include <stdbool.h>
#include "board_constants.h"
#include "structs.h"

#define CUCKOO_TABLE_SIZE 8192

extern uint64_t cuckoo_keys[CUCKOO_TABLE_SIZE];
extern uint16_t cuckoo_moves[CUCKOO_TABLE_SIZE];

static inline uint32_t cuckoo_h1(uint64_t key) {
    return (uint32_t)(key & 0x1FFF);
}

static inline uint32_t cuckoo_h2(uint64_t key) {
    return (uint32_t)((key >> 16) & 0x1FFF);
}

void cuckoo_init(void);
bool has_game_cycle(const board* pos, int ply);

#endif //POTENTIAL_CUCKOO_H