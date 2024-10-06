//
// Created by erena on 5.10.2024.
//

#include "see.h"

int moveEstimatedValue(board *position, int move) {
    // Start with the value of the piece on the target square
    int targetPiece =

}

int see(board *position, int move, int threshold) {
    int from, to, enpassant, promotion, colour, balance, nextVictim;
    U64 bishops, rooks, occupied, attackers, myAttackers;

    // unpack move information
    from = getMoveSource(move);
    to = getMoveTarget(move);
    enpassant = getMoveEnpassant(move);
    promotion = getMovePromoted(move);

    // Next victim is moved piece or promotion type
    //nextVictim = promotion ? promotion : position->



}

