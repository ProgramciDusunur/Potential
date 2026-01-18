//
// Created by erena on 13.09.2024.
//

#include "search.h"


/*███████████████████████████████████████████████████████████████████████████████*\
  ██                                                                           ██
  ██          T U N A B L E    S E A R C H    P A R A M E T E R S              ██
  ██                                                                           ██
\*███████████████████████████████████████████████████████████████████████████████*/


/*╔═══════════════════════════════╗
  ║ Static Exchange Evaluation    ║
  ╚═══════════════════════════════╝*/
  int SEE_PIECE_VALUES[] = {100, 300, 300, 500, 1200, 0, 0};
  int QS_SEE_THRESHOLD = 0;
  int SEE_MOVE_ORDERING_THRESHOLD = -82;
  int SEE_QUIET_THRESHOLD = -67;
  int SEE_NOISY_THRESHOLD = -32;
  int SEE_DEPTH = 10;
  
  
  /*╔═══════════════════════╗
    ║ Null Move Pruning     ║
    ╚═══════════════════════╝*/
  int NMP_DEPTH = 3;  
  int NMP_BASE_REDUCTION = 5120;
  int NMP_DEPTH_MULTIPLIER = 256;
  int NMP_REDUCTION_DEPTH_DIVISOR = 1024;
  int NMP_EVAL_DIVISOR = 400;
  
  
  /*╔═══════════════════════╗
    ║ Late Move Reduction   ║
    ╚═══════════════════════╝*/
  int LMR_TABLE[2][maxPly][maxPly];
  double LMR_TABLE_BASE_NOISY = 0.38;
  double LMR_TABLE_NOISY_DIVISOR = 3.76;
  double LMR_TABLE_BASE_QUIET = 1.01;
  double LMR_TABLE_QUIET_DIVISOR = 2.32;
  int LMR_FULL_DEPTH_MOVES = 2;
  int LMR_REDUCTION_LIMIT = 3;
  int DEEPER_LMR_MARGIN = 35;  
  int QUIET_HISTORY_LMR_DIVISOR = 4096;
  int QUIET_HISTORY_LMR_MINIMUM_SCALER = 3072;
  int QUIET_HISTORY_LMR_MAXIMUM_SCALER = 3072;
  int PAWN_HISTORY_LMR_DIVISOR = 4096;
  int PAWN_HISTORY_LMR_MINIMUM_SCALER = 3072;
  int PAWN_HISTORY_LMR_MAXIMUM_SCALER = 3072;
  int NOISY_HISTORY_LMR_DIVISOR = 10240;  
  int QUIET_NON_PV_LMR_SCALER = 1024;
  int CUT_NODE_LMR_SCALER = 2048;
  int TT_PV_LMR_SCALER = 1024;
  int TT_PV_FAIL_LOW_LMR_SCALER = 1024;
  int TT_CAPTURE_LMR_SCALER = 1024;
  int GOOD_EVAL_LMR_SCALER = 1024;
  int IMPROVING_LMR_SCALER = 1024;
  int LMR_FUTILITY_OFFSET[] = {0, 164, 82, 41, 20, 10};
  
  
  /*╔═══════════════════════╗
    ║ Late Move Pruning     ║
    ╚═══════════════════════╝*/
  int LMP_BASE = 4;
  int LMP_MULTIPLIER = 3;

/*╔═════════════╗
  ║   Probcut   ║
  ╚═════════════╝*/
  int PROBCUT_BETA_MARGIN = 150;
  int PROBCUT_DEPTH = 5;
  int PROBCUT_DEPTH_SUBTRACTOR = 4;
  int PROBCUT_IMPROVING_MARGIN = 30;
  int PROBCUT_SEE_NOISY_THRESHOLD = 100;
  int PROBCUT_NOISY_HISTORY_DIVISOR = 10240;


/*╔═══════════════════╗
  ║   Small Probcut   ║
  ╚═══════════════════╝*/
  int SPROBCUT_BETA_MARGIN = 350;
  int SPROBCUT_TT_DEPTH_SUBTRACTOR = 4;
  
  
  /*╔════════════════════╗
    ║ Futility Pruning   ║
    ╚════════════════════╝*/
  int FUTILITY_PRUNING_OFFSET[] = {0, 82, 41, 20, 10, 5};
  int FP_DEPTH = 5;
  int FP_MARGIN = 82;
  
  
  /*╔══════════════════════════════╗
    ║ Reverse Futility Pruning     ║
    ╚══════════════════════════════╝*/
  int RFP_MARGIN = 52;
  int RFP_IMPROVING_MARGIN = 45;
  int RFP_DEPTH = 11;
  
  
  /*╔══════════╗
    ║ Razoring ║
    ╚══════════╝*/
  int RAZORING_DEPTH = 3;
  int RAZORING_FULL_MARGIN = 200;    
  int RAZORING_DEPTH_SCALE = 15;
  int RAZORING_VERIFY_MARGIN = 120;
  int RAZORING_TRIM = 1;
  int RAZORING_FULL_D = 2;
  int RAZORING_VERIFY_D = 3;
  int RAZORING_MARGIN[] = {0, 100, 200, 300, 400};
  
  
  /*╔═════════════════════╗
    ║ Singular Extensions ║
    ╚═════════════════════╝*/
  int SE_DEPTH = 5;
  int SE_TT_DEPTH_SUBTRACTOR = 3;
  // Positive Extensions
  int DOUBLE_EXTENSION_MARGIN = 20;
  int TRIPLE_EXTENSION_MARGIN = 40;
  int QUADRUPLE_EXTENSION_MARGIN = 85;
  // Negative Extensions
  int DOUBLE_NEGATIVE_EXTENSION_MARGIN = 60;
  int TRIPLE_NEGATIVE_EXTENSION_MARGIN = 90;
  
  /*╔═══════════════════════════════╗
    ║ Internal Iterative Reductions ║
    ╚═══════════════════════════════╝*/
  int IIR_DEPTH = 8;
  int IIR_TT_DEPTH_SUBTRACTOR = 3;

  /*╔══════════════════════════════╗
    ║      Aspiration Windows      ║
    ╚══════════════════════════════╝*/
  int ASP_WINDOW_BASE = 9;
  int ASP_WINDOW_MIN_DEPTH = 4;
  double ASP_WINDOW_MULTIPLIER = 1.8;

  uint64_t nodes_spent_table[4096] = {0};  



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

void pick_next_move(int moveNum, moves *moveList, int *move_scores) {
    int bestIndex = moveNum;
    int bestScore = move_scores[moveNum];

    for (int i = moveNum + 1; i < moveList->count; i++) {
        if (move_scores[i] > bestScore) {
            bestScore = move_scores[i];
            bestIndex = i;
        }
    }

    if (bestIndex != moveNum) {
        int tempScore = move_scores[moveNum];
        move_scores[moveNum] = move_scores[bestIndex];
        move_scores[bestIndex] = tempScore;

        uint16_t tempMove = moveList->moves[moveNum];
        moveList->moves[moveNum] = moveList->moves[bestIndex];
        moveList->moves[bestIndex] = tempMove;
    }
}

void init_move_scores(moves *moveList, int *move_scores, uint16_t tt_move, board* position) {
    for (int count = 0; count < moveList->count; count++) {
        uint16_t current_move = moveList->moves[count];
        
        if (tt_move == current_move) {
            move_scores[count] = 2000000000;
        } else {
            move_scores[count] = scoreMove(current_move, position);
        }
    }
}

void init_quiescence_scores(moves *moveList, int *move_scores, board* position) {
    for (int count = 0; count < moveList->count; count++) {
        uint16_t move = moveList->moves[count];
        
        if (getMoveCapture(move)) {
            int target_piece = P;
            uint8_t bb_piece = position->mailbox[getMoveTarget(move)];
            
            if (bb_piece != NO_PIECE && getBit(position->bitboards[bb_piece], getMoveTarget(move))) {
                target_piece = bb_piece;
            }
            
            move_scores[count] = mvvLva[position->mailbox[getMoveSource(move)]][target_piece] + 1000000000;
        } else {
            move_scores[count] = 0;
        }
    }
}

/*  =======================
         Move ordering
    =======================

    1. TT Move
    2. PV Moves
    3. Promotion Moves
    4. Captures in MVV/LVA / SEE / Capture History / Recapture Bonus    
    5. Quiet History / Continuation History / Pawn History
*/

// score moves
int scoreMove(uint16_t move, board* position) {
    // make sure we are dealing with PV move
    if (position->scorePv && position->pvTable[0][position->ply] == move) {
        // disable score PV flag
        position->scorePv = 0;

        // give PV move the highest score to search it first
        return 1500000000;
    }

    // score promotion move
    if (getMovePromote(move)) {
        switch (getMovePromotedPiece(position->side, move)) {
            case q:
            case Q:
                return 1000000000;
                break;
            case n:
            case N:
                return 800000000;
                break;
            case r:
            case R:
                return -500000000;
                break;
            case b:
            case B:
                return -800000000;
                break;
        }
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

        int previous_move_target_square = getMoveTarget(position->move[myMAX(0, position->ply - 1)]);
        int recapture_bonus = getMoveTarget(move) == previous_move_target_square ? 200000 : 0;

        int piece = position->mailbox[getMoveSource(move)];

        // score move by MVV LVA lookup [source piece][target piece]
        captureScore += mvvLva[piece][target_piece];

        captureScore += captureHistory[piece][getMoveTarget(move)][position->mailbox[getMoveTarget(move)]];

        captureScore += SEE(position, move, SEE_MOVE_ORDERING_THRESHOLD) ? 1000000000 : -1000000;

        captureScore += recapture_bonus;
        
        // NMP refutation move
        //captureScore += getMoveTarget(move) == getMoveSource(position->nmp_refutation_move[position->ply]) ? 500000 : 0;

        return captureScore;

    }
    // score quiet moves
    else {
        int quiet_score = 0;
        quiet_score +=
            // quiet main history 
            quietHistory[position->side][getMoveSource(move)][getMoveTarget(move)]
            [is_square_threatened(position, getMoveSource(move))][is_square_threatened(position, getMoveTarget(move))];

        // 1 ply continuation history
        quiet_score += getContinuationHistoryScore(position, 1, move);
        // 2 ply continuation history
        quiet_score += getContinuationHistoryScore(position, 2, move);
        // 4 ply continuation history
        quiet_score += getContinuationHistoryScore(position, 4, move);
        // pawn history
        quiet_score += pawnHistory[position->pawnKey % 2048][position->mailbox[getMoveSource(move)]][getMoveTarget(move)];
        // NMP refutation move
        //quiet_score += getMoveSource(move) == getMoveTarget(position->nmp_refutation_move[position->ply]) ? 500000 : 0;

        return quiet_score;        
    }
    return 0;
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
void printMove(uint16_t move) {
    if (getMovePromote(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromotedPiece(black, move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }
}

int getLmrReduction(int depth, int moveNumber, bool isQuiet) {
    return LMR_TABLE[isQuiet][myMIN(63, myMAX(depth, 0))][myMIN(63, moveNumber)];
}

uint8_t justPawns(board *pos) {
    return !((pos->bitboards[N] | pos->bitboards[n] | pos->bitboards[B] |
              pos->bitboards[b] | pos->bitboards[R] | pos->bitboards[r] |
              pos->bitboards[Q] | pos->bitboards[q]) &
             pos->occupancies[pos->side]);
}


int move_estimated_value(board *pos, uint16_t move) {

    // Start with the value of the piece on the target square
    int target_piece = pos->mailbox[getMoveTarget(move)] > 5
                       ? pos->mailbox[getMoveTarget(move)] - 6
                       : pos->mailbox[getMoveTarget(move)];

    int value = SEE_PIECE_VALUES[target_piece];

    // Factor in the new piece's value and remove our promoted pawn
    if (getMovePromote(move))
        value += SEE_PIECE_VALUES[getMovePromotedPiece(white, move)] - SEE_PIECE_VALUES[PAWN];

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

int SEE(board *pos, uint16_t move, int threshold) {

    int from, to, enpassant, promotion, colour, balance, nextVictim;
    uint64_t bishops, rooks, occupied, attackers, myAttackers;

    // Unpack move information
    from = getMoveSource(move);
    to = getMoveTarget(move);
    enpassant = getMoveEnpassant(move);
    promotion = getMovePromote(move);

    // Next victim is moved piece or promotion type
    nextVictim = promotion ? getMovePromotedPiece(white, move) : pos->mailbox[from];
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

void scaleTime(my_time* time, uint8_t bestMoveStability, uint8_t evalStability, uint16_t move, double complexity, board* pos) {
    double bestMoveScale[5] = {2.43, 1.35, 1.09, 0.88, 0.68};
    double evalScale[5] = {1.25, 1.15, 1.00, 0.94, 0.88};
    double complexityScale = my_max_double(0.77 + clamp_double(complexity, 0.0, 200.0) / 400.0, 1.0);
    double not_bm_nodes_fraction = 
       (double)nodes_spent_table[move & 4095] / (double)pos->nodes_searched;
    double node_scaling_factor = (1.5f - not_bm_nodes_fraction) * 1.35f;
    time->softLimit =
            myMIN(time->starttime + time->baseSoft * bestMoveScale[bestMoveStability] * 
                evalScale[evalStability] * node_scaling_factor * complexityScale, time->maxTime + time->starttime);    
}

bool has_enemy_any_threat(board *pos) {
    return (pos->occupancies[pos->side] & pos->pieceThreats.stmThreats[pos->side ^ 1]) != 0;
}

int get_draw_score(board *pos) {
    return (pos->nodes_searched & 3) - 2; // Randomize between -2 and +2
}

// quiescence search
int quiescence(int alpha, int beta, board* position, my_time* time) {
    if (time->isNodeLimit) {
        check_node_limit(time, position);
    }

    if ((position->nodes_searched & 2047) == 0) {
        communicate(time, position);
    }

    if (position->ply > maxPly - 1) {
        // evaluate position
        return evaluate(position);
    }

    int score = 0, bestScore = 0;

    int pvNode = beta - alpha > 1;

    //int rootNode = position->ply == 0;

    if (position->ply > position->seldepth) {
        position->seldepth = position->ply;
    }


    uint16_t bestMove = 0;
    uint16_t tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;
    bool tt_pv = pvNode;

    // read hash entry
    if (position->ply &&
        (tt_hit =
                 readHashEntry(position, &tt_move, &tt_score, &tt_depth, &tt_flag, &tt_pv)) && !pvNode) {
        if ((tt_flag == hashFlagExact) ||
            ((tt_flag == hashFlagBeta) && (tt_score <= alpha)) ||
            ((tt_flag == hashFlagAlpha) && (tt_score >= beta))) {
             return tt_score >= beta ? (tt_score * 3 + beta) / 4 :
                                          tt_score;
        }
    }

    // evaluate position
    int evaluation = evaluate(position);

    evaluation = adjust_eval_with_corrhist(position, evaluation);

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

    int futilityValue = bestScore + 100;

    // legal moves counter
    //int legal_moves = 0;

    int move_scores[256];
    init_quiescence_scores(moveList, move_scores, position);

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        pick_next_move(count, moveList, move_scores);
        uint16_t move = moveList->moves[count];

        if (bestScore > -mateFound) {
            if (!SEE(position, move, QS_SEE_THRESHOLD)) {
                continue;
            }

            if (getMoveCapture(move) && futilityValue <= alpha && !SEE(position, move, 1)) {
                bestScore = myMAX(bestScore, futilityValue);
                continue;
            }
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
        position->nodes_searched++;

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
    writeHashEntry(position->hashKey, bestScore, bestMove, 0, hashFlag, tt_pv, position);

    // node (move) fails low
    return bestScore;
}


// negamax alpha beta search
int negamax(int alpha, int beta, int depth, board* pos, my_time* time, bool cutNode) {
    if (time->isNodeLimit) {
        check_node_limit(time, pos);
    }
    
    if ((pos->nodes_searched & 2047) == 0) {
        communicate(time, pos);
    }    

    if (pos->ply > maxPly - 1) {
        // evaluate position
        return evaluate(pos);
    }
    // init PV length
    pos->pvLength[pos->ply] = pos->ply;

    if (pos->ply > pos->seldepth) {
        pos->seldepth = pos->ply;
    }


    // variable to store current move's score (from the static evaluation perspective)
    int score = 0;

    depth = myMIN(depth, maxPly - 1);


    int pvNode = beta - alpha > 1;

    int rootNode = pos->ply == 0;

    uint16_t bestMove = 0;
    uint64_t pos_key = 0;
    uint16_t tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;
    bool tt_pv = pvNode;    

    // Check for fifty-move rule
    if (pos->fifty >= 100) {
        int in_check = isSquareAttacked((pos->side == white) ? getLS1BIndex(pos->bitboards[K]) :
                                    getLS1BIndex(pos->bitboards[k]),
                                    pos->side ^ 1, pos);
        if (!in_check) {
            // return draw by fifty-move rule
            return get_draw_score(pos);
        }       
    }

    if (!rootNode) {

        if (isRepetition(pos) || isMaterialDraw(pos)) {
            return get_draw_score(pos);
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
        pos_key = pos->hashKey;
        if (tt_depth >= depth) {
            if ((tt_flag == hashFlagExact) ||
                ((tt_flag == hashFlagBeta) && (tt_score <= alpha)) ||
                ((tt_flag == hashFlagAlpha) && (tt_score >= beta))) {
                return tt_score >= beta ? (tt_score * 3 + beta) / 4 :
                                          tt_score;
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

    int static_eval = adjust_eval_with_corrhist(pos, raw_eval);

    bool improving = false;

    bool corrplexity = abs(raw_eval - static_eval) > 82;
    int corrplexity_value = abs(raw_eval - static_eval);

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

    improving |= pos->staticEval[pos->ply] >= beta + 100;

    uint16_t rfpMargin = improving ? RFP_IMPROVING_MARGIN * (depth - 1) : RFP_MARGIN * depth;

    rfpMargin += 6 * depth * depth;

    bool rfp_tt_pv_decision = !tt_pv || (tt_pv && tt_hit && tt_score >= beta + 90 - 15 * ((tt_depth + depth) / 2));    

    // Reverse Futility Pruning
    if (!pos->isSingularMove[pos->ply] && rfp_tt_pv_decision &&
        depth <= RFP_DEPTH && !pvNode && !in_check && (!tt_hit || ttAdjustedEval != static_eval) &&
        ttAdjustedEval - rfpMargin >= beta + corrplexity * 20)
        return ttAdjustedEval;

    // Null Move Pruning
    if (!pos->isSingularMove[pos->ply] && !pvNode &&
        depth >= NMP_DEPTH && !in_check && !rootNode &&
            ttAdjustedEval >= beta + 30 &&
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

        int R = (NMP_BASE_REDUCTION + depth * NMP_DEPTH_MULTIPLIER) / NMP_REDUCTION_DEPTH_DIVISOR;

        R += myMIN((ttAdjustedEval - beta) / NMP_EVAL_DIVISOR, 3);        

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
            if (score > mateValue) {
                score = beta;
            }

            if (pos->nmpPly || depth < 15) {
                pos->move[myMIN(pos->ply, maxPly - 1)] = 0;
                pos->piece[myMIN(pos->ply, maxPly - 1)] = 0;
                return score;
            }

             // Skip verification if null move score is much above beta (scaled by depth)
            if (score >= beta + depth) {
                pos->move[myMIN(pos->ply, maxPly - 1)] = 0;
                pos->piece[myMIN(pos->ply, maxPly - 1)] = 0;
                return score;
            }
                
            pos->nmpPly = pos->ply + (depth - R) * 2 / 2;
            int verificationScore = -negamax(beta - 1, beta, depth - R, pos, time, false);
            pos->nmpPly = 0;

            if (verificationScore >= beta) {
                pos->move[myMIN(pos->ply, maxPly - 1)] = 0;
                pos->piece[myMIN(pos->ply, maxPly - 1)] = 0;
                return score;
            }
        }

        // Refutation, our opponent has an argument
        if (score < beta) {
            // store null move refutation move
            pos->nmp_refutation_move[pos->ply] = pos->move[myMIN(pos->ply, maxPly - 1)];

            uint16_t nmp_ref_move = pos->nmp_refutation_move[pos->ply];
            int nmp_depth = depth - R;

            if (!isTactical(nmp_ref_move)) {
                int refutation_bonus = 100 + 50 * nmp_depth;
                adjust_single_quiet_hist_entry(pos, pos->side, nmp_ref_move, refutation_bonus);
            }
        }
    }    

    // razoring
    if (!pos->isSingularMove[pos->ply] &&
        !pvNode && !in_check && depth <= RAZORING_DEPTH) {
        int max_razor_index = 4;
        int razor_depth = myMIN(myMIN(depth, RAZORING_DEPTH), max_razor_index);

        if (razor_depth > 0) {
            const int margin = RAZORING_MARGIN[razor_depth];

            if (ttAdjustedEval + margin <= alpha) {
                const bool allow_full_razor = depth == 1 ||
                (depth <= RAZORING_FULL_D && ttAdjustedEval + margin + RAZORING_FULL_MARGIN <= alpha);

                if (allow_full_razor) {
                    return quiescence(alpha, beta, pos, time);
                }

                const int capped_alpha = myMAX(alpha - margin, -mateValue);
                const int razor_alpha = capped_alpha;
                const int razor_beta = razor_alpha + 1;
                int razor_score = quiescence(razor_alpha, razor_beta, pos, time);

                // We proved a fail low.
                if (razor_score <= razor_alpha) {                       
                    return razor_score;
                }

                if (razor_score >= razor_beta + RAZORING_VERIFY_MARGIN && depth <= RAZORING_VERIFY_D) {                    
                    depth -= myMIN(RAZORING_TRIM, depth - 1);
                }
            }
        }
    }

    // legal moves counter
    int legal_moves = 0;

    int probcut_beta = beta + PROBCUT_BETA_MARGIN - PROBCUT_IMPROVING_MARGIN * improving;
    if (!pvNode && !in_check && depth >= PROBCUT_DEPTH && abs(beta) < mateValue  && !pos->isSingularMove[pos->ply] &&
        (!tt_hit || tt_depth + 3 < depth || tt_score >= probcut_beta)) {
            moves capture_promos[1];
            capture_promos->count = 0;
            int probcut_depth = depth - PROBCUT_DEPTH_SUBTRACTOR;

            noisyGenerator(capture_promos, pos);

            int move_scores[256];
            init_move_scores(capture_promos, move_scores, tt_move, pos);
            for (int count = 0; count < capture_promos->count; count++) {
                pick_next_move(count, capture_promos, move_scores);
                uint16_t move = capture_promos->moves[count];
                int move_history =
                captureHistory[pos->mailbox[getMoveSource(move)]][getMoveTarget(move)][pos->mailbox[getMoveTarget(move)]];

                if (!SEE(pos, move, PROBCUT_SEE_NOISY_THRESHOLD)) {
                    continue;
                }

                // Noisy Futility Pruning
                int noisyFPMargin = static_eval + 164 + 100 * depth;
                if (!pvNode && !in_check && noisyFPMargin <= alpha) {
                    continue;
                }

                struct copyposition copyPosition;
                // preserve board state
                copyBoard(pos, &copyPosition);
                // increment ply
                pos->ply++;

                // increment repetition index & store hash key
                pos->repetitionIndex++;
                pos->repetitionTable[pos->repetitionIndex] = pos->hashKey;

                // make sure to make only legal moves
                if (makeMove(capture_promos->moves[count], allMoves, pos) == 0) {
                    // decrement ply
                    pos->ply--;

                    // decrement repetition index
                    pos->repetitionIndex--;

                    // skip to next move
                    continue;
                }

                prefetch_hash_entry(pos->hashKey);
                pos->nodes_searched++;
                legal_moves++;


                int probcut_value = -quiescence(-probcut_beta, -probcut_beta + 1, pos, time);

                if (probcut_value >= probcut_beta) {
                    int adjusted_probcut_depth = probcut_depth * 1024;

                    // Capture History based reduction
                    adjusted_probcut_depth += move_history / PROBCUT_NOISY_HISTORY_DIVISOR * 256;

                    adjusted_probcut_depth /= 1024;

                    probcut_value = -negamax(-probcut_beta, -probcut_beta + 1, adjusted_probcut_depth, pos, time, !cutNode);
                }

                // decrement ply
                pos->ply--;

                // decrement repetition index
                pos->repetitionIndex--;

                // take move back
                takeBack(pos, &copyPosition);

                if (probcut_value >= probcut_beta) {
                    writeHashEntry(pos->hashKey, probcut_value, move, probcut_depth, hashFlagAlpha, tt_pv, pos);
                    return probcut_value;
                }
            }
    }

    int small_probcut_beta = beta + SPROBCUT_BETA_MARGIN;
    
    // Small Probcut
    if (!pos->isSingularMove[pos->ply] && !pvNode && tt_flag == hashFlagAlpha && tt_depth >= depth - SPROBCUT_TT_DEPTH_SUBTRACTOR &&
        tt_score >= small_probcut_beta && abs(tt_score) < mateValue && abs(beta) < mateValue) {
            return small_probcut_beta;            
    }

    bool enemy_has_no_threats = !has_enemy_any_threat(pos);


    // create move list instance
    moves moveList[1], badQuiets[1], noisyMoves[1];
    badQuiets->count = 0;
    noisyMoves->count = 0;

    // generate moves
    moveGenerator(moveList, pos);

    // if we are now following PV line
    if (pos->followPv)
        // enable PV move scoring
        enable_pv_scoring(moveList, pos);

    int move_scores[256];
    init_move_scores(moveList, move_scores, tt_move, pos);

    // number of moves searched in a move list
    int moves_searched = 0;

    int bestScore = -infinity;    

    // legal moves counter
    legal_moves = 0;

    // quiet move counter
    int quietMoves = 0;

    // capture move counter
    //int captureMoves = 0;

    const int originalAlpha = alpha;

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        pick_next_move(count, moveList, move_scores);
        uint16_t currentMove = moveList->moves[count];

        if (currentMove == pos->isSingularMove[pos->ply]) {
            continue;
        }

        bool notTactical = getMoveCapture(currentMove) == 0 && getMovePromote(currentMove) == 0;

        int pawnHistoryValue = notTactical ? pawnHistory[pos->pawnKey % 2048][pos->mailbox[getMoveSource(currentMove)]][getMoveTarget(currentMove)] : 0;

        int moveHistory = notTactical ? quietHistory[pos->side][getMoveSource(currentMove)][getMoveTarget(currentMove)]
                                        [is_square_threatened(pos, getMoveSource(currentMove))][is_square_threatened(pos, getMoveTarget(currentMove))] +
                getContinuationHistoryScore(pos, 1, currentMove) + getContinuationHistoryScore(pos, 4, currentMove): 
                captureHistory[pos->mailbox[getMoveSource(currentMove)]][getMoveTarget(currentMove)][pos->mailbox[getMoveTarget(currentMove)]];

        int lmrDepth = myMAX(0, depth - getLmrReduction(depth, legal_moves, notTactical) + (moveHistory / 8192 * notTactical));


        bool isNotMated = bestScore > -mateFound;

        if (!rootNode && notTactical && isNotMated) {

            int lmpThreshold = (LMP_BASE + LMP_MULTIPLIER * lmrDepth * lmrDepth) / (2 - improving);

            // Late Move Pruning
            if (legal_moves>= lmpThreshold) {
                continue;
            }

            // Futility Pruning
            if (lmrDepth <= FP_DEPTH && !in_check && (static_eval + FUTILITY_PRUNING_OFFSET[clamp(lmrDepth, 1, 5)]) + FP_MARGIN * lmrDepth + moveHistory / 32 <= alpha) {
                continue;
            }
            // Quiet History Pruning
            if (lmrDepth <= 4 && !in_check && moveHistory < lmrDepth * lmrDepth * -2048) {
                break;
            }

        }

        // SEE PVS Pruning
        int seeThreshold =
                notTactical ? SEE_QUIET_THRESHOLD * lmrDepth : SEE_NOISY_THRESHOLD * lmrDepth * lmrDepth;
        if (lmrDepth <= SEE_DEPTH && legal_moves > 0 && !SEE(pos, currentMove, seeThreshold))
            continue;

        int previous_move_target_square = getMoveTarget(pos->move[myMAX(0, pos->ply - 1)]);
        int extensions = 0;

        // Singular Extensions
        if (pos->ply < depth * 2 && !rootNode && depth >= SE_DEPTH + tt_pv && currentMove == tt_move && !pos->isSingularMove[pos->ply] &&
            tt_depth >= depth - SE_TT_DEPTH_SUBTRACTOR && tt_flag != hashFlagBeta &&
            abs(tt_score) < mateValue) {
            const int singularBeta = tt_score - (depth * 5 + (tt_pv && !pvNode) * 10) / 8;
            const int singularDepth = (depth - 1) / 2;


            struct copyposition copyPosition;
            // preserve board state
            copyBoard(pos, &copyPosition);

            // make sure to make only legal moves
            if (makeMove(moveList->moves[count], allMoves, pos) == 0) {
                continue;
            }

            pos->isSingularMove[pos->ply] = currentMove;

            // take move back
            takeBack(pos, &copyPosition);


            const int singularScore =
                    negamax(singularBeta - 1, singularBeta, singularDepth, pos, time, cutNode);

            pos->isSingularMove[pos->ply] = 0;

            // Singular Extension
            if (singularScore < singularBeta) {
                extensions++;

                // Double Extension                
                int doubleMargin = DOUBLE_EXTENSION_MARGIN + 40 * !notTactical - (moveHistory / 512) - (pawnHistoryValue / 384) - (corrplexity_value / 16);

                if (!pvNode && singularScore <= singularBeta - doubleMargin) {
                    extensions++;

                    // Low Depth Extension
                    depth += depth < 10;
                }

                // Triple Extension
                int tripleMargin = TRIPLE_EXTENSION_MARGIN + 80 * !notTactical - (moveHistory / 512 * notTactical);

                if (singularScore <= singularBeta - tripleMargin) {
                    extensions++;
                }
                
                // ╔═══════════════════════════╗
                // ║            /\             ║
                // ║           /  \            ║
                // ║         <SCALER>          ║
                // ║           \  /            ║
                // ║            \/             ║
                // ╟    «-·´¯`·.¸¸.»·´¯`·-»    ╢
                // ║    Scaling STC / LTC      ║
                // ║   STC:  0.93  +-  1.94    ║
                // ║   LTC: 14.05  +-  7.19    ║
                // ╚═══════════════════════════╝

                // ~~~~ Quadruple Extension ~~~~ //
                int quadrupleMargin = QUADRUPLE_EXTENSION_MARGIN + 170 * !notTactical;

                if (singularScore <= singularBeta - quadrupleMargin) {
                    extensions++;
                }
            }            

            // Negative Extensions
            else if (tt_score >= beta) {
                extensions -= 2 + !pvNode;
            }

            // ╔══════════════════════════════╗
            // ║              /\              ║
            // ║             /  \             ║
            // ║           <SCALER>           ║
            // ║             \  /             ║
            // ║              \/              ║
            // ╟      «-·´¯`·.¸¸.»·´¯`·-»     ╢        
            // ║                              ║
            // ║     STC:   -1.42  +-  3.69   ║
            // ║     LTC:   -6.85  +-  5.03   ║
            // ║    VLTC:    0.71  +-  2.94   ║
            // ║   VVLTC:    5.13  +-  4.04   ║            
            // ╚══════════════════════════════╝

            // ~~~~ Recapture Extension ~~~~ //
            else if (pvNode && !notTactical && getMoveTarget(tt_move) == previous_move_target_square) {
                extensions += 1;
            }
            
            // Cut Node Extension
            else if (cutNode) {
                extensions -= 2;
            }
        }

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


        // increment nodes count
        pos->nodes_searched++;

        prefetch_hash_entry(pos->hashKey);

        // increment legal moves
        legal_moves++;

        if (notTactical) {
            pos->move[myMIN(pos->ply, maxPly - 1)] = currentMove;
            pos->piece[myMIN(pos->ply, maxPly - 1)] = copyPosition.mailboxCopy[getMoveSource(currentMove)];
            addMoveToHistoryList(badQuiets, currentMove);
            quietMoves++;
        } else {
            pos->move[myMIN(pos->ply, maxPly - 1)] = currentMove;
            pos->piece[myMIN(pos->ply, maxPly - 1)] = copyPosition.mailboxCopy[getMoveSource(currentMove)];
            //captureMoves++;
            addMoveToHistoryList(noisyMoves, currentMove);
        }

        uint64_t nodes_before_search = pos->nodes_searched;

        int new_depth = depth - 1 + extensions;

        int lmrReduction = getLmrReduction(depth, legal_moves, notTactical) * 1024;

        /* All Moves */

        // Reduce More
        if (cutNode) {
            lmrReduction += CUT_NODE_LMR_SCALER + !tt_move * 1024;
        }

        if (tt_pv && tt_hit && tt_score <= alpha) {
            lmrReduction += TT_PV_FAIL_LOW_LMR_SCALER;
        }

        if (tt_hit && getMoveCapture(tt_move)) {
            lmrReduction += TT_CAPTURE_LMR_SCALER;
        }

        if (enemy_has_no_threats && !in_check && static_eval - 365 > beta) {
            lmrReduction += GOOD_EVAL_LMR_SCALER;
        }

        // ╔══════════════════════════════╗
        // ║              /\              ║
        // ║             /  \             ║
        // ║           <SCALER>           ║
        // ║             \  /             ║
        // ║              \/              ║
        // ╟      «-·´¯`·.¸¸.»·´¯`·-»     ╢        
        // ║                              ║
        // ║     STC:  -3.12  +-  3.95    ║
        // ║     LTC:   0.71  +-  1.28    ║
        // ║    VLTC:   6.87  +-  4.51    ║
        // ║                              ║
        // ╚══════════════════════════════╝

        if (!improving && !in_check) {
            lmrReduction += IMPROVING_LMR_SCALER;
        }

        if (notTactical) {
            // Reduce More
            if (!pvNode && quietMoves >= 4) {
                lmrReduction += QUIET_NON_PV_LMR_SCALER;
            }

            // Futility LMR
            lmrReduction += (static_eval + 164 + 82 * depth <= alpha && !in_check) * 1024;


            // if the move have good history decrease reduction other hand the move have bad history then reduce more
            int moveHistoryReduction = moveHistory / QUIET_HISTORY_LMR_DIVISOR;
            lmrReduction -= clamp(moveHistoryReduction * 1024, -QUIET_HISTORY_LMR_MINIMUM_SCALER, QUIET_HISTORY_LMR_MAXIMUM_SCALER);

            // pawn history based reduction, same logic as the quiet history
            int pawnHistoryReduction = pawnHistoryValue / PAWN_HISTORY_LMR_DIVISOR;            
            lmrReduction -= clamp(pawnHistoryReduction * 1024, -PAWN_HISTORY_LMR_MINIMUM_SCALER, PAWN_HISTORY_LMR_MAXIMUM_SCALER);
        }
        // Noisy Moves
        else { 
            // capture history based reduction, same logic as the quiet history
            lmrReduction -= moveHistory / NOISY_HISTORY_LMR_DIVISOR;
        }

        // Reduce Less
        if (tt_pv) {
            lmrReduction -= TT_PV_LMR_SCALER + (512 * pvNode) + (256 * improving);
        }
        

        lmrReduction /= 1024;

        int reduced_depth = myMAX(1, myMIN(new_depth - lmrReduction, new_depth)) + pvNode;

        if(moves_searched >= LMR_FULL_DEPTH_MOVES &&
           depth >= LMR_REDUCTION_LIMIT) {

            score = -negamax(-alpha - 1, -alpha, reduced_depth, pos, time, true);

            if (score > alpha && lmrReduction != 0) {
                bool doDeeper = score > bestScore + DEEPER_LMR_MARGIN;
                bool historyReduction = notTactical ? moveHistory / 16384 : 0;
                bool doShallower = score < bestScore + new_depth;
                new_depth -= doShallower;
                new_depth += doDeeper;
                new_depth -= historyReduction;
                score = -negamax(-alpha - 1, -alpha, new_depth, pos, time, !cutNode);
            }
        }
        else if (!pvNode || legal_moves > 1) {
            // if we have chance about to dive into quiescence search then extend
            if (currentMove == tt_move && pos->rootDepth > 8 && tt_depth > 1) {
                new_depth = myMAX(new_depth, 1);
            }
            
            score = -negamax(-alpha - 1, -alpha, new_depth, pos, time, !cutNode);
        }

        if (pvNode && (legal_moves == 1 || score > alpha)) {
            if (!rootNode && currentMove == tt_move && tt_score < alpha && tt_flag == hashFlagBeta) {
                new_depth -= 1;
            }

            // if we have chance about to dive into quiescence search then extend
            if (currentMove == tt_move && pos->rootDepth > 8 && tt_depth > 1) {
                new_depth = myMAX(new_depth, 1);
            }
            
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, new_depth, pos, time, false);
        }

        // decrement ply
        pos->ply--;

        // decrement repetition index
        pos->repetitionIndex--;

        // take move back
        takeBack(pos, &copyPosition);

        if (rootNode) {
            nodes_spent_table[currentMove & 4095] += pos->nodes_searched - nodes_before_search;
        }

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
                        int quiet_history_score = 
                        quietHistory[pos->side][getMoveSource(currentMove)][getMoveTarget(currentMove)]
                        [is_square_threatened(pos, getMoveSource(currentMove))][is_square_threatened(pos, getMoveTarget(currentMove))];

                        updateQuietMoveHistory(bestMove, pos->side, depth, badQuiets, pos);
                        updateContinuationHistory(pos, bestMove, depth, badQuiets, quiet_history_score);
                        updatePawnHistory(pos, bestMove, depth, badQuiets);                       
                        
                    } else { // noisy moves
                        updateCaptureHistory(pos, bestMove, depth);
                    }

                    // always penalize bad noisy moves
                    updateCaptureHistoryMalus(pos, depth, noisyMoves, bestMove);

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
            return get_draw_score(pos);
    }

    // Prior counter-move: if this node failed low, the parent fails high.
    // We give a small history bonus to the prior.
    uint16_t counter_move = pos->move[myMIN(pos->ply, maxPly - 1)];
    bool counter_move_available = counter_move ? !getMoveCapture(counter_move) && !getMovePromote(counter_move) : false;

    if (score <= originalAlpha && pos->move[myMIN(pos->ply, maxPly - 1)] != 0 && counter_move_available) {
        int pcm_bonus = 0;

        pcm_bonus += myMIN(originalAlpha - score * depth, 2048);

        adjust_single_quiet_hist_entry(pos, pos->side, counter_move, pcm_bonus);
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
            update_pawn_correction_hist(pos, depth, corrhistBonus);
            update_minor_correction_hist(pos, depth, corrhistBonus);
            update_major_correction_hist(pos, depth, corrhistBonus);
            update_non_pawn_corrhist(pos, depth, corrhistBonus);
            update_continuation_corrhist(pos, depth, corrhistBonus);
            update_king_rook_pawn_corrhist(pos, depth, corrhistBonus);
        }

        // store hash entry with the score equal to alpha
        writeHashEntry(pos_key, bestScore, bestMove, depth, hashFlag, tt_pv, pos);
    }
    // node (move) fails low
    return bestScore;
}

// search position for the best move
void searchPosition(int depth, board* position, bool benchmark, my_time* time) {
    // define best score variable
    int score = 0;

    // reset "time is up" flag
    time->stopped = 0;

    // reset nodes counter
    position->nodes_searched = 0;

    // reset follow PV flags
    position->followPv = 0;
    position->scorePv = 0;
    
    memset(nodes_spent_table, 0, sizeof(nodes_spent_table));
    memset(position->pvTable, 0, sizeof(position->pvTable));
    memset(position->pvLength, 0, sizeof(position->pvLength));
    memset(position->staticEval, 0, sizeof(position->staticEval));    

    // define initial alpha beta bounds
    int alpha = -infinity;
    int beta = infinity;

    int totalTime = 0;
    // set root depth
    position->rootDepth = 0;

    int previousBestMove = 0;
    uint8_t bestMoveStability = 0;
    int averageScore = noEval;
    uint8_t evalStability = 0;
    int baseSearchScore = -infinity;

    quiet_history_aging();

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        //printf("Node limit: %llu\n", time->isNodeLimit ? time->node_limit : 0);
        if (time->stopped == 1) {
            break;
        }

        for (int i = 0; i < maxPly; ++i) {
            position->isSingularMove[i] = 0;
            position->staticEval[i] = noEval;
            position->piece[i] = 0;
            position->move[i] = 0;
        }

        position->seldepth = 0;
        position->rootDepth = current_depth;


        int startTime = getTimeMiliSecond();

        if ((time->timeset && startTime >= time->softLimit && position->pvTable[0][0] != 0) || (time->isNodeLimit && position->nodes_searched >= time->node_limit)) {
            time->stopped = 1;
        }

        int window = ASP_WINDOW_BASE;
        int aspirationWindowDepth = current_depth;

        while (true) {

            if ((time->timeset && (startTime >= time->softLimit) && position->pvTable[0][0] != 0) || (time->isNodeLimit && position->nodes_searched >= time->node_limit)) {
                time->stopped = 1;
            }

            if (time->stopped == 1) {
                break;
            }

            if (current_depth >= ASP_WINDOW_MIN_DEPTH) {
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
                int exceed = score - beta;

                beta = myMIN(infinity, beta + window);
                aspirationWindowDepth = myMAX(aspirationWindowDepth - 1, current_depth - 5);

                // If we failed high significantly (at least delta), increase delta more
                if (exceed) {
                    window += window / 4;
                }

            } else {
                break;
            }
            window *= 1.8f;

        }

        baseSearchScore = current_depth == 1 ? score : baseSearchScore;
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

        // Complexity TM
        double complexity = 0;
        if (abs(score) < mateValue) {
            complexity = 0.6 * abs(baseSearchScore - score) * log(depth);
        }

        if (time->timeset && current_depth > 6) {
            scaleTime(time, bestMoveStability, evalStability, position->pvTable[0][0], complexity, position);
        }
        
        int endTime = getTimeMiliSecond();
        totalTime += endTime - startTime;

        if (position->pvLength[0] && !benchmark) {
            unsigned long long nps = (totalTime > 0) ? (position->nodes_searched * 1000) / totalTime : 0;

            printf("info depth %d seldepth %d ", current_depth, position->seldepth);

            if (is_mate_score(score))
                printf("score mate %d nodes %llu nps %llu hashfull %d time %d pv ",
                       (score > 0 ? mateValue - score + 1 : -mateValue - score) / 2,
                       position->nodes_searched, nps, hash_full(), totalTime);            
            else
                printf("score cp %d nodes %llu nps %llu hashfull %d time %d pv ",
                       score, position->nodes_searched, nps, hash_full(), totalTime);

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
