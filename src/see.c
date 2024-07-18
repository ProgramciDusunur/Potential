//
// Created by erena on 31.05.2024.
//


#include "see.h"
#include "board_constants.h"
#include "evaluation.h"
#include "bit_manipulation.h"
#include "move.h"
#include "mask.h"
#include <stdlib.h>

int anyPassPawnFree(const board* pos, int side, U64 attackBoard) {
    if (side) {
        int blackPieceCount = countBits((pos->bitboards[p] |  pos->bitboards[b] | pos->bitboards[n] |
                                         pos->bitboards[r] | pos->bitboards[q] | pos->bitboards[k])  & attackBoard);
        int whitePassPawns = countBits(pos->bitboards[P] & attackBoard);
        //printf("siyahlarin hedef kareye bakan toplam tas sayisi: %d ve beyazlarin hedef kareye bakan toplam gecer piyon sayisi: %d\n", blackPieceCount, whitePassPawns);
        if (blackPieceCount <= whitePassPawns) {
            return 1;
        }
    } else {
        int whitePieceCount = countBits((pos->bitboards[P] |  pos->bitboards[B] | pos->bitboards[N] |
                                         pos->bitboards[R] | pos->bitboards[Q] | pos->bitboards[K])  & attackBoard);
        int blackPassPawns = countBits(pos->bitboards[p] & attackBoard);
        //printf("beyazlarin hedef kareye bakan toplam tas sayisi: %d ve siyahlarin hedef kareye bakan toplam gecer piyon sayisi: %d\n", whitePieceCount, blackPassPawns);
        if (whitePieceCount <= blackPassPawns) {
            return 1;
        }
    }

    return 0;
}


U64 get_least_valuable_piece(U64 attadef, int side, int *targetPiece, const board* pos) {
    //pBitboard(attadef);
    //printf("Sira su tarafta: %d\n", side);
    for (int p_trav = P; p_trav <= K; p_trav++) {
        int offset = side * 6;
        U64 bb = pos->bitboards[p_trav + offset] & attadef;

        if (bb) {

            *targetPiece = p_trav + offset;
            if (anyPassPawnFree(pos, side,attadef)) {
                //printf("Hedef tas bir siyah fil! \n");
                //anyPassPawnFree(pos, side,attadef);

                return 0;
            }
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
    U64 attadef = get_attackers(pos, getMoveTarget(move), !pos->side);



    int piece = getMovePiece(move);

    if (piece == P && get_rank[getMoveTarget(move)] == 7) {
        piece = getMovePromoted(move);
        gain[d] = abs(seeMaterial[piece]) - abs(seeMaterial[P]);
        //printf("piyon terfi etti: %d\n", getMovePromoted(move));
    } else if (piece == p && get_rank[getMoveTarget(move)] == 1) {
        piece = getMovePromoted(move);
        gain[d] = abs(seeMaterial[piece]) - abs(seeMaterial[p]);
    } else {
        gain[d] = (get_captured_piece(move, pos)) != -1 ? abs(seeMaterial[get_captured_piece(move, pos)]) : 0;
        //printf("Gain sifir: %d\n", gain[d]);
    }

    if (getMoveEnpassant(move)) {
        gain[d] = seeMaterial[P];
    }

    int side = pos->side;

    do {
        d++;
        gain[d] = abs(seeMaterial[piece]) - gain[d - 1];
        //if (max(-gain[d - 1], gain[d]) < 0) break;
        //printf("Gain: %d D: %d because: %d - %d\n ", gain[d], d, seeMaterial[piece], gain[d - 1]);


        // remove calculated piece from occupancy board
        occ ^= fromSet;
        // remove calculated piece from attack board
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

