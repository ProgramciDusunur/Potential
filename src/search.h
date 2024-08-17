//
// Created by erena on 31.05.2024.
//
#pragma once

#include "board.h"
#include "math.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "history.h"
#include <values.h>
#include "time.h"
#include "table.h"
#include "move.h"
#include "evaluation.h"
#include "uci.h"


#define maxPly 64

static int lmr_full_depth_moves = 4;
static int lmr_reduction_limit = 3;
static int lateMovePruningBaseReduction = 4;
static int nullMoveDepth = 3;

static U64 nodes = 0, variant = 0;

static int lmrTable[maxPly][maxPly];
static int counterMoves[2][maxPly][maxPly];


// position repetition detection
static inline int isRepetition(board* position) {
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

static inline void initializeLMRTable() {
    for (int i = 1; i < maxPly; ++i) {
        for (int j = 1; j < maxPly; ++j) {
            lmrTable[i][j] = round(1.2 + log(i) * log(j) * 0.6);
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
static inline int scoreMove(int move, board* position) {
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
        /*int seeScore = see(position, move);
        if (seeScore > 0) {
            return 15000;
        } else if (seeScore == 0) {
            return 14000;
        } else {
            return -10000;
        }*/

    }

        // score quiet move
    else {

        // score 1st killer move
        if (position->killerMoves[position->ply][0] == move)
            return 900000000;

            // score 2nd killer move
        else if (position->killerMoves[position->ply][1] == move)
            return 800000000;
        /*else if (counterMoves[position->side][getMoveSource(move)][getMoveTarget(move)] == move)
            return 700000000;*/

        /*if (historyMoves[getMoveSource(move)][getMoveTarget(move)] < 0) {
             printf("History score negative: %d\n", historyMoves[getMoveSource(move)][getMoveTarget(move)]);
         }*/
        return historyMoves[getMoveSource(move)][getMoveTarget(move)];

    }
    return 0;
}

// sort moves in descending order
static inline int sort_moves(moves *moveList, int bestMove, board* position) {
    // move scores
    int move_scores[moveList->count];

    // score all the moves within a move list
    for (int count = 0; count < moveList->count; count++) {
        // if hash move available
        if (bestMove == moveList->moves[count])
            // score move
            move_scores[count] = 2000000000;

        else
            // score move
            move_scores[count] = scoreMove(moveList->moves[count], position);
    }

    // loop over current move within a move list
    for (int current_move = 0; current_move < moveList->count; current_move++) {
        // loop over next move within a move list
        for (int next_move = current_move + 1; next_move < moveList->count; next_move++) {
            // compare current and next move scores
            if (move_scores[current_move] < move_scores[next_move]) {
                // swap scores
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;

                // swap moves
                int temp_move = moveList->moves[current_move];
                moveList->moves[current_move] = moveList->moves[next_move];
                moveList->moves[next_move] = temp_move;
            }
        }
    }
}

// enable PV move scoring
static inline void enable_pv_scoring(moves *moveList, board* position) {
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
static inline void printMove(int move) {
    if (getMovePromoted(move)) {
        printf("%s%s%c", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               promotedPieces[getMovePromoted(move)]);
    } else {
        printf("%s%s", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)]);
    }
}


static inline int getLmrReduction(int depth, int moveNumber, bool isPv) {
    int reduction = lmrTable[depth][moveNumber];
    // Reduce Less
    /*if (isPv) {
        reduction -= 1;
    }*/
    // Reduce More
    return reduction;
}

static inline void clearCounterMoves() {
    for (int m = 0;m < 2;m++) {
        for (int i = 0;i < 64;i++) {
            for (int j = 0;j < 64;j++) {
                counterMoves[m][i][j] = 0;
            }
        }
    }
}

// quiescence search
static inline int quiescence(int alpha, int beta, board* position, int negamaxScore, time* time) {
    if ((nodes & 2047) == 0) {
        communicate(time);
    }
    // increment nodes count
    nodes++;


    int pvNode = beta - alpha > 1;



    // best move (to store in TT)
    int bestMove = 0;

    // define hash flag
    int hashFlag = hashFlagAlpha;


    // read hash entry
    if (position->ply && (negamaxScore = readHashEntry(alpha, beta, &bestMove, 0, position)) != noHashEntry && pvNode == 0) {
        // if the move has already been searched (hence has a value)
        // we just return the score for this move
        return negamaxScore;
    }

    // evaluate position
    int evaluation = evaluate(position);

    // fail-hard beta cutoff
    if (evaluation >= beta) {
        // node (move) fails high
        return beta;
    }


    // found a better move
    if (evaluation > alpha) {
        // PV node (move)
        alpha = evaluation;
    }

    // create move list instance
    moves moveList[1];

    // generate moves
    moveGenerator(moveList, position);

    // sort moves
    sort_moves(moveList, 0, position);

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        //if (see(position, moveList->moves[count]) < 0) continue;
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



        // score current move
        int score = -quiescence(-beta, -alpha, position, score, time);

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // take move back
        takeBack(position, &copyPosition);

        if (time->stopped == 1) return 0;


        // found a better move
        if (score > alpha) {
            // PV node (move)
            alpha = score;

            bestMove = moveList->moves[count];

            hashFlag = hashFlagExact;
            // fail-hard beta cutoff
            if (score >= beta) {
                writeHashEntry(beta, bestMove, 0, hashFlagBeta, position);
                // node (move) fails high
                return beta;
            }
        }

    }
    writeHashEntry(alpha, bestMove, 0, hashFlag, position);
    // node (move) fails low
    return alpha;
}


// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth, board* position, time* time, bool cutNode) {
    // variable to store current move's score (from the static evaluation perspective)
    int score;

    // best move (to store in TT)
    int bestMove = 0;

    // define hash flag
    int hashFlag = hashFlagAlpha;

    if ((nodes & 2047) == 0) {
        communicate(time);
    }

    if (position->ply && isRepetition(position)) {
        return 0;
    }

    int pvNode = beta - alpha > 1;

    int rootNode = position->ply == 0;

    int ttBound = readHashFlag(position);

    bool improving;

    int pastStack;

    // read hash entry
    if (position->ply && (score = readHashEntry(alpha, beta, &bestMove, depth, position)) != noHashEntry && pvNode == 0) {
        // if the move has already been searched (hence has a value)
        // we just return the score for this move
        return score;
    }
    // init PV length
    position->pvLength[position->ply] = position->ply;

    // recursion escapre condition
    if (depth == 0)
        // run quiescence search
        return quiescence(alpha, beta, position, score, time);

    // IIR by Ed Schroder (~15 Elo)
    if (depth >= 4 && ttBound == hashFlagNone)
        depth--;

    // increment nodes count
    nodes++;

    // is king in check
    int in_check = isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[K]) :
                                    getLS1BIndex(position->bitboards[k]),
                                    position->side ^ 1, position);

    // increase search depth if the king has been exposed into a check
    if (in_check) depth++;

    // legal moves counter
    int legal_moves = 0;

    // quiet move counter
    int quietMoves = 0;

    // capture move counter
    int captureMoves = 0;

    // get static evaluation score
    int static_eval = evaluate(position);

    position->staticEval[position->ply] = static_eval;

    position->improvingRate[position->ply] = 0.0;


    if (position->staticEval[position->ply-2] != noEval) {
        pastStack = position->ply - 2;
    }
    if (position->staticEval[position->ply-4] != noEval) {
        pastStack = position->ply - 4;
    }

    if (pastStack) {
        const double diff = position->staticEval[position->ply] - position->staticEval[pastStack];
        position->improvingRate[position->ply] = fmin(fmax(position->improvingRate[position->ply] + diff / 25, -3.0), 2.0);
    }

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

    int canPrune = in_check == 0 && pvNode == 0;


    // Reverse futility pruning
    if (depth < 6 && canPrune && abs(beta - 1) > -infinity + 100) {
        // improving factor
        int improvingMargin = (int)((position->improvingRate[position->ply] / 2 ) * 20);

        // define evaluation margin
        int eval_margin = (100 + improvingMargin) * depth;

        // evaluation margin substracted from static evaluation score fails high
        if (static_eval - eval_margin >= beta)
            // evaluation margin substracted from static evaluation score
            return static_eval - eval_margin;
    }

    // null move pruning
    if (depth >= nullMoveDepth && in_check == 0 && position->ply) {
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

        int R = 3 + (int)(0.1875 * depth);

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
            return beta;
    }

    // razoring (~8 Elo)
    if (canPrune && depth <= 3) {
        // get static eval and add first bonus
        score = static_eval + 125;

        // define new score
        int new_score;

        // static evaluation indicates a fail-low node
        if (score < beta) {
            // on depth 1
            if (depth == 1) {
                // get quiscence score
                new_score = quiescence(alpha, beta, position, score, time);

                // return quiescence score if it's greater then static evaluation score
                return (new_score > score) ? new_score : score;
            }

            // add second bonus to static evaluation
            score += 175;

            // static evaluation indicates a fail-low node
            if (score < beta && depth <= 2) {
                // get quiscence score
                new_score = quiescence(alpha, beta, position, score, time);

                // quiescence score indicates fail-low node
                if (new_score < beta)
                    // return quiescence score if it's greater than static evaluation score
                    return (new_score > score) ? new_score : score;
            }
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
    sort_moves(moveList, bestMove, position);

    // number of moves searched in a move list
    int moves_searched = 0;

    int skipQuiet = 0;


    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
        int currentMove = moveList->moves[count];

        bool isQuiet = getMoveCapture(currentMove) == 0;
        if (skipQuiet && isQuiet) {
            continue;
        }

        bool isNotMated = alpha > -mateScore + maxPly;

        int lmpBase = 4;
        int lmpMultiplier = 2;
        int lmpThreshold = (lmpBase + lmpMultiplier * depth * depth);
        // Late Move Pruning (~18 Elo)
        if (!rootNode && isQuiet &&
            isNotMated &&
            legal_moves>= lmpThreshold) {
            skipQuiet = 1;
        }
        /*int seeScore = see(position, moveList->moves[count]);
        if (in_check == 0 && seeScore < -17 * depth * depth) {
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
        if (makeMove(moveList->moves[count], allMoves, position) == 0) {
            // decrement ply
            position->ply--;

            // decrement repetition index
            position->repetitionIndex--;

            // skip to next move
            continue;
        }

        /*if (isQuiet) {
            addMoveToHistoryList(badQuiets, currentMove);
        }*/

        // increment legal moves
        legal_moves++;

        if (isQuiet) {
            quietMoves++;
        } else {
            captureMoves++;
        }






        // full depth search
        if (moves_searched == 0)
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1, position, time, false);

            // late move reduction (LMR)
        else {
            int lmrReduction = getLmrReduction(depth, position->ply, pvNode);
            if (isQuiet) {
                // Reduce More
                /*if (!improving && quietMoves >= 8 * depth && !pvNode) {
                    lmrReduction += 1;
                }*/
                if (!pvNode && quietMoves >= 4) {
                    lmrReduction += 1;
                }

                /*if (position->improvingRate[position->ply] < -2.0) {
                    //printf("improving rate calculated %f\n", position->improvingRate[position->ply]);
                    lmrReduction += 1;
                }*/

                // Reduce Less
                if (position->killerMoves[position->ply][0] == bestMove || position->killerMoves[position->ply][1] == bestMove) {
                    lmrReduction -= 1;
                }
                /*if (in_check) {
                    lmrReduction -= 1;
                }*/
                /*if (pvNode && moves_searched <= 10) {
                    lmrReduction -= 1;
                }*/


            } else {
                // Reduce More
                if (cutNode) {
                    lmrReduction += 1;
                }
                /*if (pvNode && captureMoves >= 8 && moves_searched >= 4) {
                    lmrReduction -= 1;
                }*/
            }
            // condition to consider LMR
            if (moves_searched >= lmr_full_depth_moves &&
                depth >= lmr_reduction_limit &&
                getMovePromoted(currentMove) == 0) {
                // search current move with reduced depth:
                if (pvNode) {
                    score = -negamax(-alpha - 1, -alpha, depth - lmrReduction, position, time, false);
                } else {
                    score = -negamax(-alpha - 1, -alpha, depth - lmrReduction, position, time, !cutNode);
                }

            }


                // hack to ensure that full-depth search is done
            else {
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
                if ((score > alpha) && (score < beta))
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
        if (score > alpha) {
            // switch hash flag from storing for fail-low node
            // to the one storing score for PV node
            hashFlag = hashFlagExact;

            // store best move (for TT)
            bestMove = currentMove;

            // on quiet moves
            /*if (getMoveCapture(currentMove) == 0)
                // store history moves
                position->historyMoves[getMovePiece(currentMove)][getMoveTarget(currentMove)] += depth;*/

            // PV node (move)
            alpha = score;

            // write PV move
            position->pvTable[position->ply][position->ply] = currentMove;

            // loop over the next ply
            for (int next_ply = position->ply + 1; next_ply < position->pvLength[position->ply + 1]; next_ply++)
                // copy move from deeper ply into a current ply's line
                position->pvTable[position->ply][next_ply] = position->pvTable[position->ply + 1][next_ply];

            // adjust PV length
            position->pvLength[position->ply] = position->pvLength[position->ply + 1];

            // fail-hard beta cutoff
            if (score >= beta) {
                // store hash entry with the score equal to beta
                writeHashEntry(beta, bestMove, depth, hashFlagBeta, position);
                //int lastMove = moveList->moves[position->ply - 1];
                // on quiet moves
                if (isQuiet) {
                    // store killer moves
                    if (position->killerMoves[position->ply][0] != bestMove) {
                        position->killerMoves[position->ply][1] = position->killerMoves[position->ply][0];
                        position->killerMoves[position->ply][0] = bestMove;
                    }
                    //position->killerMoves[position->ply][1] = position->killerMoves[position->ply][0];
                    //position->killerMoves[position->ply][0] = bestMove;
                    //counterMoves[position->side][getMoveSource(lastMove)][getMoveTarget(lastMove)] = currentMove;
                    updateHistory(bestMove, depth, badQuiets);
                }

                // node (move) fails high
                return beta;
            }/* else {
                if (isQuiet) {
                    addMoveToHistoryList(badQuiets, currentMove);
                }
            }*/
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
    // store hash entry with the score equal to alpha
    writeHashEntry(alpha, bestMove, depth, hashFlag, position);

    // node (move) fails low
    return alpha;
}



// search position for the best move
static inline void searchPosition(int depth, board* position, bool benchmark, time* time) {
    // define best score variable
    int score = 0;

    // reset "time is up" flag
    time->stopped = 0;

    // reset nodes counter
    nodes = 0;

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
            continue;
        }

        alpha = score - 50;
        beta = score + 50;

        int endTime = getTimeMiliSecond();
        totalTime += endTime - startTime;

        if (position->pvLength[0] && !benchmark) {
            unsigned long long nps = (totalTime > 0) ? (nodes * 1000) / totalTime : 0;

            printf("info depth %d ", current_depth);

            if (score > -mateValue && score < -mateScore)
                printf("score mate %d nodes %llu nps %llu time %d pv ",
                       -(score + mateValue) / 2 - 1,
                       nodes, nps, totalTime);
            else if (score > mateScore && score < mateValue)
                printf("score mate %d nodes %llu nps %llu time %d pv ",
                       (mateValue - score) / 2 + 1,
                       nodes, nps, totalTime);
            else
                printf("score cp %d nodes %llu nps %llu time %d pv ",
                       score, nodes, nps, totalTime);

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

