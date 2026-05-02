//
// Created by erena on 13.09.2024.
//

#include "table.h"

U64 sideKey;
U64 FMR[100 / 10];
U64 hash_entries = 0;
TTCluster *hashTable = NULL;
uint8_t tt_age = 0;

__extension__ typedef unsigned __int128 uint128_t;


int hash_full(void) {
  int used = 0;
  int samples = 1000;

  for (int i = 0; i < samples; ++i) {
    if (tt_bound(hashTable[i / TT_CLUSTER_SIZE].entries[i % TT_CLUSTER_SIZE].flags) != hashFlagNone) {
      used++;
    }
  }

  return used;
}


// generate "almost" unique position ID aka hash key from scratch
U64 generateHashKey(board* position) {
    // final hash key
    U64 finalKey = 0ULL;

    // temp piece bitboard copy
    U64 bitboard;


    // loop over piece bitboards
    for (int piece = P; piece <= k; piece++) {
        // init piece bitboard copy
        bitboard = position->bitboards[piece];

        // loop over the pieces within a bitboard
        while (bitboard) {
            // init square occupied by the piece
            int square = getLS1BIndex(bitboard);

            // hash piece
            finalKey ^= pieceKeys[piece][square];

            // pop LS1B
            popBit(bitboard, square);
        }
    }

    if (position->enpassant != no_sq) {
        // hash enpassant
        finalKey ^= enpassantKeys[position->enpassant];
    }
    // hash castling rights
    finalKey ^= castleKeys[position->castle];

    // hash the side only if black is to move
    if (position->side == black) { finalKey ^= sideKey; }

    // return generated hash key
    return finalKey;
}

U64 generatePawnKey(board* position) {

    uint64_t final_key = 0ULL;
    uint64_t bitboard;

    bitboard = position->bitboards[P];

    while (bitboard) {
        int square = getLS1BIndex(bitboard);

        final_key ^= pieceKeys[P][square];
        popBit(bitboard, square);
    }

    bitboard = position->bitboards[p];

    while (bitboard) {
        int square = getLS1BIndex(bitboard);

        final_key ^= pieceKeys[p][square];
        popBit(bitboard, square);
    }
    return final_key;

}

U64 generateMinorKey(board *position) {
    uint64_t final_key = 0ULL;
    uint64_t bitboard;


    for (int i = 0; i < 6; i++) {

        int piece = minorPieces[i];
        bitboard = position->bitboards[piece];

        while (bitboard) {

            int square = getLS1BIndex(bitboard);

            final_key ^= pieceKeys[piece][square];
            popBit(bitboard, square);
        }
    }

    return final_key;
}

U64 generateMajorKey(board *position) {
    uint64_t final_key = 0ULL;
    uint64_t bitboard;


    for (int i = 0; i < 4; i++) {

        int piece = majorPieces[i];
        bitboard = position->bitboards[piece];

        while (bitboard) {

            int square = getLS1BIndex(bitboard);

            final_key ^= pieceKeys[piece][square];
            popBit(bitboard, square);
        }
    }

    return final_key;
}

// generates white non pawn hashing key
U64 generate_white_np_hash_key(board *position) {
    uint64_t final_key = 0ULL;
    uint64_t bitboard;


    for (int i = 0; i < 5; i++) {
        int piece = whiteNonPawnPieces[i];
        bitboard = position->bitboards[piece];

        while (bitboard) {

            int square = getLS1BIndex(bitboard);

            final_key ^= pieceKeys[piece][square];
            popBit(bitboard, square);
        }
    }


    return final_key;
}

// generates black non pawn hashing key
U64 generate_black_np_hash_key(board *position) {
    uint64_t final_key = 0ULL;
    uint64_t bitboard;


    for (int i = 0; i < 5; i++) {
        int piece = blackNonPawnPieces[i];
        bitboard = position->bitboards[piece];

        while (bitboard) {

            int square = getLS1BIndex(bitboard);

            final_key ^= pieceKeys[piece][square];
            popBit(bitboard, square);
        }
    }


    return final_key;
}

U64 generate_krp_key(board *position) {
    uint64_t final_key = 0ULL;
    uint64_t bitboard;


    for (int i = 0; i < 6; i++) {

        int piece = krpPieces[i];
        bitboard = position->bitboards[piece];

        while (bitboard) {

            int square = getLS1BIndex(bitboard);

            final_key ^= pieceKeys[piece][square];
            popBit(bitboard, square);
        }
    }

    return final_key;
}

uint64_t get_hash_index(uint64_t hash, uint8_t fmr_key) {
    hash ^= FMR[fmr_key / 10];
    return ((uint128_t)hash * (uint128_t)hash_entries) >> 64;
}

uint64_t get_hash_verification_key(uint64_t hash) {
    uint64_t key = hash;
    return key ? key : 1;
}

void prefetch_hash_entry(uint64_t hash_key, uint8_t fmr_key) {
    const uint64_t index = get_hash_index(hash_key, fmr_key);
    __builtin_prefetch(&hashTable[index]);
}

void tt_age_inc(void) {
    tt_age = (tt_age + 1) & TT_AGE_MASK;
}

void prefetch_corrhist(board *pos) { 
    const int mask = CORRHIST_SIZE - 1;
    const int side = pos->side;

    __builtin_prefetch(&thread_pool.shared_history.pawn_corrhist[side][pos->pawnKey & mask]);
    __builtin_prefetch(&thread_pool.shared_history.minor_corrhist[side][pos->minorKey & mask]);
    __builtin_prefetch(&thread_pool.shared_history.major_corrhist[side][pos->majorKey & mask]);
    __builtin_prefetch(&thread_pool.shared_history.non_pawn_corrhist[white][side][pos->whiteNonPawnKey & mask]);
    __builtin_prefetch(&thread_pool.shared_history.non_pawn_corrhist[black][side][pos->blackNonPawnKey & mask]);
    __builtin_prefetch(&thread_pool.shared_history.krp_corrhist[side][pos->krpKey & mask]);
}

void writeHashEntry(int16_t score, uint16_t bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position, uint8_t fmr_key) {
    TTCluster *cluster = &hashTable[get_hash_index(position->hashKey, fmr_key)];
    uint64_t vkey = get_hash_verification_key(position->hashKey);

    int replace = 0;
    int min_value = 0x7FFFFFFF;

    for (int i = 0; i < TT_CLUSTER_SIZE; i++) {
        tt *entry = &cluster->entries[i];

        if (entry->hashKey == vkey || tt_bound(entry->flags) == hashFlagNone) {
            replace = i;
            break;
        }

        int relative_age = ((int)TT_AGE_CYCLE + (int)tt_age - (int)tt_entry_age(entry->flags)) & TT_AGE_MASK;
        int value = (int)entry->depth - 2 * relative_age;

        if (value < min_value) {
            replace = i;
            min_value = value;
        }
    }

    tt *hashEntry = &cluster->entries[replace];

    if (bestMove != 0 || vkey != hashEntry->hashKey) {
        hashEntry->bestMove = bestMove;
    }

    if (hashFlag == hashFlagExact
        || depth + 4 + 2 * (int8_t)ttPv > hashEntry->depth) {

        if (score < -mateFound) score -= position->ply;
        if (score > mateFound) score += position->ply;

        hashEntry->hashKey = vkey;
        hashEntry->score = score;
        hashEntry->depth = depth;
        hashEntry->flags = tt_pack_flags(hashFlag, ttPv, tt_age);
    } else if (hashEntry->depth >= 5 && hashFlag != hashFlagExact) {
        hashEntry->depth--;
    }
}

bool readHashEntry(board *position, uint16_t *move, int16_t *tt_score,
                    uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv, uint8_t fmr_key) {
    TTCluster *cluster = &hashTable[get_hash_index(position->hashKey, fmr_key)];
    uint64_t vkey = get_hash_verification_key(position->hashKey);

    for (int i = 0; i < TT_CLUSTER_SIZE; i++) {
        tt *entry = &cluster->entries[i];

        if (entry->hashKey == vkey && tt_bound(entry->flags) != hashFlagNone) {
            int16_t score = entry->score;

            if (score < -mateFound)
                score += position->ply;
            if (score > mateFound)
                score -= position->ply;

            *move = entry->bestMove;
            *tt_score = score;
            *tt_depth = entry->depth;
            *tt_flag = tt_bound(entry->flags);
            *tt_pv = tt_pv_flag(entry->flags);

            return true;
        }
    }

    return false;
}


void clearHashTable(void) {
    if (hashTable == NULL) return;
    memset(hashTable, 0, hash_entries * sizeof(TTCluster));
    tt_age = 0;
}

size_t current_allocated_bytes = 0;

void free_hash_table() {
    if (hashTable == NULL) return;
#ifdef _WIN32
    VirtualFree(hashTable, 0, MEM_RELEASE);
#else
    munmap(hashTable, current_allocated_bytes);
#endif
    hashTable = NULL;
    current_allocated_bytes = 0;
}

void init_hash_table(int mb) {
    int attempts = 0;
    int max_attempts = 5;
    char status_msg[100] = "FAILED (standard pages)";

    if (hashTable != NULL) free_hash_table();

    while (attempts < max_attempts) {
        size_t bytes = (size_t)mb * 1024 * 1024;
        void* ptr = NULL;

#ifdef _WIN32
        // Try to enable SeLockMemoryPrivilege
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
            TOKEN_PRIVILEGES tp;
            if (LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &tp.Privileges[0].Luid)) {
                tp.PrivilegeCount = 1;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
            }
            CloseHandle(hToken);
        }

        SIZE_T lp_min = GetLargePageMinimum();
        if (lp_min > 0) {
            size_t rounded = (bytes + lp_min - 1) & ~(lp_min - 1);
            ptr = VirtualAlloc(NULL, rounded, MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE);
            if (ptr) {
                bytes = rounded;
                snprintf(status_msg, sizeof(status_msg), "SUCCESS (Windows Large Pages)");
            }
        }
        if (!ptr) ptr = VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

#else
        // Linux: Try Static Huge Pages first
        ptr = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (ptr != MAP_FAILED) {
            snprintf(status_msg, sizeof(status_msg), "SUCCESS (Static Huge Pages)");
        } else {
            // Fallback to standard mmap + THP hint
            ptr = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr != MAP_FAILED) {
                madvise(ptr, bytes, MADV_HUGEPAGE);
                snprintf(status_msg, sizeof(status_msg), "SUCCESS (Transparent Huge Pages hinted)");
            } else {
                ptr = NULL;
            }
        }
#endif

        if (ptr == NULL || ptr == (void*)-1) {
            mb /= 2;
            attempts++;
        } else {
            hashTable = (TTCluster*)ptr;
            current_allocated_bytes = bytes;
            hash_entries = bytes / sizeof(TTCluster);
            clearHashTable();
            printf("info string Hash: %d MB | Huge Pages: %s\n", mb, status_msg);
            return;
        }
    }
    exit(1);
}

// init random hash keys
void initRandomKeys(void) {    
    // loop over piece codes
    for (int piece = P; piece <= k; piece++) {
        // loop over board squares
        for (int square = 0; square < 64; square++) {
            pieceKeys[piece][square] = get_random_uint64_number();
        }
    }
    // loop over board squares
    for (int square = 0; square < 64; square++) {
        // init random enpassant keys
        enpassantKeys[square] = get_random_uint64_number();
    }
    // loop over castling keys
    for (int index = 0; index < 16; index++) {
        // init castling keys
        castleKeys[index] = get_random_uint64_number();
    }
    // init random side key
    sideKey = get_random_uint64_number();

    for (int i = 0; i < 100 / 10; i++) {
        if (i * 10 <= 50) {
            FMR[i] = get_random_uint64_number();
        } else {
            FMR[i] = 0ULL;
        }
    }
}
