//
// Created by erena on 13.09.2024.
//

#include "table.h"

U64 sideKey;
U64 FMR[100 / 10 + 1];
U64 hash_entries = 0;
tt *hashTable = NULL;
uint8_t tt_age = 0;

__extension__ typedef unsigned __int128 uint128_t;


int hash_full(void) {
  int used = 0;

  // TO-DO: Recheck this.
  int samples = 1000 / 4;

  for (int i = 0; i < samples; ++i) {
      for (int j = 0; j < 4; ++j) {
        if (hashTable[i].entries[j].key != 0 || (hashTable[i].entries[j].flag & 0x3) != hashFlagNone) {
          used++;
        }
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
            finalKey ^= pieceKeys[piece][square].hashKey;

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

        final_key ^= pieceKeys[P][square].hashKey;
        popBit(bitboard, square);
    }

    bitboard = position->bitboards[p];

    while (bitboard) {
        int square = getLS1BIndex(bitboard);

        final_key ^= pieceKeys[p][square].hashKey;
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

            final_key ^= pieceKeys[piece][square].hashKey;
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

            final_key ^= pieceKeys[piece][square].hashKey;
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

            final_key ^= pieceKeys[piece][square].hashKey;
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

            final_key ^= pieceKeys[piece][square].hashKey;
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

            final_key ^= pieceKeys[piece][square].hashKey;
            popBit(bitboard, square);
        }
    }

    return final_key;
}

uint64_t get_hash_index(uint64_t hash, uint8_t fmr_key) {
    hash ^= FMR[fmr_key / 10];
    return ((uint128_t)hash * (uint128_t)hash_entries) >> 64;
}

uint32_t get_hash_low_bits(uint64_t hash) {
    return (uint32_t)hash;
}

void prefetch_hash_entry(uint64_t hash_key, uint8_t fmr_key) {
    const uint64_t index = get_hash_index(hash_key, fmr_key);
    __builtin_prefetch(&hashTable[index]);
}

void prefetch_corrhist(board *pos, ThreadData *t) { 
    const int mask = t->shared_history->corrhist_mask;
    const int side = pos->side;

    __builtin_prefetch(&t->shared_history->pawn_corrhist[side][pos->pawnKey & mask]);
    __builtin_prefetch(&t->shared_history->minor_corrhist[side][pos->minorKey & mask]);
    __builtin_prefetch(&t->shared_history->major_corrhist[side][pos->majorKey & mask]);
    __builtin_prefetch(&t->shared_history->non_pawn_corrhist[white][side][pos->whiteNonPawnKey & mask]);
    __builtin_prefetch(&t->shared_history->non_pawn_corrhist[black][side][pos->blackNonPawnKey & mask]);
    __builtin_prefetch(&t->shared_history->krp_corrhist[side][pos->krpKey & mask]);
}

void writeHashEntry(uint64_t key, int16_t score, uint16_t bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position, uint8_t fmr_key) {
    tt *cluster = &hashTable[get_hash_index(key, fmr_key)];
    uint16_t hash16 = (uint16_t)key;
    
    int replace = 0;
    int min_value = 999999;
    
    for (int i = 0; i < 4; i++) {
        tt_entry *candidate = &cluster->entries[i];
        if (candidate->key == hash16 || (candidate->flag & 0x3) == hashFlagNone) {
            replace = i;
            break;
        }
        
        int candidate_age = candidate->flag >> 3;
        int relative_age = (32 + tt_age - candidate_age) & 31;
        int value = (int)candidate->depth - 2 * relative_age;
        
        if (value < min_value) {
            replace = i;
            min_value = value;
        }
    }
    
    tt_entry *entry = &cluster->entries[replace];
    
    if (!(hash16 != entry->key
        || hashFlag == hashFlagExact
        || depth + 4 + 2 * ttPv > entry->depth
        || (entry->flag >> 3) != tt_age)) {
        return;
    }

    if (bestMove != 0 || hash16 != entry->key) {
        entry->bestMove = bestMove;
    }

    if (score < -mateFound) score -= position->ply;
    if (score > mateFound) score += position->ply;

    entry->key = hash16;
    entry->score = score;
    entry->depth = depth;
    entry->flag = hashFlag | (ttPv << 2) | (tt_age << 3);
}

// read hash entry data
bool readHashEntry(board *position, uint16_t *move, int16_t *tt_score,
                    uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv, uint8_t fmr_key) {
    tt *cluster = &hashTable[get_hash_index(position->hashKey, fmr_key)];
    uint16_t hash16 = (uint16_t)position->hashKey;
    
    for (int i = 0; i < 4; i++) {
        tt_entry *entry = &cluster->entries[i];
        if (entry->key == hash16 && (entry->flag & 0x3) != hashFlagNone) {
            int16_t score = entry->score;
            if (score < -mateFound) score += position->ply;
            if (score > mateFound) score -= position->ply;

            *move = entry->bestMove;
            *tt_score = score;
            *tt_depth = entry->depth;
            *tt_flag = entry->flag & 0x3;
            *tt_pv = (entry->flag >> 2) & 1;
            return true;
        }
    }
    return false;
}


void clearHashTable(void) {
    for (uint64_t i = 0; i < hash_entries; i++) {
        for (int j = 0; j < 4; j++) {
            hashTable[i].entries[j].key = 0;
            hashTable[i].entries[j].bestMove = 0;
            hashTable[i].entries[j].score = 0;
            hashTable[i].entries[j].depth = 0;
            hashTable[i].entries[j].flag = 0;
        }
    }
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
#ifdef __linux__
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
#else        
        ptr = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr != MAP_FAILED) {
            snprintf(status_msg, sizeof(status_msg), "SUCCESS (standard pages)");
        } else {
            ptr = NULL;
        }
#endif
#endif

        if (ptr == NULL || ptr == (void*)-1) {
            mb /= 2;
            attempts++;
        } else {
            hashTable = (tt*)ptr;
            current_allocated_bytes = bytes;
            hash_entries = bytes / sizeof(tt);
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
            U64 key = get_random_uint64_number();
            pieceKeys[piece][square].hashKey = key;
            pieceKeys[piece][square].pawnKey = (piece == P || piece == p) ? key : 0;
            pieceKeys[piece][square].minorKey = (isMinor(piece)) ? key : 0;
            pieceKeys[piece][square].majorKey = (isMajor(piece)) ? key : 0;
            pieceKeys[piece][square].whiteNonPawnKey = (piece != P && piece != p && pieceColor(piece) == white) ? key : 0;
            pieceKeys[piece][square].blackNonPawnKey = (piece != P && piece != p && pieceColor(piece) == black) ? key : 0;
            pieceKeys[piece][square].krpKey = (isKRP(piece) && piece != P && piece != p) ? key : 0;
            pieceKeys[piece][square].padding_key = 0;
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

    for (int i = 0; i <= 100 / 10; i++) {
        if (i * 10 <= 50) {
            FMR[i] = get_random_uint64_number();
        } else {
            FMR[i] = 0ULL;
        }
    }
}
