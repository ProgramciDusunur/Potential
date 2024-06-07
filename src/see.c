//
// Created by erena on 31.05.2024.
//


#include "see.h"
#include "board_constants.h"
#include "evaluation.h"
#include "bit_manipulation.h"
#include "move.h"
#include "mask.h"
#include <stdio.h>




U64 get_least_valuable_piece(U64 attadef, int side, int *piece, const board* pos) {
    for (int p_trav = P; p_trav <= K; p_trav++) {
        int offset = side * 6;
        U64 bb = pos->bitboards[p_trav + offset] & attadef;
        if (bb) {
            //printf("Tahtada secilen bitboardun index: %d\n", p_trav + offset);
            //pBitboard(pos->bitboards[p_trav + offset]);
            *piece = p_trav + offset;
            return bb & -bb;
        }
    }
    return 0ULL;
}

int get_captured_piece(int move, const board* position) {
    int target_square = getMoveTarget(move);
    for (int i = (position->side^1)*6; i <= K + (position->side^1)*6; i++) {
        if (getBit(position->bitboards[i], target_square)) {
            return i;
        }
    }
    return -1;
}

int see(const board* pos, const int move) {
    int gain[32], d = 0;
    U64 pawns = pos->bitboards[P] | pos->bitboards[p];
    U64 diagnols = pos->bitboards[B] | pos->bitboards[Q] | pos->bitboards[b] | pos->bitboards[q];
    U64 horizontals = pos->bitboards[R] | pos->bitboards[Q] | pos->bitboards[r] | pos->bitboards[q];
    U64 sliders = diagnols | horizontals | pawns;
    U64 fromSet = 1ULL << getMoveSource(move);
    U64 occ = pos->occupancies[both];
    U64 attadef = get_attackers(pos, getMoveTarget(move));
    int piece = getMovePiece(move);
    if (piece == P && get_rank[getMoveTarget(move)] == 7) {
        piece = getMovePromoted(move);
        gain[d] = abs(material_score[opening][piece]) - abs(material_score[opening][P]);
    } else if (piece == p && get_rank[getMoveTarget(move)] == 1) {
        piece = getMovePromoted(move);
        gain[d] = abs(material_score[opening][piece]) - abs(material_score[opening][p]);
    } else {
        gain[d] = (get_captured_piece(move, pos)) != -1 ? abs(material_score[opening][get_captured_piece(move, pos)]) : 0;
        //printf("Gain sifir: %d\n", gain[d]);
    }
    if (getMoveEnpassant(move)) {
        gain[d] = material_score[opening][P];
    }
    int side = pos->side;
    do {
        d++;
        gain[d] = abs(material_score[opening][piece]) - gain[d - 1];
        //printf("Gain: %d D: %d because: %d - %d\n ", gain[d], d, material_score[opening][piece], gain[d - 1]);
        //if (max(-gain[d - 1], gain[d]) < 0) break;
        occ ^= fromSet;
        attadef ^= fromSet;
        if (fromSet & sliders) {
            if (piece == R || piece == r || piece == Q || piece == q) {
                attadef |= rookAttack(getMoveTarget(move), occ) & horizontals;
            }
            if (piece == P || piece == p || piece == B || piece == b || piece == Q || piece == q) {
                attadef |= bishopAttack(getMoveTarget(move), occ) & diagnols;
            }
            attadef ^= fromSet;
        }
        attadef &= occ;
        side ^= 1;
        fromSet = get_least_valuable_piece(attadef, side, &piece, pos);
        if (piece == P && get_rank[getMoveTarget(move)] == 7) {
            piece = Q;
        } else if (piece == p && get_rank[getMoveTarget(move)] == 0) {
            piece = q;
        }
    } while (fromSet);
    while (--d)
        gain[d-1]= -max(-gain[d-1], gain[d]);

    return gain[0];
}
