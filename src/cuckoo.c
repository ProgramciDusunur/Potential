#include "cuckoo.h"

U64 cuckoo_keys[CUCKOO_TABLE_SIZE];
uint16_t cuckoo_moves[CUCKOO_TABLE_SIZE];

int cuckoo_h1(U64 key) {
    return (int)(key & 0x1FFF);
}
int cuckoo_h2(U64 key) {
    return (int)((key >> 16) & 0x1FFF);
}


int canReach(int piece, int sq1, int sq2) {
    U64 target = 1ULL << sq2;
    
    int type = piece % 6;

    switch (type) {
        case 1:
            return (knightAttacks[sq1] & target) != 0;
        case 2:
            return (bishopAttack(sq1, 0) & target) != 0;
        case 3:
            return (rookAttack(sq1, 0) & target) != 0;
        case 4:
            return ((bishopAttack(sq1, 0) | rookAttack(sq1, 0)) & target) != 0;
        case 5:
            return (kingAttacks[sq1] & target) != 0;
        default:
            return 0;
    }
}


void cuckoo_init(void) {
    memset(cuckoo_keys, 0, sizeof(cuckoo_keys));
    memset(cuckoo_moves, 0, sizeof(cuckoo_moves));

    int count = 0;
    
    for (int piece = N; piece <= k; piece++) {

        if (piece == P || piece == p) continue;

        for (int sq1 = 0; sq1 < 64; sq1++) {
            for (int sq2 = sq1 + 1; sq2 < 64; sq2++) {
                if (!canReach(piece, sq1, sq2)) continue;

                U64 key = pieceKeys[piece][sq1].hashKey
                        ^ pieceKeys[piece][sq2].hashKey
                        ^ sideKey;
                
                uint16_t move = encodeMove(sq1, sq2, mf_normal);
                
                int slot = cuckoo_h1(key);

                while (1) {                    
                    U64 tmpKey = cuckoo_keys[slot];
                    uint16_t tmpMove = cuckoo_moves[slot];

                    cuckoo_keys[slot] = key;
                    cuckoo_moves[slot] = move;

                    key = tmpKey;
                    move = tmpMove;
                    
                    if (key == 0) break;

                    if (slot == cuckoo_h1(key))
                        slot = cuckoo_h2(key);
                    else
                        slot = cuckoo_h1(key);
                }

                count++;
            }
        }
    }

    // Correction: the entry size should be 3668
    // Knight: 168 squares
    // Bishop: 280 squares
    // Rook: 448 squares
    // Queen: 728 squares (280 + 448)
    // King: 210 squares

    // 1834 squares for per piece type (excluding pawns) = 3668 total entries

    if (count != 3668) {
        fprintf(stderr, "Cuckoo table initialization error: expected 3668 entries, got %d\n", count);
        exit(1);
    }

    assert(count == 3668);
}

bool has_game_cycle(board *pos, int ply) {    
    int end = pos->fifty < pos->repetitionIndex
            ? pos->fifty : pos->repetitionIndex;

    
    if (end < 3) return false;

    U64 originalKey = pos->hashKey;
    U64 occ = pos->occupancies[both];

    for (int i = 3; i <= end; i += 2) {        
        U64 pastKey = pos->repetitionTable[pos->repetitionIndex - i + 1];

        U64 diff = originalKey ^ pastKey;
        
        int slot = cuckoo_h1(diff);
        if (diff != cuckoo_keys[slot]) {
            slot = cuckoo_h2(diff);
        }
        if (diff != cuckoo_keys[slot]) {
            continue;
        }

        
        uint16_t mv = cuckoo_moves[slot];
        int sq1 = getMoveSource(mv);
        int sq2 = getMoveTarget(mv);
        
        U64 between = lineBB[sq1][sq2] | rayBB[sq1][sq2];
        if (between & occ) {
            continue;
        }
        
        if (ply > i) {
            return true;
        }
        
        int piece = pos->mailbox[sq1];
        if (piece >= NO_PIECE) {
            piece = pos->mailbox[sq2];
        }
        if (piece < NO_PIECE) {            
            int pieceColor = (piece < 6) ? white : black;
            if (pieceColor == pos->side) {
                return true;
            }
        }
    }

    return false;
}

