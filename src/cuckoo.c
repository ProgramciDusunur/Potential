//
// Created by Potential Engine Team
//

#include "cuckoo.h"
#include "board_constants.h"
#include "move.h"
#include "bit_manipulation.h"
#include <string.h>

#define CUCKOO_SIZE 8192
#define CUCKOO_MASK (CUCKOO_SIZE - 1)

CuckooEntry cuckooTable[2][CUCKOO_SIZE];

static inline int H1(U64 key) {
    return (int)(key & CUCKOO_MASK);
}

static inline int H2(U64 key) {
    return (int)((key >> 16) & CUCKOO_MASK);
}

// Insert logic
void insertCuckoo(U64 key, uint16_t move) {
    int index1 = H1(key);
    int index2 = H2(key);

    // Try slot 1
    if (cuckooTable[0][index1].move == 0) {
        cuckooTable[0][index1].key = key;
        cuckooTable[0][index1].move = move;
        return;
    }

    // Try slot 2
    if (cuckooTable[1][index2].move == 0) {
        cuckooTable[1][index2].key = key;
        cuckooTable[1][index2].move = move;
        return;
    }    
    
    cuckooTable[0][index1].key = key;
    cuckooTable[0][index1].move = move;
}

void initCuckoo(void) {
    memset(cuckooTable, 0, sizeof(cuckooTable));
   
    int pieces[] = {N, B, R, Q, K}; // White pieces    
    
    for (int piece_idx = P; piece_idx <= k; piece_idx++) {
        if (piece_idx == P || piece_idx == p) continue; 
        for (int src = 0; src < 64; src++) {
            U64 attacks = 0;
            
            switch (piece_idx) {
                case N: case n: attacks = knightAttacks[src]; break;
                case B: case b: attacks = getBishopAttacks(src, 0); break;
                case R: case r: attacks = getRookAttacks(src, 0); break;
                case Q: case q: attacks = getQueenAttacks(src, 0); break;
                case K: case k: attacks = kingAttacks[src]; break;
            }

            while (attacks) {
                int dst = getLS1BIndex(attacks);
                
                U64 moveKey = pieceKeys[piece_idx][src] ^ pieceKeys[piece_idx][dst] ^ sideKey;
                
                uint16_t move = encodeMove(src, dst, 0); // 0 flag for quiet
                
                insertCuckoo(moveKey, move);

                popBit(attacks, dst);
            }
        }
    }
    
    printf("Cuckoo Table Initialized.\n");
}

int hasUpcomingRepetition(board *pos) {
    if (pos->repetitionIndex < 1) return 0;

    

    for (int i = pos->repetitionIndex - 1; i >= 0; i -= 2) {
        U64 diff = pos->hashKey ^ pos->repetitionTable[i];                
        
        int idx1 = H1(diff);
        int idx2 = H2(diff);
        
        uint16_t move = 0;
        
        if (cuckooTable[0][idx1].key == diff) {
            move = cuckooTable[0][idx1].move;
        } else if (cuckooTable[1][idx2].key == diff) {
            move = cuckooTable[1][idx2].move;
        }
        
        if (move != 0) {           
            int src = getMoveSource(move);
            int dst = getMoveTarget(move);
            
            int piece = pos->mailbox[src];
                       
            if (piece == NO_PIECE) continue;
            if (pieceColor(piece) != pos->side) continue;            
            if (pos->mailbox[dst] != NO_PIECE) continue;                        
            
            U64 attacks = 0;
            switch (piece) {
                case N: case n: attacks = knightAttacks[src]; break;
                case B: case b: attacks = getBishopAttacks(src, pos->occupancies[both]); break;
                case R: case r: attacks = getRookAttacks(src, pos->occupancies[both]); break;
                case Q: case q: attacks = getQueenAttacks(src, pos->occupancies[both]); break;
                case K: case k: attacks = kingAttacks[src]; break;
            }
            
            if (getBit(attacks, dst)) {               
                return 1; 
            }
        }
    }
    
    return 0;
}
