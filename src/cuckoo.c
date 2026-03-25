#include "cuckoo.h"

#define TABLE_SIZE 8192

uint64_t cuckoo_keys[TABLE_SIZE];
uint16_t cuckoo_moves[TABLE_SIZE];

bool can_attack(uint8_t sq0, uint8_t sq1) {
    if (sq0 == sq1) {
        return false;
    }
    
    bool can_attack = false;
    return can_attack;
}