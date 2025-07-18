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


void writeHashEntry(uint64_t key, int16_t score, int bestMove, uint8_t depth, uint8_t hashFlag, bool ttPv, board* position) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[get_hash_index(position->hashKey)];

    if (bestMove != 0 || key != position->hashKey) {
        hashEntry->bestMove = bestMove;
    }

    if (hashFlag == hashFlagExact || key != position->hashKey || depth + 2 * ttPv + 4 > hashEntry->depth) {
        // store score independent from the actual path
        // from root node (position) to current node (position)
        if (score < -mateScore) score -= position->ply;
        if (score > mateScore) score += position->ply;


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
int readHashEntry(board *position, int *move, int16_t *tt_score,
                    uint8_t *tt_depth, uint8_t *tt_flag, bool *tt_pv) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[get_hash_index(position->hashKey)];

    // make sure we're dealing with the exact position we need
    if (hashEntry->hashKey == get_hash_low_bits(position->hashKey)) {

        // extract stored score from TT entry
        int16_t score = hashEntry->score;

        if (score < -mateScore)
            score += position->ply;
        if (score > mateScore)
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

// dynamically allocate memory for hash table
void init_hash_table(int mb) {
    // init hash size
    U64 hash_size;

    int attempts = 0;
    int max_attempts = 5;

    // free hash table if not empty
    if (hashTable != NULL) {
        printf("Clearing hash memory...\n");
        // free hash table dynamic memory
        free(hashTable);
    }

    while (attempts < max_attempts) {
        hash_size = 0x100000 * mb;
        hash_entries = hash_size / sizeof(tt);

        // allocate memory
        hashTable = (tt *) malloc(hash_entries * sizeof(tt));

        // if allocation has failed
        if (hashTable == NULL) {
            printf("Couldn't allocate memory for hash table, trying %dMB...\n", mb / 2);
            mb /= 2; // allocate with half size
            attempts++;
        } else {
            // clear hash table
            clearHashTable();
            printf("Hash table is initialied with %llu entries\n", hash_entries);
            return;
        }
    }

    // if all attempts fail
    if (hashTable == NULL) {
        printf("Failed to allocate memory for hash table after %d attempts\n", max_attempts);
        exit(1); // or handle the error as needed
    }
}

// init random hash keys
void initRandomKeys(void) {
    // update pseudo random number state
    state = 1804289383;
    // loop over piece codes
    for (int piece = P; piece <= k; piece++) {
        // loop over board squares
        for (int square = 0; square < 64; square++) {
            pieceKeys[piece][square] = getRandom64Numbers();
        }
    }
    // loop over board squares
    for (int square = 0; square < 64; square++) {
        // init random enpassant keys
        enpassantKeys[square] = getRandom64Numbers();
    }
    // loop over castling keys
    for (int index = 0; index < 16; index++) {
        // init castling keys
        castleKeys[index] = getRandom64Numbers();
    }
    // loop over castling keys
    sideKey = getRandom64Numbers();
}