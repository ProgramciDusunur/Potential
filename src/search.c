//
// Created by erena on 13.09.2024.
//

#include "search.h"




int lmr_full_depth_moves = 4;
int lmr_reduction_limit = 3;
int lateMovePruningBaseReduction = 4;
int nullMoveDepth = 3;

U64 searchNodes = 0;

int lmrTable[maxPly][maxPly];
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

void initializeLMRTable(void) {
    for (int i = 1; i < maxPly; ++i) {
        for (int j = 1; j < maxPly; ++j) {
            lmrTable[i][j] = round(1.0 + log(i) * log(j) * 0.5);
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

        // score quiet move
    else {

        /*// score 1st killer move
        if (position->killerMoves[position->ply][0] == move)
            return 900000000;

            // score 2nd killer move
        else if (position->killerMoves[position->ply][1] == move)
            return 800000000;
        else if (counterMoves[position->side][getMoveSource(move)][getMoveTarget(move)] == move)
            return 700000000;*/

        return historyMoves[getMoveSource(move)][getMoveTarget(move)];

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
    int reduction = lmrTable[depth][moveNumber];
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

// quiescence search
int quiescence(int alpha, int beta, board* position, time* time) {
    if ((searchNodes & 2047) == 0) {
        communicate(time);
    }

    int score = 0;

    //int pvNode = beta - alpha > 1;

    //int rootNode = position->ply == 0;

    // best move (to store in TT)
    //int bestMove = 0;

    // define hash flag
    //int hashFlag = hashFlagAlpha;


    // read hash entry
    /*if (position->ply && (negamaxScore = readHashEntry(alpha, beta, &bestMove, 0, position)) != noHashEntry && pvNode == 0) {
        // if the move has already been searched (hence has a value)
        // we just return the score for this move
        return negamaxScore;
    }*/

    // evaluate position
    int evaluation = evaluate(position);

    int bestScore = evaluation;

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

        /*if (!SEE(position, moveList->moves[count], QS_SEE_THRESHOLD))
        {
            continue;
        }*/
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

    //writeHashEntry(alpha, bestMove, 0, hashFlag, position);
    // node (move) fails low
    return bestScore;
}


// negamax alpha beta search
int negamax(int alpha, int beta, int depth, board* position, time* time, bool predictedCutNode) {
    // init PV length
    position->pvLength[position->ply] = position->ply;


    // variable to store current move's score (from the static evaluation perspective)
    int score = 0;





    if ((searchNodes & 2047) == 0) {
        communicate(time);
    }

    if (position->ply && isRepetition(position)) {
        return 0;
    }

    int pvNode = beta - alpha > 1;

    int rootNode = position->ply == 0;

    int bestMove = 0;
    int tt_move = 0;
    int16_t tt_score = 0;
    uint8_t tt_hit = 0;
    uint8_t tt_depth = 0;
    uint8_t tt_flag = hashFlagExact;

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
    if (depth == 0)
        // run quiescence search
        return quiescence(alpha, beta, position, time);

    // Internal Iterative Reductions
    if ((pvNode || predictedCutNode) && depth >= 8 && !tt_move) {
        depth--;
    }

    // is king in check
    int in_check = isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[K]) :
                                    getLS1BIndex(position->bitboards[k]),
                                    position->side ^ 1, position);


    // legal moves counter
    int legal_moves = 0;

    // quiet move counter
    //int quietMoves = 0;

    // capture move counter
    //int captureMoves = 0;

    // get static evaluation score
    int static_eval = evaluate(position);

    /*position->staticEval[position->ply] = static_eval;

    position->improvingRate[position->ply] = 0.0;

    if (position->staticEval[position->ply-2] != noEval) {
        pastStack = position->ply - 2;
    } else if (position->staticEval[position->ply-4] != noEval) {
        pastStack = position->ply - 4;
    }

    if (pastStack) {
        const double diff = position->staticEval[position->ply] - position->staticEval[pastStack];
        position->improvingRate[position->ply] = fmin(fmax(position->improvingRate[position->ply] + diff / 50, -3.0), 2.0);
    }*/

    /*if(in_check)
        improving = false;
    else if (position->staticEval[position->ply-2] != noEval) {
        improving = position->staticEval[position->ply] > position->staticEval[position->ply-2];
    }
    else if (position->staticEval[position->ply-4] != noEval) {
        improving = position->staticEval[position->ply] > position->staticEval[position->ply-4];
    }
    else
        improving = true;*/

    //printf("static eval calculated %d\n", position->staticEval[position->ply]);

    bool canPrune = in_check == 0 && pvNode == 0;


    // evaluation pruning / static null move pruning
    // reverse futility pruning
    if (depth <= 2 && !pvNode && !in_check && static_eval - 82 * depth >= beta)
        return static_eval;


    // null move pruning
    if (depth >= nullMoveDepth && in_check == 0 && !rootNode) {
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

        int R = 3;

        /* search moves with reduced depth to find beta cutoffs
           depth - R where R is a reduction limit */
        score = -negamax(-beta, -beta + 1, depth - R, position, time, !predictedCutNode);

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

    const int originalAlpha = alpha;

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        int currentMove = moveList->moves[count];

        bool isQuiet = getMoveCapture(currentMove) == 0;

        if (skipQuiet && isQuiet) {
            skipQuiet = 0;
            continue;
        }

        bool isNotMated = alpha > -mateScore + maxPly;

        if (!rootNode && isQuiet && isNotMated) {

            if (canPrune && depth <= 2 && static_eval + 82 * depth <= alpha) {
                skipQuiet = 1;
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
            //quietMoves++;
        } else {
            //captureMoves++;
        }




        // full-depth search
        if (moves_searched == 0) {
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1, position, time, false);
        } else {
            // condition to consider LMR
            if(moves_searched >= lmr_full_depth_moves &&
               depth >= lmr_reduction_limit) {
                // search current move with reduced depth:
                score = -negamax(-alpha - 1, -alpha, depth - 2, position, time, true);
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
                        updateQuietMoveHistory(bestMove, depth, badQuiets);
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
    memset(historyMoves, 0, sizeof(historyMoves));
    memset(position->pvTable, 0, sizeof(position->pvTable));
    memset(position->pvLength, 0, sizeof(position->pvLength));
    memset(position->staticEval, 0, sizeof(position->staticEval));
    //memset(time, 0, sizeof(*time));
    //memset(counterMoves, 0, sizeof(counterMoves));

    // define initial alpha beta bounds
    int alpha = -infinity;
    int beta = infinity;

    int totalTime = 0;

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (time->stopped == 1) {
            break;
        }

        int startTime = getTimeMiliSecond();
        position->followPv = 1;
        // find best move within a given position
        score = negamax(alpha, beta, current_depth, position, time, false);

        if (score <= alpha || score >= beta) {
            alpha = -infinity;
            beta = infinity;
            current_depth -= 1;
            continue;
        }

        alpha = score - 50;
        beta = score + 50;


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
