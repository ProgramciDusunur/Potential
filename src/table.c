//
// Created by erena on 13.09.2024.
//

#include "table.h"

U64 sideKey;
U64 hash_entries = 0;
tt *hashTable = NULL;

__extension__ typedef unsigned __int128 uint128_t;


int hash_full(void) {
  int used = 0;
  int samples = 1000;

  for (int i = 0; i < samples; ++i) {
    if (hashTable[i].hashKey != 0) {
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

uint64_t get_hash_index(uint64_t hash) {
    return ((uint128_t)hash * (uint128_t)hash_entries) >> 64;
}

uint32_t get_hash_low_bits(uint64_t hash) {
    return (uint32_t)hash;
}

void prefetch_hash_entry(uint64_t hash_key) {
    const uint64_t index = get_hash_index(hash_key);
    __builtin_prefetch(&hashTable[index]);
}


void writeHashEntry(uint64_t key, int16_t score, uint16_t bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[get_hash_index(position->hashKey)];

    if (bestMove != 0 || key != position->hashKey) {
        hashEntry->bestMove = bestMove;
    }

    if (hashFlag == hashFlagExact || key != position->hashKey || depth + 2 * ttPv + 4 > hashEntry->depth) {
        // store score independent from the actual path
        // from root node (position) to current node (position)
        if (score < -mateValue) score -= position->ply;
        if (score > mateValue) score += position->ply;


        hashEntry->hashKey = get_hash_low_bits(position->hashKey);
        hashEntry->score = score;
        hashEntry->flag = hashFlag;
        hashEntry->depth = depth;        
        hashEntry->ttPv = ttPv;
    } else if (hashEntry->depth >= 5 && hashFlag != hashFlagExact) {        
        hashEntry->depth--;
    }
        

    
}

// read hash entry data
int readHashEntry(board *position, uint16_t *move, int16_t *tt_score,
                    uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[get_hash_index(position->hashKey)];

    // make sure we're dealing with the exact position we need
    if (hashEntry->hashKey == get_hash_low_bits(position->hashKey)) {

        // extract stored score from TT entry
        int16_t score = hashEntry->score;

        if (score < -mateValue)
            score += position->ply;
        if (score > mateValue)
            score -= position->ply;

        *move = hashEntry->bestMove;
        *tt_score = score;
        *tt_depth = hashEntry->depth;
        *tt_flag = hashEntry->flag;
        *tt_pv = hashEntry->ttPv;

        return 1;

    }
    // if hash entry doesn't exist
    return 0;
}


void clearHashTable(void) {
    // init hash table entry pointer
    tt *hash_entry;

    // loop over TT elements
    for (hash_entry = hashTable; hash_entry < hashTable + hash_entries; hash_entry++)
    {
        // reset TT inner fields
        hash_entry->hashKey = 0;
        hash_entry->depth = 0;
        hash_entry->flag = 0;
        hash_entry->score = 0;
        hash_entry->bestMove = 0;
        hash_entry->ttPv = 0;
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
        // Linux: Try Static Huge Pages first
        ptr = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (ptr != MAP_FAILED) {
            snprintf(status_msg, sizeof(status_msg), "SUCCESS (Static Huge Pages)");
        } else {
            // Fallback to standard mmap + THP hint
            ptr = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | ANONYMOUS, -1, 0);
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
    // loop over castling keys
    sideKey = get_random_uint64_number();
}
