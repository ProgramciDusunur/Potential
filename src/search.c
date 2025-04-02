//
// Created by erena on 13.09.2024.
//

#include "search.h"


/* Tunable Search Parameters */

// Static Exchange Evaluation
int SEE_PIECE_VALUES[] = {100, 300, 300, 500, 1200, 0, 0};
int QS_SEE_THRESHOLD = 0;
int SEE_MOVE_ORDERING_THRESHOLD = -82;
int SEE_QUIET_THRESHOLD = -67;
int SEE_NOISY_THRESHOLD = -32;
int SEE_DEPTH = 10;

// Null Move Pruning
int NMP_DEPTH = 3;
int NMP_BASE_REDUCTION = 3;
int NMP_REDUCTION_DEPTH_DIVISOR = 3;
int NMP_EVAL_DIVISOR = 400;

// Late Move Reduction
int LMR_TABLE[2][maxPly][maxPly];
int LMR_FULL_DEPTH_MOVES = 4;
int LMR_REDUCTION_LIMIT = 3;
int DEEPER_LMR_MARGIN = 35;
double LMR_TABLE_BASE_NOISY = 0.38;
double LMR_TABLE_NOISY_DIVISOR = 3.76;
double LMR_TABLE_BASE_QUIET = 1.01;
double LMR_TABLE_QUIET_DIVISOR = 2.32;


// Late Move Pruning
int LMP_BASE = 4;
int LMP_MULTIPLIER = 3;

// Futility Pruning
int FUTILITY_PRUNING_OFFSET[] = {0, 82, 41, 20, 10, 5};
int FP_DEPTH = 5;
int FP_MARGIN = 82;

// Reverse Futility Pruning
int RFP_MARGIN = 82;
int RFP_IMPROVING_MARGIN = 65;
int RFP_DEPTH = 6;

// Razoring
int RAZORING_DEPTH = 3;
int RAZORING_MARGIN = 200;

// Singular Extensions
int SE_DEPTH = 7;
int SEE_TT_DEPTH_SUBTRACTOR = 3;
int DOUBLE_EXTENSION_MARGIN = 20;
int TRIPLE_EXTENSION_MARGIN = 40;

// Correction History
int CORRHIST_WEIGHT_SCALE = 256;
int CORRHIST_GRAIN = 256;
int CORRHIST_LIMIT = 1024;
int CORRHIST_SIZE = 16384;
int CORRHIST_MAX = 16384;

int PAWN_CORRECTION_HISTORY[2][16384];
int MINOR_CORRECTION_HISTORY[2][16384];
int NON_PAWN_CORRECTION_HISTORY[2][2][16384];

// Internal Iterative Reductions
int IIR_DEPTH = 8;
int IIR_TT_DEPTH_SUBTRACTOR = 3;


U64 searchNodes = 0;

int counterMoves[2][maxPly][maxPly];



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
        for (int moves = 1; moves < maxPly; ++moves) {
            if (moves == 0 || depth == 0) {
                LMR_TABLE[0][depth][moves] = 0;
                LMR_TABLE[1][depth][moves] = 0;
                continue;
            }
            LMR_TABLE[0][depth][moves] = LMR_TABLE_BASE_NOISY + log(depth) * log(moves) / LMR_TABLE_NOISY_DIVISOR; // noisy/tactical
            LMR_TABLE[1][depth][moves] = LMR_TABLE_BASE_QUIET + log(depth) * log(moves) / LMR_TABLE_QUIET_DIVISOR; // quiet
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

        captureScore += SEE(position, move, SEE_MOVE_ORDERING_THRESHOLD) ? 1000000000 : -1000000;

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

        return quietHistory[position->side][getMoveSource(move)][getMoveTarget(move)] +
                getContinuationHistoryScore(position, 1, move) +
                    getContinuationHistoryScore(position, 2, move) +
                        getContinuationHistoryScore(position, 4, move) +
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

int getLmrReduction(int depth, int moveNumber, bool isQuiet) {
    return LMR_TABLE[isQuiet][myMIN(63, depth)][myMIN(63, moveNumber)];
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
    U64 pawnKey = position->pawnKey;

    int entry = PAWN_CORRECTION_HISTORY[position->side][pawnKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 2 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    PAWN_CORRECTION_HISTORY[position->side][pawnKey % CORRHIST_SIZE] = entry;
}

void updateMinorCorrectionHistory(board *position, const int depth, const int diff) {
    U64 minorKey = position->minorKey;

    int entry = MINOR_CORRECTION_HISTORY[position->side][minorKey % CORRHIST_SIZE];

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 2 * myMIN(depth + 1, 16);

    entry = (entry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    entry = clamp(entry, -CORRHIST_MAX, CORRHIST_MAX);

    MINOR_CORRECTION_HISTORY[position->side][minorKey % CORRHIST_SIZE] = entry;
}

void update_non_pawn_corrhist(board *position, const int depth, const int diff) {
    U64 whiteKey = position->whiteNonPawnKey;
    U64 blackKey = position->blackNonPawnKey;

    const int scaledDiff = diff * CORRHIST_GRAIN;
    const int newWeight = 2 * myMIN(depth + 1, 16);

    int whiteEntry = NON_PAWN_CORRECTION_HISTORY[white][position->side][whiteKey % CORRHIST_SIZE];

    whiteEntry = (whiteEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    whiteEntry = clamp(whiteEntry, -CORRHIST_MAX, CORRHIST_MAX);

    int blackEntry = NON_PAWN_CORRECTION_HISTORY[black][position->side][blackKey % CORRHIST_SIZE];

    blackEntry = (blackEntry * (CORRHIST_WEIGHT_SCALE - newWeight) + scaledDiff * newWeight) / CORRHIST_WEIGHT_SCALE;
    blackEntry = clamp(blackEntry, -CORRHIST_MAX, CORRHIST_MAX);

    NON_PAWN_CORRECTION_HISTORY[white][position->side][whiteKey % CORRHIST_SIZE] = whiteEntry;
    NON_PAWN_CORRECTION_HISTORY[black][position->side][blackKey % CORRHIST_SIZE] = blackEntry;
}

int adjustEvalWithCorrectionHistory(board *position, const int rawEval) {
    U64 pawnKey = position->pawnKey;
    U64 minorKey = position->minorKey;

    int pawnEntry = PAWN_CORRECTION_HISTORY[position->side][pawnKey % CORRHIST_SIZE];
    int minorEntry = MINOR_CORRECTION_HISTORY[position->side][minorKey % CORRHIST_SIZE];

    U64 whiteNPKey = position->whiteNonPawnKey;
    int whiteNPEntry = NON_PAWN_CORRECTION_HISTORY[white][position->side][whiteNPKey % CORRHIST_SIZE];

    U64 blackNPKey = position->blackNonPawnKey;
    int blackNPEntry = NON_PAWN_CORRECTION_HISTORY[black][position->side][blackNPKey % CORRHIST_SIZE];

    int mateFound = mateValue - maxPly;

    int adjust = pawnEntry + minorEntry + whiteNPEntry + blackNPEntry;

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

    int value = SEE_PIECE_VALUES[target_piece];

    // Factor in the new piece's value and remove our promoted pawn
    if (getMovePromoted(move))
        value += SEE_PIECE_VALUES[promoted_piece] - SEE_PIECE_VALUES[PAWN];

        // Target square is encoded as empty for enpass moves
    else if (getMoveEnpassant(move))
        value = SEE_PIECE_VALUES[PAWN];

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
    balance -= SEE_PIECE_VALUES[nextVictim];

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
        balance = -balance - 1 - SEE_PIECE_VALUES[nextVictim];

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

    int pvNode = beta - alpha > 1;

    //int rootNode = position->ply == 0;


    int bestMove = 0;
    int tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;
    bool tt_pv = pvNode;

    // read hash entry
    if (position->ply &&
        (tt_hit =
                 readHashEntry(position, &tt_move, &tt_score, &tt_depth, &tt_flag, &tt_pv))) {
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
    if (moveList->count > 0) {
        quiescence_sort_moves(moveList, position);
    }




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

        prefetch_hash_entry(position->hashKey);

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
    writeHashEntry(bestScore, bestMove, 0, hashFlag, tt_pv, position);

    // node (move) fails low
    return bestScore;
}


// negamax alpha beta search
int negamax(int alpha, int beta, int depth, board* pos, time* time, bool cutNode) {
    // init PV length
    pos->pvLength[pos->ply] = pos->ply;


    // variable to store current move's score (from the static evaluation perspective)
    int score = 0;





    if ((searchNodes & 2047) == 0) {
        communicate(time);
    }


    int pvNode = beta - alpha > 1;

    int rootNode = pos->ply == 0;

    int bestMove = 0;
    int tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;
    bool tt_pv = pvNode;

    if (!rootNode) {

        if (isRepetition(pos) || isMaterialDraw(pos)) {
            return 0;
        }


        // Mate distance pruning
        alpha = myMAX(alpha, -mateValue + (int)pos->ply);
        beta = myMIN(beta, mateValue - (int)pos->ply - 1);
        if (alpha >= beta)
            return alpha;
    }

    // read hash entry
    if (!pos->isSingularMove[pos->ply] && !rootNode &&
        (tt_hit =
                readHashEntry(pos, &tt_move, &tt_score, &tt_depth, &tt_flag, &tt_pv)) &&
                !pvNode) {
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
        return quiescence(alpha, beta, pos, time);


    // is king in check
    int in_check = isSquareAttacked((pos->side == white) ? getLS1BIndex(pos->bitboards[K]) :
                                    getLS1BIndex(pos->bitboards[k]),
                                    pos->side ^ 1, pos);

    // get static evaluation score
    int raw_eval = evaluate(pos);

    int static_eval = adjustEvalWithCorrectionHistory(pos, raw_eval);

    bool improving = false;

    int pastStack = -1;

    pos->staticEval[pos->ply] = static_eval;

    pastStack = pos->ply >= 2 && pos->staticEval[pos->ply - 2] != noEval  ?  pos->ply - 2 : -1;

    improving = pastStack > -1 && !in_check && pos->staticEval[pos->ply] > pos->staticEval[pastStack];

    // Internal Iterative Reductions
    if ((pvNode || cutNode) && depth >= IIR_DEPTH && (!tt_move || tt_depth < depth - IIR_TT_DEPTH_SUBTRACTOR)) {
        depth--;
    }


    int ttAdjustedEval = static_eval;

    if (!pos->isSingularMove[pos->ply] && tt_move && !in_check &&
        (tt_flag == hashFlagExact ||
         (tt_flag == hashFlagAlpha && tt_score >= static_eval) ||
         (tt_flag == hashFlagBeta && tt_score <= static_eval))) {

        ttAdjustedEval = tt_score;
    }

    uint16_t rfpMargin = improving ? RFP_IMPROVING_MARGIN * (depth - 1) : RFP_MARGIN * depth;

    // reverse futility pruning
    if (!pos->isSingularMove[pos->ply] &&
        depth <= RFP_DEPTH && !pvNode && !in_check && (!tt_hit || ttAdjustedEval != static_eval) &&
        ttAdjustedEval - rfpMargin >= beta)
        return ttAdjustedEval;

    // null move pruning
    if (!pos->isSingularMove[pos->ply] && !pvNode &&
        depth >= NMP_DEPTH && !in_check && !rootNode &&
            static_eval >= beta &&
            pos->ply >= pos->nmpPly &&
            !justPawns(pos)) {
        struct copyposition copyPosition;
        // preserve board state
        copyBoard(pos, &copyPosition);

        pos->ply++;

        // increment repetition index & store hash key
        pos->repetitionIndex++;
        pos->repetitionTable[pos->repetitionIndex] = pos->hashKey;

        // hash enpassant if available
        if (pos->enpassant != no_sq) { pos->hashKey ^= enpassantKeys[pos->enpassant]; }

        // reset enpassant capture square
        pos->enpassant = no_sq;

        // switch the side, literally giving opponent an extra move to make
        pos->side ^= 1;

        // hash the side
        pos->hashKey ^= sideKey;

        prefetch_hash_entry(pos->hashKey);

        int R = NMP_BASE_REDUCTION + depth / NMP_REDUCTION_DEPTH_DIVISOR;

        R += myMIN((static_eval - beta) / NMP_EVAL_DIVISOR, 3);

        pos->move[pos->ply] = 0;
        pos->piece[pos->ply] = 0;

        /* search moves with reduced depth to find beta cutoffs
           depth - R where R is a reduction limit */
        score = -negamax(-beta, -beta + 1, depth - R, pos, time, !cutNode);

        // decrement ply
        pos->ply--;

        // decrement repetition index
        pos->repetitionIndex--;

        // restore board state
        takeBack(pos, &copyPosition);


        if (time->stopped == 1) return 0;

        // fail-hard beta cutoff
        if (score >= beta) {

            // if there is any unproven mate don't return but we can still return beta
            if (score > mateScore) {
                score = beta;
            }

            if (pos->nmpPly || depth < 15) {
                return score;
            }

            pos->nmpPly = pos->ply + (depth - R) * 2 / 2;
            int verificationScore = -negamax(beta - 1, beta, depth - R, pos, time, false);
            pos->nmpPly = 0;

            if (verificationScore >= beta) {
                return score;
            }
        }


    }

    // razoring
    if (!pos->isSingularMove[pos->ply] &&
        !pvNode && !in_check && depth <= RAZORING_DEPTH && static_eval + RAZORING_MARGIN * depth < alpha) {
        int razoringScore = quiescence(alpha, beta, pos, time);
        if (razoringScore <= alpha) {
            return razoringScore;
        }
    }

    // create move list instance
    moves moveList[1], badQuiets[1];
    badQuiets->count = 0;

    // generate moves
    moveGenerator(moveList, pos);

    // if we are now following PV line
    if (pos->followPv)
        // enable PV move scoring
        enable_pv_scoring(moveList, pos);

    // sort moves
    sort_moves(moveList, tt_move, pos);

    // number of moves searched in a move list
    int moves_searched = 0;

    int bestScore = -infinity;

    //bool skipQuiet = false;

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

        if (currentMove == pos->isSingularMove[pos->ply]) {
            continue;
        }

        bool notTactical = getMoveCapture(currentMove) == 0 && getMovePromoted(currentMove) == 0;


        int moveHistory = notTactical ? quietHistory[pos->side][getMoveSource(currentMove)][getMoveTarget(currentMove)] +
                getContinuationHistoryScore(pos, 1, currentMove) + getContinuationHistoryScore(pos, 4, currentMove): 0;

        int lmrDepth = myMAX(0, depth - getLmrReduction(depth, legal_moves, notTactical));

        bool isNotMated = bestScore > -mateScore;

        if (!rootNode && notTactical && isNotMated) {

                int lmpThreshold = (LMP_BASE + LMP_MULTIPLIER * lmrDepth * lmrDepth);

                // Late Move Pruning
                if (legal_moves>= lmpThreshold) {
                    continue;
                }

                // Futility Pruning
                if (lmrDepth <= FP_DEPTH && !pvNode && !in_check && (static_eval + FUTILITY_PRUNING_OFFSET[clamp(lmrDepth, 1, 5)]) + FP_MARGIN * lmrDepth <= alpha) {
                    continue;
                }
                // Quiet History Pruning
                if (depth <= 4 && !in_check && moveHistory < depth * -2048) {
                    break;
                }

        }

        // SEE PVS Pruning
        int seeThreshold =
                notTactical ? SEE_QUIET_THRESHOLD * lmrDepth : SEE_NOISY_THRESHOLD * lmrDepth * lmrDepth;
        if (lmrDepth <= SEE_DEPTH && legal_moves > 0 && !SEE(pos, currentMove, seeThreshold))
            continue;

        struct copyposition copyPosition;
        // preserve board state
        copyBoard(pos, &copyPosition);

        // increment ply
        pos->ply++;

        // increment repetition index & store hash key
        pos->repetitionIndex++;
        pos->repetitionTable[pos->repetitionIndex] = pos->hashKey;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], allMoves, pos) == 0) {
            // decrement ply
            pos->ply--;

            // decrement repetition index
            pos->repetitionIndex--;

            // skip to next move
            continue;
        }

        int extensions = 0;

        // Singular Extensions
        if (!rootNode && depth >= SE_DEPTH + tt_pv && currentMove == tt_move && !pos->isSingularMove[pos->ply] &&
            tt_depth >= depth - SEE_TT_DEPTH_SUBTRACTOR && tt_flag != hashFlagBeta &&
            abs(tt_score) < mateScore) {
            const int singularBeta = tt_score - depth;
            const int singularDepth = (depth - 1) / 2;


            // decrement ply
            pos->ply--;

            // take move back
            takeBack(pos, &copyPosition);

            pos->isSingularMove[pos->ply] = currentMove;

            const int singularScore =
                    negamax(singularBeta - 1, singularBeta, singularDepth, pos, time, cutNode);

            pos->isSingularMove[pos->ply] = 0;

            makeMove(moveList->moves[count], allMoves, pos);

            pos->ply++;

            // Singular Extension
            if (singularScore < singularBeta) {
                extensions++;

                // Double Extension
                if (!pvNode && singularScore <= singularBeta - DOUBLE_EXTENSION_MARGIN) {
                    extensions++;

                    // Low Depth Extension
                    depth += depth < 10;
                }

                // Triple Extension
                if (!getMoveCapture(currentMove) && singularScore + TRIPLE_EXTENSION_MARGIN < singularBeta) {
                    extensions++;
                }

            }

            // Negative Extensions
            else if (tt_score >= beta) {
                extensions -= 1 + !pvNode;
                // Double Negative Extension
                if (!pvNode && tt_score >= beta + 60) {
                    extensions -= 1;

                    // High Depth Extension
                    depth -= depth > 12;
                }

                // Triple Negative Extension
                if (notTactical && tt_score - 90 >= beta) {
                    extensions -= 1;
                }

            }
        }


        // increment nodes count
        searchNodes++;

        prefetch_hash_entry(pos->hashKey);

        // increment legal moves
        legal_moves++;

        if (notTactical) {
            pos->move[pos->ply] = currentMove;
            pos->piece[pos->ply] = copyPosition.mailboxCopy[getMoveSource(currentMove)];
            addMoveToHistoryList(badQuiets, currentMove);
            quietMoves++;
        } else {
            //captureMoves++;
        }

        int new_depth = depth - 1 + extensions;

        int lmrReduction = getLmrReduction(depth, legal_moves, notTactical);

        /* All Moves */

        // Reduce More

        if (notTactical) {
            // Reduce More
            if (!pvNode && quietMoves >= 4) {
                lmrReduction += 1;
            }

            // if the move have good history decrease reduction other hand the move have bad history then reduce more
            int moveHistoryReduction = moveHistory / 4096;
            lmrReduction -= clamp(moveHistoryReduction, -3, 3);
        }

        // Reduce Less
        if (tt_pv) {
            lmrReduction -= 1;
        }



        int reduced_depth = myMAX(1, myMIN(new_depth - lmrReduction, new_depth));

        if(moves_searched >= LMR_FULL_DEPTH_MOVES &&
           depth >= LMR_REDUCTION_LIMIT) {

            score = -negamax(-alpha - 1, -alpha, reduced_depth, pos, time, true);

            if (score > alpha && lmrReduction != 0) {
                bool doDeeper = score > bestScore + DEEPER_LMR_MARGIN;
                new_depth += doDeeper;
                score = -negamax(-alpha - 1, -alpha, new_depth, pos, time, !cutNode);
            }
        }
        else if (!pvNode || legal_moves > 1) {
            score = -negamax(-alpha - 1, -alpha, new_depth, pos, time, !cutNode);
        }

        if (pvNode && (legal_moves == 1 || score > alpha)) {
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, new_depth, pos, time, false);
        }

        // decrement ply
        pos->ply--;

        // decrement repetition index
        pos->repetitionIndex--;

        // take move back
        takeBack(pos, &copyPosition);

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
                    pos->pvTable[pos->ply][pos->ply] = currentMove;

                    // loop over the next ply
                    for (int next_ply = pos->ply + 1; next_ply < pos->pvLength[pos->ply + 1]; next_ply++)
                        // copy move from deeper ply into a current ply's line
                        pos->pvTable[pos->ply][next_ply] = pos->pvTable[pos->ply + 1][next_ply];

                    // adjust PV length
                    pos->pvLength[pos->ply] = pos->pvLength[pos->ply + 1];
                }

                // fail-hard beta cutoff
                if (score >= beta) {
                    if (notTactical) {
                        // store killer moves
                        pos->killerMoves[pos->ply][0] = bestMove;
                        updateQuietMoveHistory(bestMove, pos->side, depth, badQuiets);
                        updateContinuationHistory(pos, bestMove, depth, badQuiets);

                        if (rootNode) {
                            updateRootHistory(pos, bestMove, depth, badQuiets);
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
            // return mating score (assuming closest distance to mating pos)
            return -mateValue + pos->ply;

            // king is not in check
        else
            // return stalemate score
            return 0;
    }

    if (!pos->isSingularMove[pos->ply]) {

        uint8_t hashFlag = hashFlagExact;
        if (alpha >= beta) {
            hashFlag = hashFlagAlpha;
        } else if (alpha <= originalAlpha) {
            hashFlag = hashFlagBeta;
        }

        if (!in_check && (bestMove == 0 || !getMoveCapture(bestMove)) &&
            !(hashFlag == hashFlagAlpha && bestScore <= static_eval) &&
            !(hashFlag == hashFlagBeta && bestScore >= static_eval)) {

            int corrhistBonus = clamp(bestScore - static_eval, -CORRHIST_LIMIT, CORRHIST_LIMIT);
            updatePawnCorrectionHistory(pos, depth, corrhistBonus);
            updateMinorCorrectionHistory(pos, depth, corrhistBonus);
            update_non_pawn_corrhist(pos, depth, corrhistBonus);
        }

        // store hash entry with the score equal to alpha
        writeHashEntry(bestScore, bestMove, depth, hashFlag, tt_pv, pos);
    }

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
    //memset(position->mailbox, NO_PIECE, sizeof(position->mailbox));
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

        for (int i = 0; i < maxPly; ++i) {
            position->isSingularMove[i] = 0;
            position->staticEval[i] = noEval;
            position->piece[i] = 0;
            position->move[i] = 0;
        }

        int startTime = getTimeMiliSecond();

        if (time->timeset && startTime >= time->softLimit) {
            time->stopped = 1;
        }

        int window = 9;
        int aspirationWindowDepth = current_depth;

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
            score = negamax(alpha, beta, myMAX(aspirationWindowDepth, 1), position, time, false);

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
                aspirationWindowDepth = current_depth;
            }

            else if (score >= beta) {
                beta = myMIN(infinity, beta + window);
                aspirationWindowDepth = myMAX(aspirationWindowDepth - 1, current_depth - 5);

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
