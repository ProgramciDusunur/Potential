//
// Created by erena on 13.09.2024.
//

#include "table.h"

U64 sideKey;
U64 hash_entries = 0;
tt *hashTable = NULL;


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

void writeHashEntry(int score, int bestMove, int depth, int hashFlag, board* position) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[position->hashKey % hash_entries];

    // store score independent from the actual path
    // from root node (position) to current node (position)
    if (score < -mateScore) score -= position->ply;
    if (score > mateScore) score += position->ply;


    hashEntry->hashKey = position->hashKey;
    hashEntry->score = score;
    hashEntry->flag = hashFlag;
    hashEntry->depth = depth;
    hashEntry->bestMove = bestMove;
}

// read hash entry data
int readHashEntry(board *position, int *move, int16_t *tt_score,
                    uint8_t *tt_depth, uint8_t *tt_flag) {
    // create a TT instance pointer to particular hash entry storing
    // the scoring data for the current board position if available
    tt *hashEntry = &hashTable[position->hashKey % hash_entries];

    // make sure we're dealing with the exact position we need
    if (hashEntry->hashKey == position->hashKey) {

        // extract stored score from TT entry
        int score = hashEntry->score;

        if (score < -mateScore)
            score += position->ply;
        if (score > mateScore)
            score -= position->ply;

        *move = hashEntry->bestMove;
        *tt_score = score;
        *tt_depth = hashEntry->depth;
        *tt_flag = hashEntry->flag;

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
