//
// Created by erena on 13.09.2024.
//

#include "search.h"


// Static Exchange Evaluation
int QS_SEE_THRESHOLD = 0;
int SEE_MOVE_ORDER_THRESHOLD = -82;
int SEE_QUIET_THRESHOLD = -67;
int SEE_NOISY_THRESHOLD = -32;
int SEE_DEPTH = 10;

int lmr_full_depth_moves = 4;
int lmr_reduction_limit = 3;
int lateMovePruningBaseReduction = 4;
int nullMoveDepth = 3;

U64 searchNodes = 0;

int lmrTable[maxPly][maxPly];
int counterMoves[2][maxPly][maxPly];

const int SEEPieceValues[] = {100, 300, 300, 500, 1200, 0, 0};

int CORRHIST_WEIGHT_SCALE = 256;
int CORRHIST_GRAIN = 256;
int CORRHIST_SIZE = 16384;
int CORRHIST_MAX = 16384;

int pawnCorrectionHistory[2][16384];
int minorCorrectionHistory[2][16384];






// position repetition detection
int isRepetition(board* position) {
    // loop over repetition indicies range
    for (int index = 0; index < position->repetitionIndex; index++) {
        // if we found the hash key same with a current
        if (position->repetitionTable[index] == position->hashKey) {
            // we found a repetition
            return 1;
        }
    }

    // if no repetition found
    return 0;
}

// [depth][moveNumber]
void initializeLMRTable(void) {
    for (int depth = 1; depth < maxPly; ++depth) {
        for (int ply = 1; ply < maxPly; ++ply) {
            if (ply == 0 || depth == 0) {
                lmrTable[depth][ply] = 0;
                lmrTable[depth][ply] = 0;
                continue;
            }
            lmrTable[depth][ply] = round(0.75 + log(depth) * log(ply) * 0.375);
        }
    }
}

/*  =======================
         Move ordering
    =======================

    1. PV move
    2. Captures in MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
*/

// score moves
int scoreMove(int move, board* position) {
    // make sure we are dealing with PV move
    if (position->scorePv && position->pvTable[0][position->ply] == move) {
        // disable score PV flag
        position->scorePv = 0;

        // give PV move the highest score to search it first
        return 1500000000;
    }

    // score capture move
    if (getMoveCapture(move)) {
        int captureScore = 0;

        // init target piece
        int target_piece = P;

        uint8_t bb_piece = position->mailbox[getMoveTarget(move)];
        // if there's a piece on the target square
        if (bb_piece != NO_PIECE &&
            getBit(position->bitboards[bb_piece], getMoveTarget(move))) {
            target_piece = bb_piece;
        }

        // score move by MVV LVA lookup [source piece][target piece]
        captureScore += mvvLva[getMovePiece(move)][target_piece];

        captureScore += SEE(position, move, SEE_MOVE_ORDER_THRESHOLD) ? 1000000000 : -1000000;

        return captureScore;

    }
        // score quiet move
    else {

        // score 1st killer move
        if (position->killerMoves[position->ply][0] == move)
            return 900000000;
        /*
        // score 2nd killer move
        else if (position->killerMoves[position->ply][1] == move)
            return 800000000;
        else if (counterMoves[position->side][getMoveSource(move)][getMoveTarget(move)] == move)
            return 700000000;*/

        return quietHistory[getMoveSource(move)][getMoveTarget(move)] +
               (position->ply == 0 * rootHistory[position->side][getMoveSource(move)][getMoveTarget(move)] * 4);
    }
    return 0;
}


void sort_moves(moves *moveList, int tt_move, board* position) {
    // move scores
    int move_scores[moveList->count];
    int sorted_count = 0;

    // score and insert moves one by one
    for (int count = 0; count < moveList->count; count++) {
        int current_move = moveList->moves[count];
        int current_score;

        // if hash move available
        if (tt_move == current_move)
            current_score = 2000000000;
        else
        current_score = scoreMove(current_move, position);

        // Find the correct position to insert the current move
        int insert_pos = sorted_count;
        while (insert_pos > 0 && move_scores[insert_pos - 1] < current_score) {
            move_scores[insert_pos] = move_scores[insert_pos - 1];
            moveList->moves[insert_pos] = moveList->moves[insert_pos - 1];
            insert_pos--;
        }

        // Insert the current move and score
        move_scores[insert_pos] = current_score;
        moveList->moves[insert_pos] = current_move;

        sorted_count++;
    }
}

int quiescenceScoreMove(int move, board* position) {
    // score capture move
    if (getMoveCapture(move)) {
        // init target piece

        int target_piece = P;

        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;

        // pick up side to move
        if (position->side == white) {
            start_piece = p;
            end_piece = k;
        }
        else {
            start_piece = P;
            end_piece = K;
        }

        // loop over bitboards opposite to the current side to move
        for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
            // if there's a piece on the target square
            if (getBit(position->bitboards[bb_piece], getMoveTarget(move))) {
                // remove it from corresponding bitboard
                target_piece = bb_piece;
                break;
            }
        }

        // score move by MVV LVA lookup [source piece][target piece]
        return mvvLva[getMovePiece(move)][target_piece] + 1000000000;
    }

    return 0;
}

void quiescence_sort_moves(moves *moveList, board* position) {
    // move scores
    int move_scores[moveList->count];
    int sorted_count = 0;

    // score and insert moves one by one
    for (int count = 0; count < moveList->count; count++) {
        int current_move = moveList->moves[count];
        int current_score;

        // if hash move available
        /*if (bestMove == current_move)
            current_score = 2000000000;
        else*/
        current_score = quiescenceScoreMove(current_move, position);

        // Find the correct position to insert the current move
        int insert_pos = sorted_count;
        while (insert_pos > 0 && move_scores[insert_pos - 1] < current_score) {
            move_scores[insert_pos] = move_scores[insert_pos - 1];
            moveList->moves[insert_pos] = moveList->moves[insert_pos - 1];
            insert_pos--;
        }

        // Insert the current move and score
        move_scores[insert_pos] = current_score;
        moveList->moves[insert_pos] = current_move;

        sorted_count++;
    }
}



// enable PV move scoring
void enable_pv_scoring(moves *moveList, board* position) {
    // disable following PV
    position->followPv = 0;

    // loop over the moves within a move list
    for (int count = 0; count < moveList->count; count++) {
        // make sure we hit PV move
        if (position->pvTable[0][position->ply] == moveList->moves[count]) {
            // enable move scoring
            position->scorePv = 1;

            // enable following PV
            position->followPv = 1;
        }
    }
}

// print move
void printMove(int move) {
    if (getMovePromoted(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromoted(move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }
}


int getLmrReduction(int depth, int moveNumber) {
    int reduction = lmrTable[myMIN(63, depth)][myMIN(63, moveNumber)];
    return reduction;
}

void clearCounterMoves(void) {
    for (int m = 0;m < 2;m++) {
        for (int i = 0;i < 64;i++) {
            for (int j = 0;j < 64;j++) {
                counterMoves[m][i][j] = 0;
            }
        }
    }
}

void updatePawnCorrectionHistory(board *position, const int depth, const int diff) {
    U64 pawnKey = generatePawnKey(position);
    int entry = pawnCorrectionHistory[position->side][pawnKey % CORRHIST_SIZE];
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = myMIN(depth + 1, 16);
    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
    pawnCorrectionHistory[position->side][pawnKey % CORRHIST_SIZE] = entry;
}

void updateMinorCorrectionHistory(board *position, const int depth, const int diff) {
    U64 minorKey = generateMinorKey(position);
    int entry = minorCorrectionHistory[position->side][minorKey % CORRHIST_SIZE];
    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = myMIN(depth + 1, 16);
    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);
    minorCorrectionHistory[position->side][minorKey % CORRHIST_SIZE] = entry;
}

int adjustEvalWithCorrectionHistory(board *position, const int rawEval) {
    U64 pawnKey = generatePawnKey(position);
    U64 minorKey = generateMinorKey(position);
    int pawnEntry = pawnCorrectionHistory[position->side][pawnKey % CORRHIST_SIZE];
    int minorEntry = minorCorrectionHistory[position->side][minorKey % CORRHIST_SIZE];
    int mateFound = mateValue - maxPly;
    int adjust = pawnEntry + minorEntry;
    return clamp(rawEval + adjust / CORRHIST_GRAIN, -mateFound + 1, mateFound - 1);
}

uint8_t justPawns(board *pos) {
    return !((pos->bitboards[N] | pos->bitboards[n] | pos->bitboards[B] |
              pos->bitboards[b] | pos->bitboards[R] | pos->bitboards[r] |
              pos->bitboards[Q] | pos->bitboards[q]) &
             pos->occupancies[pos->side]);
}


int move_estimated_value(board *pos, int move) {

    // Start with the value of the piece on the target square
    int target_piece = pos->mailbox[getMoveTarget(move)] > 5
                       ? pos->mailbox[getMoveTarget(move)] - 6
                       : pos->mailbox[getMoveTarget(move)];
    int promoted_piece = getMovePromoted(move);
    promoted_piece = promoted_piece > 5 ? promoted_piece - 6 : promoted_piece;

    int value = SEEPieceValues[target_piece];

    // Factor in the new piece's value and remove our promoted pawn
    if (getMovePromoted(move))
        value += SEEPieceValues[promoted_piece] - SEEPieceValues[PAWN];

        // Target square is encoded as empty for enpass moves
    else if (getMoveEnpassant(move))
        value = SEEPieceValues[PAWN];

        // We encode Castle moves as KxR, so the initial step is wrong
    else if (getMoveCastling(move))
        value = 0;

    return value;
}

uint64_t all_attackers_to_square(board *pos, uint64_t occupied, int sq) {

    // When performing a static exchange evaluation we need to find all
    // attacks to a given square, but we also are given an updated occupied
    // bitboard, which will likely not match the actual board, as pieces are
    // removed during the iterations in the static exchange evaluation

    return (getPawnAttacks(white, sq) & pos->bitboards[p]) |
           (getPawnAttacks(black, sq) & pos->bitboards[P]) |
           (getKnightAttacks(sq) & (pos->bitboards[n] | pos->bitboards[N])) |
           (getBishopAttacks(sq, occupied) &
            ((pos->bitboards[b] | pos->bitboards[B]) |
             (pos->bitboards[q] | pos->bitboards[Q]))) |
           (getRookAttacks(sq, occupied) &
            ((pos->bitboards[r] | pos->bitboards[R]) |
             (pos->bitboards[q] | pos->bitboards[Q]))) |
           (getKingAttacks(sq) & (pos->bitboards[k] | pos->bitboards[K]));
}

int SEE(board *pos, int move, int threshold) {

    int from, to, enpassant, promotion, colour, balance, nextVictim;
    uint64_t bishops, rooks, occupied, attackers, myAttackers;

    // Unpack move information
    from = getMoveSource(move);
    to = getMoveTarget(move);
    enpassant = getMoveEnpassant(move);
    promotion = getMovePromoted(move);

    // Next victim is moved piece or promotion type
    nextVictim = promotion ? promotion : pos->mailbox[from];
    nextVictim = nextVictim > 5 ? nextVictim - 6 : nextVictim;

    // Balance is the value of the move minus threshold. Function
    // call takes care for Enpass, Promotion and Castling moves.
    balance = move_estimated_value(pos, move) - threshold;

    // Best case still fails to beat the threshold
    if (balance < 0)
        return 0;

    // Worst case is losing the moved piece
    balance -= SEEPieceValues[nextVictim];

    // If the balance is positive even if losing the moved piece,
    // the exchange is guaranteed to beat the threshold.
    if (balance >= 0)
        return 1;

    // Grab sliders for updating revealed attackers
    bishops = pos->bitboards[b] | pos->bitboards[B] | pos->bitboards[q] |
              pos->bitboards[Q];
    rooks = pos->bitboards[r] | pos->bitboards[R] | pos->bitboards[q] |
            pos->bitboards[Q];

    // Let occupied suppose that the move was actually made
    occupied = pos->occupancies[both];
    occupied = (occupied ^ (1ull << from)) | (1ull << to);
    if (enpassant)
        occupied ^= (1ull << pos->enpassant);

    // Get all pieces which attack the target square. And with occupied
    // so that we do not let the same piece attack twice
    attackers = all_attackers_to_square(pos, occupied, to) & occupied;

    // Now our opponents turn to recapture
    colour = pos->side ^ 1;

    while (1) {

        // If we have no more attackers left we lose
        myAttackers = attackers & pos->occupancies[colour];
        if (myAttackers == 0ull) {
            break;
        }

        // Find our weakest piece to attack with
        for (nextVictim = PAWN; nextVictim <= QUEEN; nextVictim++) {
            if (myAttackers &
                (pos->bitboards[nextVictim] | pos->bitboards[nextVictim + 6])) {
                break;
            }
        }

        // Remove this attacker from the occupied
        occupied ^=
                (1ull << getLS1BIndex(myAttackers & (pos->bitboards[nextVictim] |
                                                     pos->bitboards[nextVictim + 6])));

        // A diagonal move may reveal bishop or queen attackers
        if (nextVictim == PAWN || nextVictim == BISHOP || nextVictim == QUEEN)
            attackers |= getBishopAttacks(to, occupied) & bishops;

        // A vertical or horizontal move may reveal rook or queen attackers
        if (nextVictim == ROOK || nextVictim == QUEEN)
            attackers |= getRookAttacks(to, occupied) & rooks;

        // Make sure we did not add any already used attacks
        attackers &= occupied;

        // Swap the turn
        colour = !colour;

        // Negamax the balance and add the value of the next victim
        balance = -balance - 1 - SEEPieceValues[nextVictim];

        // If the balance is non negative after giving away our piece then we win
        if (balance >= 0) {

            // As a slide speed up for move legality checking, if our last attacking
            // piece is a king, and our opponent still has attackers, then we've
            // lost as the move we followed would be illegal
            if (nextVictim == KING && (attackers & pos->occupancies[colour]))
                colour = colour ^ 1;

            break;
        }
    }

    // Side to move after the loop loses
    return pos->side != colour;
}


uint8_t isMaterialDraw(board *pos) {
    uint8_t piece_count = countBits(pos->occupancies[both]);

    // K v K
    if (piece_count == 2) {
        return 1;
    }
    // Initialize knight and bishop count only after we check that piece count is
    // higher then 2 as there cannot be a knight or bishop with 2 pieces on the
    // board
    uint8_t knight_count =
            countBits(pos->bitboards[n] | pos->bitboards[N]);
    // KN v K || KB v K
    if (piece_count == 3 &&
        (knight_count == 1 ||
                countBits(pos->bitboards[b] | pos->bitboards[B]) == 1)) {
        return 1;
    } else if (piece_count == 4) {
        // KNN v K || KN v KN
        if (knight_count == 2) {
            return 1;
        }
            // KB v KB
        else if (countBits(pos->bitboards[b]) == 1 &&
                countBits(pos->bitboards[B]) == 1) {
            return 1;
        }
    }
    return 0;
}

// quiescence search
int quiescence(int alpha, int beta, board* position, time* time) {
    if ((searchNodes & 2047) == 0) {
        communicate(time);
    }

    int score = 0, bestScore = 0;

    //int pvNode = beta - alpha > 1;

    //int rootNode = position->ply == 0;

    // best move (to store in TT)
    //int bestMove = 0;


    int bestMove = 0;
    int tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;

    // read hash entry
    if (position->ply &&
        (tt_hit =
                 readHashEntry(position, &tt_move, &tt_score, &tt_depth, &tt_flag))) {
        if ((tt_flag == hashFlagExact) ||
            ((tt_flag == hashFlagBeta) && (tt_score <= alpha)) ||
            ((tt_flag == hashFlagAlpha) && (tt_score >= beta))) {
            return tt_score;
        }
    }

    // evaluate position
    int evaluation = evaluate(position);

    evaluation = adjustEvalWithCorrectionHistory(position, evaluation);

    score = bestScore = tt_hit ? tt_score : evaluation;

    // fail-hard beta cutoff
    if (evaluation >= beta) {
        // node (move) fails high
        return evaluation;
    }


    // found a better move
    if (evaluation > alpha) {
        // PV node (move)
        alpha = evaluation;
    }

    // create move list instance
    moves moveList[1];

    // generate moves
    noisyGenerator(moveList, position);

    // sort moves
    quiescence_sort_moves(moveList, position);

    // legal moves counter
    //int legal_moves = 0;


    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {

        if (!SEE(position, moveList->moves[count], QS_SEE_THRESHOLD)) {
            continue;
        }
        struct copyposition copyPosition;
        // preserve board state
        copyBoard(position, &copyPosition);

        // increment ply
        position->ply++;

        // increment repetition index & store hash key
        position->repetitionIndex++;
        position->repetitionTable[position->repetitionIndex] = position->hashKey;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], onlyCaptures, position) == 0) {
            // decrement ply
            position->ply--;

            // decrement repetition index
            position->repetitionIndex--;

            // skip to next move
            continue;
        }

        //legal_moves++;

        // increment nodes count
        searchNodes++;

        // score current move
        score = -quiescence(-beta, -alpha, position, time);

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // take move back
        takeBack(position, &copyPosition);

        if (time->stopped == 1) return 0;


        if (score > bestScore) {
            bestScore = score;
            // found a better move
            if (score > alpha) {
                //bestMove = moveList->moves[count];

                //hashFlag = hashFlagExact;
                alpha = score;
            }

            if (score >= beta) {
                //writeHashEntry(beta, bestMove, 0, hashFlagBeta, position);
                // node (move) fails high
                break;
            }
        }
    }

    uint8_t hashFlag = hashFlagNone;
    if (alpha >= beta) {
        hashFlag = hashFlagAlpha;
    } else {
        hashFlag = hashFlagBeta;
    }


    // store hash entry with the score equal to alpha
    writeHashEntry(bestScore, bestMove, 0, hashFlag, position);

    // node (move) fails low
    return bestScore;
}


// negamax alpha beta search
int negamax(int alpha, int beta, int depth, board* position, time* time, bool cutNode) {
    // init PV length
    position->pvLength[position->ply] = position->ply;


    // variable to store current move's score (from the static evaluation perspective)
    int score = 0;





    if ((searchNodes & 2047) == 0) {
        communicate(time);
    }


    int pvNode = beta - alpha > 1;

    int rootNode = position->ply == 0;

    int bestMove = 0;
    int tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;

    if (!rootNode) {

        if (isRepetition(position) || isMaterialDraw(position)) {
            return 0;
        }


        // Mate distance pruning
        alpha = myMAX(alpha, -mateValue + (int)position->ply);
        beta = myMIN(beta, mateValue - (int)position->ply - 1);
        if (alpha >= beta)
            return alpha;
    }

    // read hash entry
    if (!rootNode &&
        (tt_hit =
                 readHashEntry(position, &tt_move, &tt_score, &tt_depth, &tt_flag))) {
        if (tt_depth >= depth) {
            if ((tt_flag == hashFlagExact) ||
                ((tt_flag == hashFlagBeta) && (tt_score <= alpha)) ||
                ((tt_flag == hashFlagAlpha) && (tt_score >= beta))) {
                return tt_score;
            }
        }
    }


    // recursion escapre condition
    if (depth <= 0)
        // run quiescence search
        return quiescence(alpha, beta, position, time);


    // is king in check
    int in_check = isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[K]) :
                                    getLS1BIndex(position->bitboards[k]),
                                    position->side ^ 1, position);


    // get static evaluation score
    int raw_eval = evaluate(position);

    int static_eval = adjustEvalWithCorrectionHistory(position, raw_eval);

    bool improving = false;

    int pastStack = -1;

    position->staticEval[position->ply] = static_eval;

    position->improvingRate[position->ply] = 0.0;

    if (position->ply >= 2 && position->staticEval[position->ply-2] != noEval) {
        pastStack = position->ply - 2;
    } else if (position->ply >= 4 && position->staticEval[position->ply-4] != noEval) {
        pastStack = position->ply - 4;
    }

    if (pastStack > -1 && !in_check) {
        improving = position->staticEval[position->ply] > position->staticEval[pastStack];
        const double diff = position->staticEval[position->ply] - position->staticEval[pastStack];
        position->improvingRate[position->ply] = fmin(fmax(position->improvingRate[position->ply] + diff / 50, (-1.0)), 1.0);
    }

    // Internal Iterative Reductions
    if ((pvNode || cutNode || !improving) && depth >= 8 && !tt_move) {
        depth--;
    }

    bool canPrune = in_check == 0 && pvNode == 0;

    uint16_t rfpMargin = improving ? 65 * (depth - 1) : 82 * depth;

    // reverse futility pruning
    if (depth <= 5 && !pvNode && !in_check && static_eval - rfpMargin >= beta)
        return static_eval;

    // null move pruning
    if (depth >= nullMoveDepth && in_check == 0 && !rootNode &&
            static_eval >= beta &&
            !justPawns(position)) {
        struct copyposition copyPosition;
        // preserve board state
        copyBoard(position, &copyPosition);

        position->ply++;

        // increment repetition index & store hash key
        position->repetitionIndex++;
        position->repetitionTable[position->repetitionIndex] = position->hashKey;

        // hash enpassant if available
        if (position->enpassant != no_sq) { position->hashKey ^= enpassantKeys[position->enpassant]; }

        // reset enpassant capture square
        position->enpassant = no_sq;

        // switch the side, literally giving opponent an extra move to make
        position->side ^= 1;

        // hash the side
        position->hashKey ^= sideKey;

        int R = 3 + depth / 3;

        /* search moves with reduced depth to find beta cutoffs
           depth - R where R is a reduction limit */
        score = -negamax(-beta, -beta + 1, depth - R, position, time, !cutNode);

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // restore board state
        takeBack(position, &copyPosition);


        if (time->stopped == 1) return 0;

        // fail-hard beta cutoff
        if (score >= beta)

            // node (move) fails high
            return score;
    }

    // razoring
    if (canPrune && depth <= 3 && static_eval + 200 * depth < alpha) {
        int razoringScore = quiescence(alpha, beta, position, time);
        if (razoringScore <= alpha) {
            return razoringScore;
        }
    }

    // create move list instance
    moves moveList[1], badQuiets[1];
    badQuiets->count = 0;

    // generate moves
    moveGenerator(moveList, position);

    // if we are now following PV line
    if (position->followPv)
        // enable PV move scoring
        enable_pv_scoring(moveList, position);

    // sort moves
    sort_moves(moveList, tt_move, position);

    // number of moves searched in a move list
    int moves_searched = 0;

    int bestScore = -infinity;

    bool skipQuiet = false;

    // legal moves counter
    int legal_moves = 0;

    // quiet move counter
    int quietMoves = 0;

    // capture move counter
    //int captureMoves = 0;

    const int originalAlpha = alpha;

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        int currentMove = moveList->moves[count];

        bool isQuiet = getMoveCapture(currentMove) == 0;


        if (skipQuiet && isQuiet) {
            skipQuiet = 0;
            continue;
        }



        int moveHistory = quietHistory[getMoveSource(currentMove)][getMoveTarget(currentMove)];

        bool isNotMated = alpha > -mateScore + maxPly;

        if (!rootNode && isQuiet && isNotMated) {

            int lmpBase = 4;
            int lmpMultiplier = 3;
            int lmpThreshold = (lmpBase + lmpMultiplier * (depth - 1) * (depth - 1));

            if (legal_moves>= lmpThreshold) {
                skipQuiet = 1;
            }

            if (canPrune && depth <= 4 && static_eval + 82 * depth <= alpha) {
                skipQuiet = 1;
            }

            // Quiet History Pruning
            if (canPrune && depth <= 2 && moveHistory < depth * -2048) {
                break;
            }
        }

        // SEE PVS Pruning
        int seeThreshold =
                isQuiet ? -67 * depth : -32 * depth * depth;
        if (depth <= 10 && legal_moves > 0 && !SEE(position, currentMove, seeThreshold))
            continue;

        struct copyposition copyPosition;
        // preserve board state
        copyBoard(position, &copyPosition);

        // increment ply
        position->ply++;

        // increment repetition index & store hash key
        position->repetitionIndex++;
        position->repetitionTable[position->repetitionIndex] = position->hashKey;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], allMoves, position) == 0) {
            // decrement ply
            position->ply--;

            // decrement repetition index
            position->repetitionIndex--;

            // skip to next move
            continue;
        }

        // increment nodes count
        searchNodes++;

        if (isQuiet) {
            addMoveToHistoryList(badQuiets, currentMove);
        }

        // increment legal moves
        legal_moves++;

        if (isQuiet) {
            quietMoves++;
        } else {
            //captureMoves++;
        }




        // full-depth search
        if (moves_searched == 0) {
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1, position, time, false);
        } else {
            int lmrReduction = getLmrReduction(depth, legal_moves);

            /* All Moves */


            // Reduce More
            if (!improving) {
                lmrReduction += 1;
            }


            if (isQuiet) {

                // Reduce More
                if (!pvNode && quietMoves >= 4) {
                    lmrReduction += 1;
                }

            }


            // condition to consider LMR
            if(moves_searched >= lmr_full_depth_moves &&
               depth >= lmr_reduction_limit) {
                // search current move with reduced depth:
                score = -negamax(-alpha - 1, -alpha, depth - lmrReduction, position, time, true);
            } else {
                // hack to ensure that full-depth search is done
                score = alpha + 1;
            }


            // principle variation search PVS
            if (score > alpha) {
                /* Once you've found a move with a score that is between alpha and beta,
                   the rest of the moves are searched with the goal of proving that they are all bad.
                   It's possible to do this a bit faster than a search that worries that one
                   of the remaining moves might be good. */
                score = -negamax(-alpha - 1, -alpha, depth - 1, position, time, false);

                /* If the algorithm finds out that it was wrong, and that one of the
                   subsequent moves was better than the first PV move, it has to search again,
                   in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                   but generally not often enough to counteract the savings gained from doing the
                   "bad move proof" search referred to earlier. */
                if((score > alpha) && (score < beta))
                    /* re-search the move that has failed to be proved to be bad
                       with normal alpha beta score bounds*/
                    score = -negamax(-beta, -alpha, depth - 1, position, time, false);
            }

        }

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // take move back
        takeBack(position, &copyPosition);

        if (time->stopped == 1) return 0;

        // increment the counter of moves searched so far
        moves_searched++;

        // found a better move
        if (score > bestScore) {
            bestScore = score;

            if (score > alpha) {
                // store best move (for TT or anything)
                bestMove = currentMove;

                // PV node (move)
                alpha = score;

                if (pvNode) {
                    // write PV move
                    position->pvTable[position->ply][position->ply] = currentMove;

                    // loop over the next ply
                    for (int next_ply = position->ply + 1; next_ply < position->pvLength[position->ply + 1]; next_ply++)
                        // copy move from deeper ply into a current ply's line
                        position->pvTable[position->ply][next_ply] = position->pvTable[position->ply + 1][next_ply];

                    // adjust PV length
                    position->pvLength[position->ply] = position->pvLength[position->ply + 1];
                }


                // fail-hard beta cutoff
                if (score >= beta) {
                    if (isQuiet) {
                        // store killer moves
                        position->killerMoves[position->ply][0] = bestMove;
                        updateQuietMoveHistory(bestMove, depth, badQuiets);

                        if (rootNode) {
                            updateRootHistory(position, bestMove, depth, badQuiets);
                        }

                    }

                    // node (move) fails high
                    break;
                }
            }
        }
    }

    // we don't have any legal moves to make in the current postion
    if (legal_moves == 0) {
        // king is in check
        if (in_check)
            // return mating score (assuming closest distance to mating position)
            return -mateValue + position->ply;

            // king is not in check
        else
            // return stalemate score
            return 0;
    }

    uint8_t hashFlag = hashFlagExact;
    if (alpha >= beta) {
        hashFlag = hashFlagAlpha;
    } else if (alpha <= originalAlpha) {
        hashFlag = hashFlagBeta;
    }

    if (!in_check && (bestMove == 0 || !getMoveCapture(bestMove)) &&
    !(hashFlag == hashFlagAlpha && bestScore <= static_eval) &&
    !(hashFlag == hashFlagBeta && bestScore >= static_eval)) {
        updatePawnCorrectionHistory(position, depth, bestScore - static_eval);
        updateMinorCorrectionHistory(position, depth, bestScore - static_eval);
    }

    // store hash entry with the score equal to alpha
    writeHashEntry(bestScore, bestMove, depth, hashFlag, position);


    // node (move) fails low
    return bestScore;
}

// search position for the best move
void searchPosition(int depth, board* position, bool benchmark, time* time) {
    // define best score variable
    int score = 0;

    // reset "time is up" flag
    time->stopped = 0;

    // reset nodes counter
    searchNodes = 0;

    // reset follow PV flags
    position->followPv = 0;
    position->scorePv = 0;

    memset(position->killerMoves, 0, sizeof(position->killerMoves));
    memset(quietHistory, 0, sizeof(quietHistory));
    memset(rootHistory, 0, sizeof(rootHistory));
    memset(pawnCorrectionHistory, 0, sizeof(pawnCorrectionHistory));
    memset(minorCorrectionHistory, 0, sizeof(pawnCorrectionHistory));
    memset(position->pvTable, 0, sizeof(position->pvTable));
    memset(position->pvLength, 0, sizeof(position->pvLength));
    memset(position->staticEval, 0, sizeof(position->staticEval));
    //memset(time, 0, sizeof(*time));
    //memset(counterMoves, 0, sizeof(counterMoves));

    // define initial alpha beta bounds
    int alpha = -infinity;
    int beta = infinity;

    int totalTime = 0;

    int previousBestMove = 0;
    uint8_t bestMoveStability = 0;
    int averageScore = noEval;
    uint8_t evalStability = 0;

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (time->stopped == 1) {
            break;
        }

        int startTime = getTimeMiliSecond();

        if (time->timeset && startTime >= time->softLimit) {
            time->stopped = 1;
        }

        int window = 9;

        while (true) {

            if (time->timeset && startTime >= time->softLimit) {
                time->stopped = 1;
            }

            if (time->stopped == 1) {
                break;
            }

            if (current_depth >= 4) {
                alpha = myMAX(-infinity, score - window);
                beta = myMIN(infinity, score + window);
            }

            position->followPv = 1;
            // find best move within a given position
            score = negamax(alpha, beta, current_depth, position, time, false);


            if (score == infinity) {
                // Restore the saved best line
                memset(position->pvTable, 0, sizeof(position->pvTable));
                memset(position->pvLength, 0, sizeof(position->pvLength));
                // Break out of the loop without printing info about the unfinished
                // depth
                break;
            }
            if (score <= alpha) {
                alpha = myMAX(-infinity, alpha - window);
            }

            else if (score >= beta) {
                beta = myMIN(infinity, beta + window);

            } else {
                break;
            }
            window *= 1.8f;

        }

        averageScore = averageScore == noEval ? score : (averageScore + score) / 2;


        if (position->pvTable[0][0] == previousBestMove) {
            bestMoveStability = myMIN(bestMoveStability + 1, 4);
        } else {
            previousBestMove = position->pvTable[0][0];
            bestMoveStability = 0;
        }

        if (score > averageScore - 10 && score < averageScore + 10) {
            evalStability = myMIN(evalStability + 1, 4);
        } else {
            evalStability = 0;
        }


        if (time->timeset && current_depth > 6) {
            scaleTime(time, bestMoveStability, evalStability);
        }

        int endTime = getTimeMiliSecond();
        totalTime += endTime - startTime;

        if (position->pvLength[0] && !benchmark) {
            unsigned long long nps = (totalTime > 0) ? (searchNodes * 1000) / totalTime : 0;

            printf("info depth %d ", current_depth);

            if (score > -mateValue && score < -mateScore)
                printf("score mate %d nodes %llu nps %llu time %d pv ",
                       -(score + mateValue) / 2 - 1,
                       searchNodes, nps, totalTime);
            else if (score > mateScore && score < mateValue)
                printf("score mate %d nodes %llu nps %llu time %d pv ",
                       (mateValue - score) / 2 + 1,
                       searchNodes, nps, totalTime);
            else
                printf("score cp %d nodes %llu nps %llu time %d pv ",
                       score, searchNodes, nps, totalTime);

            // loop over the moves within a PV line
            for (int count = 0; count < position->pvLength[0]; count++) {
                printMove(position->pvTable[0][count]);
                printf(" ");
            }
            // print new line
            printf("\n");
        }

    }
    if (!benchmark) {
        // best move placeholder
        printf("bestmove ");
        printMove(position->pvTable[0][0]);
        printf("\n");
    }

}
