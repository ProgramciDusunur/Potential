#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#include "move.c"
#include "time.c"
#include "evaluation.c"
#include "table.c"
#include "mask.c"
#include "fen.c"
#include "magic.c"
#include "see.c"
#include "bit_manipulation.c"
#include "test/see_test.h"


#ifdef WIN64

#include <windows.h>

#else
#include <sys/time.h>
#endif


// random side key
U64 sideKey;


// Pawn attack masks pawnAttacks[side][square]
U64 pawnAtacks[2][64];
// Knight attack masks knightAttacks[square]
U64 knightAttacks[64];
// King attack masks kingAttacks[square]
U64 kingAttacks[64];
// Bishop attack table [square][occupancies]
U64 bishopAttacks[64][512];
// Rook attack table [square][occupancies]
U64 rookAttacks[64][4096];



// performance test node count, variant count
U64 nodes, variant;

/*   Zobrist Hashing   */

// random piece keys [piece][square]
U64 pieceKeys[12][64];
// random enpassant keys [square]
U64 enpassantKeys[64];
// random castling keys
U64 castleKeys[16];



U64 getRandom64Numbers();

U64 generateMagicNumber();

U64 findMagicNumber(int square, int relevantBits, int bishop);

static inline void perft(int depth, board* position);

static inline void addMove(moves *moveList, int move);

static inline int makeMove(int move, int moveFlag, board* position);

static inline int isSquareAttacked(int square, int side, board* position);

unsigned int getRandom32BitNumber();

int countBits(U64 bitboard);

int getLS1BIndex(U64 bitboard);

int getTimeMiliSecond();

int areSubStringsEqual(char *command, char *uciCommand, int stringSize);

void pBitboard(U64 bitboard);

void printAttackedSquares(int side, board* position);

void pBoard(board* position);

void printMove(int move);

void printMoveList(moves *moveList);

void initAll();

void initSlidersAttacks(int bishop);

void initLeaperAttacks();

void initMagicNumbers();

void parseFEN(char *fen, board* position);

void searchPosition(int depth, board* position);

void uciProtocol();

void goCommand(char *command, board* position);

void communicate();

void read_input();

static inline void moveGenerator(moves *moveList, board* position);

static inline void perftRoot(int depth, board* position);

static inline void perftChild(int depth, board* position);

static inline int evaluate(board* position);

static inline int negamax(int alpha, int beta, int depth, board* position);

static inline int quiescence(int alpha, int beta, board* positio, int score);

static inline int scoreMove(int move, board* position);

static inline int sort_moves(moves *moveList, int bestMove, board* position);

static inline void enable_pv_scoring(moves *moveList, board* position);

void initRandomKeys();


int main() {
    initAll();
    int debug = 0;
    if (debug) {
        // SEE material score (Opening) 82 = pawn, 337 = knight, 365 = bishop, 477, 1025 = Queen, 12000 = King
        see_test_case tests[30];
        tests[0].fen   = "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - ";
        tests[0].move  = encodeMove(e1, e5, R, 0, 1, 0, 0, 0);
        tests[0].score = 82;

        tests[1].fen   = "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - ";
        tests[1].move  = encodeMove(d3, e5, N, 0, 1, 0, 0, 0);
        tests[1].score = -255;

        tests[2].fen   = "4r1k1/5pp1/nbp4p/1p2p2q/1P2P1b1/1BP2N1P/1B2QPPK/3R4 b - -";
        tests[2].move  = encodeMove(g4, f3, b, 0, 1, 0, 0, 0);
        tests[2].score = -28;

        tests[3].fen   = "2r1r1k1/pp1bppbp/3p1np1/q3P3/2P2P2/1P2B3/P1N1B1PP/2RQ1RK1 b - -";
        tests[3].move  = encodeMove(d6, e5, p, 0, 1, 0, 0, 0);
        tests[3].score = 82;

        tests[4].fen   = "7r/5qpk/p1Qp1b1p/3r3n/BB3p2/5p2/P1P2P2/4RK1R w - -";
        tests[4].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
        tests[4].score = 0;

        tests[5].fen   = "6rr/6pk/p1Qp1b1p/2n5/1B3p2/5p2/P1P2P2/4RK1R w - -";
        tests[5].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
        tests[5].score = -477;

        tests[6].fen   = "7r/5qpk/2Qp1b1p/1N1r3n/BB3p2/5p2/P1P2P2/4RK1R w - -";
        tests[6].move  = encodeMove(e1, e8, R, 0, 0, 0, 0, 0);
        tests[6].score = -477;

        tests[7].fen   = "6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -";
        tests[7].move  = encodeMove(f7, f8, P, Q, 0, 0, 0, 0);
        tests[7].score = 283;

        tests[8].fen   = "6RR/4bP2/8/8/5r2/3K4/5p2/4k3 w - -";
        tests[8].move  = encodeMove(f7, f8, P, N, 0, 0, 0, 0);
        tests[8].score = 255;

        tests[9].fen   = "7R/5P2/8/8/8/3K2r1/5p2/4k3 w - -";
        tests[9].move  = encodeMove(f7, f8, P, Q, 0, 0, 0, 0);
        tests[9].score = 943;

        tests[10].fen   = "7R/5P2/8/8/8/3K2r1/5p2/4k3 w - -";
        tests[10].move  = encodeMove(f7, f8, P, B, 0, 0, 0, 0);
        tests[10].score = 283;

        tests[11].fen   = "7R/4bP2/8/8/1q6/3K4/5p2/4k3 w - -";
        tests[11].move  = encodeMove(f7, f8, P, R, 0, 0, 0, 0);
        tests[11].score = -82;

        tests[12].fen   = "8/4kp2/2npp3/1Nn5/1p2PQP1/7q/1PP1B3/4KR1r b - -";
        tests[12].move  = encodeMove(h1, f1, R, 0, 1, 0, 0, 0);
        tests[12].score = 0;

        tests[13].fen   = "8/4kp2/2npp3/1Nn5/1p2P1P1/7q/1PP1B3/4KR1r b - -";
        tests[13].move  = encodeMove(h1, f1, R, 0, 1, 0, 0, 0);
        tests[13].score = 0;

        tests[14].fen   = "2r2r1k/6bp/p7/2q2p1Q/3PpP2/1B6/P5PP/2RR3K b - -";
        tests[14].move  = encodeMove(c5, c1, Q, 0, 1, 0, 0, 0);
        tests[14].score = -71;

        tests[15].fen   = "r2qk1nr/pp2ppbp/2b3p1/2p1p3/8/2N2N2/PPPP1PPP/R1BQR1K1 w kq --";
        tests[15].move  = encodeMove(f3, e5, N, 0, 1, 0, 0, 0);
        tests[15].score = 82;

        tests[16].fen   = "6r1/4kq2/b2p1p2/p1pPb3/p1P2B1Q/2P4P/2B1R1P1/6K1 w - -";
        tests[16].move  = encodeMove(f4, e5, B, 0, 1, 0, 0, 0);
        tests[16].score = 0;

        tests[17].fen   = "3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R4B/PQ3P1P/3R2K1 w - h6";
        tests[17].move  = encodeMove(g5, h6, P, 0, 1, 0, 1, 0);
        tests[17].score = 0;

        tests[18].fen   = "3q2nk/pb1r1p2/np6/3P2Pp/2p1P3/2R1B2B/PQ3P1P/3R2K1 w - h6";
        tests[18].move  = encodeMove(g5, h6, P, 0, 1, 0, 1, 0);
        tests[18].score = 82;

        tests[19].fen   = "2r4r/1P4pk/p2p1b1p/7n/BB3p2/2R2p2/P1P2P2/4RK2 w - -";
        tests[19].move  = encodeMove(c3, c8, R, 0, 1, 0, 0, 0);
        tests[19].score = 477;

        tests[20].fen   = "2r5/1P4pk/p2p1b1p/5b1n/BB3p2/2R2p2/P1P2P2/4RK2 w - -";
        tests[20].move  = encodeMove(c3, c8, R, 0, 1, 0, 0, 0);
        tests[20].score = 365;

        /*
            Score of this test is 477 (Rook value) because after Rxc8, the bishop on f5 is not supposed
            to take back on c8 because it leads to pawn promotion. But in the current SEE implementation,
            the bishop on f5 takes on c8 leading to the SEE value being 365 (Bishop value).
        */

        tests[21].fen   = "2r4k/2r4p/p7/2b2p1b/4pP2/1BR5/P1R3PP/2Q4K w - -";
        tests[21].move  = encodeMove(c3, c5, R, 0, 1, 0, 0, 0);
        tests[21].score = 365;

        tests[22].fen   = "8/pp6/2pkp3/4bp2/2R3b1/2P5/PP4B1/1K6 w - -";
        tests[22].move  = encodeMove(g2, c6, B, 0, 1, 0, 0, 0);
        tests[22].score = -283;

        tests[23].fen   = "4q3/1p1pr1k1/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -";
        tests[23].move  = encodeMove(e6, e4, r, 0, 1, 0, 0, 0);
        tests[23].score = -395;

        tests[24].fen   = "4q3/1p1pr1kb/1B2rp2/6p1/p3PP2/P3R1P1/1P2R1K1/4Q3 b - -";
        tests[24].move  = encodeMove(h7, e4, b, 0, 1, 0, 0, 0);
        tests[24].score = 82;

        tests[25].fen = "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -";
        tests[25].move = encodeMove(e1, e5, R, 0, 1, 0, 0, 0);
        tests[25].score = 82;

        tests[26].fen = "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -";
        tests[26].move = encodeMove(d3, e5, N, 0, 1, 0, 0, 0);
        tests[26].score = -255;

        board position[1];

        for (int i=0; i < 26; i++) {
            parseFEN(tests[i].fen, position);
            int seeScore = see(position, tests[i].move);
            //printf("See score: %d", seeScore);
            if (seeScore != tests[i].score) {
                //fprintf(stderr, "Test %d failed for fen: %s\n", i+1, tests[i].fen);

                printf("Test %d failed for fen: %s expected Score: %d but see score: %d\n", i+1, tests[i].fen, tests[i].score, seeScore);
                //exit(1);
            }
        }
    } else {
        uciProtocol();
    }
    return 0;
}

// get game phase score
static inline int get_game_phase_score(board* position) {
    /*
        The game phase score of the game is derived from the pieces
        (not counting pawns and kings) that are still on the board.
        The full material starting position game phase score is:

        4 * knight material score in the opening +
        4 * bishop material score in the opening +
        4 * rook material score in the opening +
        2 * queen material score in the opening
    */

    // white & black game phase scores
    int white_piece_scores = 0, black_piece_scores = 0;

    // loop over white pieces
    for (int piece = N; piece <= Q; piece++)
        white_piece_scores += countBits(position->bitboards[piece]) * material_score[opening][piece];


    // loop over white pieces
    for (int piece = n; piece <= q; piece++)
        black_piece_scores += countBits(position->bitboards[piece]) * -material_score[opening][piece];



    // return game phase score
    return white_piece_scores + black_piece_scores;
}






// init random hash keys
void initRandomKeys() {
    // update pseudo random number state
    state = 1804289383;
    // loop over piece codes
    for (int piece = P; piece <= k; piece++) {
        // loop over board squares
        for (int square = 0; square < 64; square++) {
            pieceKeys[piece][square] = getRandom64Numbers();
        }
    }
    // loop over board squares
    for (int square = 0; square < 64; square++) {
        // init random enpassant keys
        enpassantKeys[square] = getRandom64Numbers();
    }
    // loop over castling keys
    for (int index = 0; index < 16; index++) {
        // init castling keys
        castleKeys[index] = getRandom64Numbers();
    }
    // loop over castling keys
    sideKey = getRandom64Numbers();
}

// parse user/GUI move string input (e.g. "e7e8q")
int parse_move(char *move_string, board* position) {
    // create move list instance
    moves moveList[1];

    // generate moves
    moveGenerator(moveList, position);

    // parse source square
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;

    // parse target square
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    // loop over the moves within a move list
    for (int move_count = 0; move_count < moveList->count; move_count++) {
        // init move
        int move = moveList->moves[move_count];

        // make sure source & target squares are available within the generated move
        if (source_square == getMoveSource(move) && target_square == getMoveTarget(move)) {
            // init promoted piece
            int promoted_piece = getMovePromoted(move);

            // promoted piece is available
            if (promoted_piece) {
                // promoted to queen
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    // return legal move
                    return move;

                    // promoted to rook
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    // return legal move
                    return move;

                    // promoted to bishop
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    // return legal move
                    return move;

                    // promoted to knight
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    // return legal move
                    return move;

                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }

            // return legal move
            return move;
        }
    }

    // return illegal move
    return 0;
}

// parse UCI "position" command
void parse_position(char *command, board* position) {
    // shift pointer to the right where next token begins
    command += 9;

    // init pointer to the current character in the command string
    char *current_char = command;

    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
        // init chess board with start position
        parseFEN(startPosition, position);

        // parse UCI "fen" command
    else {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");

        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            parseFEN(startPosition, position);

            // found "fen" substring
        else {
            // shift pointer to the right where next token begins
            current_char += 4;

            // init chess board with position from FEN string
            parseFEN(current_char, position);
        }
    }

    // parse moves after position
    current_char = strstr(command, "moves");

    // moves available
    if (current_char != NULL) {
        // shift pointer to the right where next token begins
        current_char += 6;

        // loop over moves within a move string
        while (*current_char) {
            // parse next move
            int move = parse_move(current_char, position);

            // if no more moves
            if (move == 0)
                // break out of the loop
                break;

            // increment repetition index
            position->repetitionIndex++;

            // write hash key into a repetition table
            position->repetitionTable[position->repetitionIndex] = position->hashKey;

            // make move on the chess board
            makeMove(move, allMoves, position);

            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;

            // go to the next move
            current_char++;
        }
    }

    // print board
    pBoard(position);
}


void uciProtocol() {
    board position;
    // reset STDIN & STDOUT buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define user / GUI input buffer
    char input[2000];

    // print engine info
    printf("id name Potential\n");
    printf("id name ProgramciDusunur\n");
    printf("uciok\n");

    // main loop
    while (1)
    {
        // reset user /GUI input
        memset(input, 0, sizeof(input));

        // make sure output reaches the GUI
        fflush(stdout);

        // get user / GUI input
        if (!fgets(input, 2000, stdin))
            // continue the loop
            continue;

        // make sure input is available
        if (input[0] == '\n')
            // continue the loop
            continue;

        // parse UCI "isready" command
        if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }

            // parse UCI "position" command
        else if (strncmp(input, "position", 8) == 0)
        {
            // call parse position function
            parse_position(input, &position);

            // clear hash table
            clearHashTable();
        }
            // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            // call parse position function
            parse_position("position startpos", &position);

            // clear hash table
            clearHashTable();
        }
            // parse UCI "go" command
        else if (strncmp(input, "go", 2) == 0)
            // call parse go function
            goCommand(input, &position);

            // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0)
            // quit from the chess engine program execution
            break;

            // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0)
        {
            // print engine info
            printf("id name Potential\n");
            printf("id name ProgramciDusunur\n");
            printf("uciok\n");
        }
    }
}

void goCommand(char *command, board* position) {

    // reset time control
    resetTimeControl();

    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command, "infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command, "binc")) && position->side == black)
        // parse black time increment
        inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command, "winc")) && position->side == white)
        // parse white time increment
        inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command, "wtime")) && position->side == white)
        // parse white time limit
        time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command, "btime")) && position->side == black)
        // parse black time limit
        time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command, "movestogo")))
        // parse number of moves to go
        movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command, "movetime")))
        // parse amount of time allowed to spend to make a move
        movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command, "depth")))
        // parse search depth
        depth = atoi(argument + 6);

    // if move time is not available
    if (movetime != -1) {
        // set time equal to move time
        time = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = getTimeMiliSecond();

    // init search depth
    depth = depth;

    // if time control is available
    if (time != -1) {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        time /= movestogo;

        // "illegal" (empty) move bug fix
        if (time > 1500) time -= 50;

        // init stoptime
        stoptime = starttime + time + inc;
    }

    // if depth is not available
    if (depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time, starttime, stoptime, depth, timeset);

    // search position
    searchPosition(depth, position);

}


void pBoard(board* position) {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (!file) {
                printf(" %d ", 8 - rank);
            }
            int piece = -1;
            for (int bbPiece = P; bbPiece <= k; bbPiece++) {
                if (getBit(position->bitboards[bbPiece], square)) {
                    piece = bbPiece;
                }
            }
#ifdef WIN64
            printf(" %c", (piece == -1) ? '.' : asciiPieces[piece]);
#else
            printf(" %s", (piece == -1) ? '.' : unicodePieces[piece]);
#endif
        }
        printf("\n");
    }
    printf("\n    a b c d e f g h\n\n");

    printf("    Side:     %s\n", !position->side ? "white" : "black");

    printf("    Enpassant: %s\n", (position->enpassant != no_sq) ? squareToCoordinates[position->enpassant] : "  no");

    printf("    Castling:  %c%c%c%c\n\n", (position->castle & wk) ? 'K' : '-',
           (position->castle & wq) ? 'Q' : '-',
           (position->castle & bk) ? 'k' : '-',
           (position->castle & bq) ? 'q' : '-');

    // print hash key
    printf("    Hash key:  %llx\n\n", position->hashKey);
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

// print move list
void printMoveList(moves *moveList) {
    /*if (!moveList->count) {
        printf("\n    No move in the move list.\n");
        return;
    }*/



    printf("\n  move   piece   capture   double   enpassant   castling");

    // loop over moves within a move list
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        int move = moveList->moves[moveCount];
#ifdef WIN64
        printf(" \n  %s%s%c   %c       %d         %d        %d           %d", squareToCoordinates[getMoveSource(move)],
               squareToCoordinates[getMoveTarget(move)],
               getMovePromoted(move) ? promotedPieces[getMovePromoted(move)] : ' ',
               asciiPieces[getMovePiece(move)],
               getMoveCapture(move) ? 1 : 0,
               getMoveDouble(move) ? 1 : 0,
               getMoveEnpassant(move) ? 1 : 0,
               getMoveCastling(move) ? 1 : 0);

#else
        printf(" \n  %s%s%c  %c       %d         %d        %d           %d",            squareToCoordinates[getMoveSource(move)],
                                                                                        squareToCoordinates[getMoveTarget(move)],
                                                                                        getMovePromoted(move) ? promotedPieces[getMovePromoted(move)] : ' ',
                                                                                        unicodePieces[getMovePiece(move)],
                                                                                        getMoveCapture(move) ? 1 : 0,
                                                                                        getMoveDouble(move) ? 1 : 0,
                                                                                        getMoveEnpassant(move) ? 1 : 0,
                                                                                        getMoveCastling(move) ? 1 : 0);
#endif
    }
    printf("\n\n  Total number of moves: %d\n\n", moveList->count);
}


static inline void perftRoot(int depth, board* position) {
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perftChild(depth - 1, position);
        printf("%s%s%c %llu \n", squareToCoordinates[getMoveSource(moveList->moves[moveCount])],
               squareToCoordinates[getMoveTarget(moveList->moves[moveCount])],
               promotedPieces[getMovePromoted(moveList->moves[moveCount])], variant);
        variant = 0;
        takeBack(position, &copyPosition);
    }
    printf("\n");

}

static inline void perftChild(int depth, board* position) {
    if (depth == 0) {
        nodes++;
        variant++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }

        // call perft driver recursively
        perftChild(depth - 1, position);
        takeBack(position, &copyPosition);
    }
}

static inline void perft(int depth, board* position) {
    if (depth == 0) {
        nodes++;
        return;
    }
    moves moveList[1];
    moveGenerator(moveList, position);
    for (int moveCount = 0; moveCount < moveList->count; moveCount++) {
        struct copyposition copyPosition;
        copyBoard(position, &copyPosition);
        if (makeMove(moveList->moves[moveCount], allMoves, position) == 0) {
            // skip to the next move
            continue;
        }
        // call perft driver recursively
        perft(depth - 1, position);
        takeBack(position, &copyPosition);
    }
}

// search position for the best move
void searchPosition(int depth, board* position) {
    // define best score variable
    int score = 0;

    // reset "time is up" flag
    stopped = 0;

    // reset nodes counter
    nodes = 0;

    // reset follow PV flags
    position->followPv = 0;
    position->scorePv = 0;

    memset(position->killerMoves, 0, sizeof(position->killerMoves));
    memset(position->historyMoves, 0, sizeof(position->historyMoves));
    memset(position->pvTable, 0, sizeof(position->pvTable));
    memset(position->pvLength, 0, sizeof(position->pvLength));

    // define initial alpha beta bounds
    int alpha = -infinity;
    int beta = infinity;

    int totalTime = 0;

    // iterative deepening
    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (stopped == 1) {
            break;
        }

        int startTime = getTimeMiliSecond();
        position->followPv = 1;
        // find best move within a given position
        score = negamax(alpha, beta, current_depth, position);

        if (score <= alpha || score >= beta) {
            alpha = -infinity;
            beta = infinity;
            continue;
        }

        alpha = score - 50;
        beta = score + 50;

        int endTime = getTimeMiliSecond();
        totalTime += endTime - startTime;

        if (position->pvLength[0]) {
            unsigned long long nps = (totalTime > 0) ? (nodes * 1000) / totalTime : 0;

            if (score > -mateValue && score < -mateScore)
                printf("info score mate %d depth %d nodes %llu nps %llu time %d pv ",
                       -(score + mateValue) / 2 - 1, current_depth,
                       nodes, nps, totalTime);
            else if (score > mateScore && score < mateValue)
                printf("info score mate %d depth %d nodes %llu nps %llu time %d pv ",
                       (mateValue - score) / 2 + 1, current_depth,
                       nodes, nps, totalTime);
            else
                printf("info score cp %d depth %d nodes %llu nps %llu time %d pv ",
                       score, current_depth, nodes, nps, totalTime);

            // loop over the moves within a PV line
            for (int count = 0; count < position->pvLength[0]; count++) {
                printMove(position->pvTable[0][count]);
                printf(" ");
            }
            // print new line
            printf("\n");
        }
    }
    // best move placeholder
    printf("bestmove ");
    printMove(position->pvTable[0][0]);
    printf("\n");
}


int areSubStringsEqual(char *command, char *uciCommand, int stringSize) {
    if (stringSize > strlen(command)) {
        return 0;
    }
    for (int index = 0; index < stringSize; index++) {
        if (*command != *uciCommand) {
            return 0;
        }
        command++;
        uciCommand++;
    }
    return 1;
}

// position evaluation
static inline int evaluate(board* position) {
    // get game phase score
    int game_phase_score = get_game_phase_score(position);

    // game phase (opening, middle game, endgame)
    int game_phase = -1;

    // pick up game phase based on game phase score
    if (game_phase_score > opening_phase_score) game_phase = opening;
    else if (game_phase_score < endgame_phase_score) game_phase = endgame;
    else game_phase = middlegame;


    // static evaluation score
    int score = 0;

    // current pieces bitboard copy
    U64 bitboard;

    // init piece & square
    int piece, square;

    // penalties
    int double_pawns = 0;

    // loop over piece bitboards
    for (int bb_piece = P; bb_piece <= k; bb_piece++) {
        // init piece bitboard copy
        bitboard = position->bitboards[bb_piece];

        // loop over pieces within a bitboard
        while (bitboard) {
            // init piece
            piece = bb_piece;

            // init square
            square = getLS1BIndex(bitboard);

            /*
                Now in order to calculate interpolated score
                for a given game phase we use this formula
                (same for material and positional scores):

                (
                  score_opening * game_phase_score +
                  score_endgame * (opening_phase_score - game_phase_score)
                ) / opening_phase_score

                E.g. the score for pawn on d4 at phase say 5000 would be
                interpolated_score = (12 * 5000 + (-7) * (6192 - 5000)) / 6192 = 8,342377261
            */

            // interpolate scores in middle_game
            if (game_phase == middlegame)
                score += (
                                 material_score[opening][piece] * game_phase_score +
                                 material_score[endgame][piece] * (opening_phase_score - game_phase_score)
                         ) / opening_phase_score;

                // score material weights with pure scores in opening or endgame
            else score += material_score[game_phase][piece];

            // score positional piece scores
            switch (piece) {
                // evaluate white pawns
                case P:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][PAWN][square] * game_phase_score +
                                         positional_score[endgame][PAWN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][PAWN][square];

                    /* double pawn penalty
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score += double_pawns * double_pawn_penalty;

                    // on isolated pawn
                    if ((bitboards[P] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score += isolated_pawn_penalty;

                    // on passed pawn
                    if ((white_passed_masks[square] & bitboards[p]) == 0)
                        // give passed pawn bonus
                        score += passed_pawn_bonus[get_rank[square]];
                    */
                    break;

                    // evaluate white knights
                case N:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KNIGHT][square] * game_phase_score +
                                         positional_score[endgame][KNIGHT][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KNIGHT][square];

                    break;

                    // evaluate white bishops
                case B:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][BISHOP][square] * game_phase_score +
                                         positional_score[endgame][BISHOP][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][BISHOP][square];

                    // mobility
                    //score += count_bits(get_bishop_attacks(square, occupancies[both]));

                    break;

                    // evaluate white rooks
                case R:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][ROOK][square] * game_phase_score +
                                         positional_score[endgame][ROOK][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][ROOK][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score += open_file_score;
                    */
                    break;

                    // evaluate white queens
                case Q:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][QUEEN][square] * game_phase_score +
                                         positional_score[endgame][QUEEN][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][QUEEN][square];

                    // mobility
                    //score += count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate white king
                case K:
                    /// interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score += (
                                         positional_score[opening][KING][square] * game_phase_score +
                                         positional_score[endgame][KING][square] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score += positional_score[game_phase][KING][square];

                    /* semi open file
                    if ((bitboards[P] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score -= open_file_score;

                    // king safety bonus
                    score += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    */
                    break;

                    // evaluate black pawns
                case p:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][PAWN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][PAWN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][PAWN][mirrorScore[square]];

                    /* double pawn penalty
                    double_pawns = count_bits(bitboards[p] & file_masks[square]);

                    // on double pawns (tripple, etc)
                    if (double_pawns > 1)
                        score -= double_pawns * double_pawn_penalty;

                    // on isolated pawnd
                    if ((bitboards[p] & isolated_masks[square]) == 0)
                        // give an isolated pawn penalty
                        score -= isolated_pawn_penalty;

                    // on passed pawn
                    if ((black_passed_masks[square] & bitboards[P]) == 0)
                        // give passed pawn bonus
                        score -= passed_pawn_bonus[get_rank[mirror_score[square]]];
                    */
                    break;

                    // evaluate black knights
                case n:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KNIGHT][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KNIGHT][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KNIGHT][mirrorScore[square]];

                    break;

                    // evaluate black bishops
                case b:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][BISHOP][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][BISHOP][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][BISHOP][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_bishop_attacks(square, occupancies[both]));
                    break;

                    // evaluate black rooks
                case r:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][ROOK][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][ROOK][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][ROOK][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file bonus
                        score -= open_file_score;
                    */
                    break;

                    // evaluate black queens
                case q:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][QUEEN][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][QUEEN][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][QUEEN][mirrorScore[square]];

                    // mobility
                    //score -= count_bits(get_queen_attacks(square, occupancies[both]));
                    break;

                    // evaluate black king
                case k:
                    // interpolate scores in middle_game
                    if (game_phase == middlegame)
                        score -= (
                                         positional_score[opening][KING][mirrorScore[square]] * game_phase_score +
                                         positional_score[endgame][KING][mirrorScore[square]] *
                                         (opening_phase_score - game_phase_score)
                                 ) / opening_phase_score;

                        // score material weights with pure scores in opening or endgame
                    else score -= positional_score[game_phase][KING][mirrorScore[square]];

                    /* semi open file
                    if ((bitboards[p] & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += semi_open_file_score;

                    // semi open file
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0)
                        // add semi open file penalty
                        score += open_file_score;

                    // king safety bonus
                    score -= count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    */
                    break;
            }

            // pop ls1b
            popBit(bitboard, square);
        }
    }

    // return final evaluation based on side
    return (position->side == white) ? score : -score;
}

int input_waiting() {
#ifndef WIN32
    fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
}


// read GUI/user input
void read_input() {
    // bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting()) {
        // tell engine to stop calculating
        stopped = 1;

        // loop to read bytes from STDIN
        do {
            // read bytes from STDIN
            bytes = read(fileno(stdin), input, 256);
        }

            // until bytes available
        while (bytes < 0);

        // searches for the first occurrence of '\n'
        endc = strchr(input, '\n');

        // if found new line set value at pointer to 0
        if (endc) *endc = 0;

        // if input is available
        if (strlen(input) > 0) {
            // match UCI "quit" command
            if (!strncmp(input, "quit", 4)) {
                // tell engine to terminate exacution
                quit = 1;
            }

                // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4)) {
                // tell engine to terminate exacution
                quit = 1;
            }
        }
    }
}


void communicate() {
    // if time is up break here
    if (timeset == 1 && getTimeMiliSecond() > stoptime) {
        // tell engine to stop calculating
        stopped = 1;
    }
    read_input();
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

/*  =======================
         Move ordering
    =======================

    1. PV move
    2. Captures in MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Unsorted moves
*/


// score moves
static inline int scoreMove(int move, board* position) {
    // if PV move scoring is allowed
    if (position->scorePv) {
        // make sure we are dealing with PV move
        if (position->pvTable[0][position->ply] == move) {
            // disable score PV flag
            position->scorePv = 0;

            // give PV move the highest score to search it first
            return 20000;
        }
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
        return mvvLva[getMovePiece(move)][target_piece] + 10000;
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
        if (position->killerMoves[0][position->ply] == move)
            return 9000;

            // score 2nd killer move
        else if (position->killerMoves[1][position->ply] == move)
            return 8000;

            // score history move
        else
            return position->historyMoves[getMovePiece(move)][getMoveTarget(move)];
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
            move_scores[count] = 30000;

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

// position repetition detection
static inline int isRepetition(board* position) {
    // loop over repetition indicies range
    for (int index = 0; index < position->repetitionIndex; index++) {
        // if we found the hash kkey same with a current
        if (position->repetitionTable[index] == position->hashKey) {
            // we found a repetition
            return 1;
        }
    }

    // if no repetition found
    return 0;
}


// quiescence search
static inline int quiescence(int alpha, int beta, board* position, int score) {
    if ((nodes & 2047) == 0) {
        communicate();
    }
    // increment nodes count
    nodes++;

    int pvNode = beta - alpha > 1;



    // best move (to store in TT)
    int bestMove = 0;

    // define hash flag
    int hashFlag = hashFlagAlpha;


    // read hash entry
    /*if (position->ply && (score = readHashEntry(alpha, beta, &bestMove, 0, position)) != noHashEntry && pvNode == 0) {
        // if the move has already been searched (hence has a value)
        // we just return the score for this move
        return score;
    }*/

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
        int score = -quiescence(-beta, -alpha, position, score);

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // take move back
        takeBack(position, &copyPosition);

        if (stopped == 1) return 0;


        // found a better move
        if (score > alpha) {
            // PV node (move)
            alpha = score;

            //bestMove = moveList->moves[count];

            //hashFlag = hashFlagExact;
            // fail-hard beta cutoff
            if (score >= beta) {
                //writeHashEntry(beta, bestMove, 0, hashFlagBeta, position);
                // node (move) fails high
                return beta;
            }
        }

    }
    //writeHashEntry(alpha, bestMove, 0, hashFlag, position);
    // node (move) fails low
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth, board* position) {
    // variable to store current move's score (from the static evaluation perspective)
    int score;

    // best move (to store in TT)
    int bestMove = 0;

    // define hash flag
    int hashFlag = hashFlagAlpha;

    if ((nodes & 2047) == 0) {
        communicate();
    }

    if (position->ply && isRepetition(position)) {
        return 0;
    }

    int pvNode = beta - alpha > 1;

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
        return quiescence(alpha, beta, position, score);



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

    // get static evaluation score
    int static_eval = evaluate(position);

    // evaluation pruning / static null move pruning
    if (depth < 4 && !pvNode && !in_check && abs(beta - 1) > -infinity + 100) {
        // define evaluation margin
        int eval_margin = 100 * depth;

        // evaluation margin substracted from static evaluation score fails high
        if (static_eval - eval_margin >= beta)
            // evaluation margin substracted from static evaluation score
            return static_eval - eval_margin;
    }

    // null move pruning
    if (depth >= 3 && in_check == 0 && position->ply) {
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


        /* search moves with reduced depth to find beta cutoffs
           depth - 1 - R where R is a reduction limit */
        score = -negamax(-beta, -beta + 1, depth - 1 - 2, position);

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // restore board state
        takeBack(position, &copyPosition);


        if (stopped == 1) return 0;

        // fail-hard beta cutoff
        if (score >= beta)

            // node (move) fails high
            return beta;
    }

    // razoring
    if (!pvNode && !in_check && depth <= 3) {
        // get static eval and add first bonus
        score = static_eval + 125;

        // define new score
        int new_score;

        // static evaluation indicates a fail-low node
        if (score < beta) {
            // on depth 1
            if (depth == 1) {
                // get quiscence score
                new_score = quiescence(alpha, beta, position, score);

                // return quiescence score if it's greater then static evaluation score
                return (new_score > score) ? new_score : score;
            }

            // add second bonus to static evaluation
            score += 175;

            // static evaluation indicates a fail-low node
            if (score < beta && depth <= 2) {
                // get quiscence score
                new_score = quiescence(alpha, beta, position, score);

                // quiescence score indicates fail-low node
                if (new_score < beta)
                    // return quiescence score if it's greater than static evaluation score
                    return (new_score > score) ? new_score : score;
            }
        }
    }

    // create move list instance
    moves moveList[1];

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

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++) {
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

        // increment legal moves
        legal_moves++;

        // full depth search
        if (moves_searched == 0)
            // do normal alpha beta search
            score = -negamax(-beta, -alpha, depth - 1, position);

            // late move reduction (LMR)
        else {
            // condition to consider LMR
            if (
                    moves_searched >= full_depth_moves &&
                    depth >= reduction_limit &&
                    in_check == 0 &&
                    getMoveCapture(moveList->moves[count]) == 0 &&
                    getMovePromoted(moveList->moves[count]) == 0


                    )
                // search current move with reduced depth:
                score = -negamax(-alpha - 1, -alpha, depth - 2, position);

                // hack to ensure that full-depth search is done
            else score = alpha + 1;

            // principle variation search PVS
            if (score > alpha) {
                /* Once you've found a move with a score that is between alpha and beta,
                   the rest of the moves are searched with the goal of proving that they are all bad.
                   It's possible to do this a bit faster than a search that worries that one
                   of the remaining moves might be good. */
                score = -negamax(-alpha - 1, -alpha, depth - 1, position);

                /* If the algorithm finds out that it was wrong, and that one of the
                   subsequent moves was better than the first PV move, it has to search again,
                   in the normal alpha-beta manner.  This happens sometimes, and it's a waste of time,
                   but generally not often enough to counteract the savings gained from doing the
                   "bad move proof" search referred to earlier. */
                if ((score > alpha) && (score < beta))
                    /* re-search the move that has failed to be proved to be bad
                       with normal alpha beta score bounds*/
                    score = -negamax(-beta, -alpha, depth - 1, position);
            }
        }

        // decrement ply
        position->ply--;

        // decrement repetition index
        position->repetitionIndex--;

        // take move back
        takeBack(position, &copyPosition);

        if (stopped == 1) return 0;

        // increment the counter of moves searched so far
        moves_searched++;

        // found a better move
        if (score > alpha) {
            // switch hash flag from storing for fail-low node
            // to the one storing score for PV node
            hashFlag = hashFlagExact;

            // store best move (for TT)
            bestMove = moveList->moves[count];

            // on quiet moves
            if (getMoveCapture(moveList->moves[count]) == 0)
                // store history moves
                position->historyMoves[getMovePiece(moveList->moves[count])][getMoveTarget(moveList->moves[count])] += depth;

            // PV node (move)
            alpha = score;

            // write PV move
            position->pvTable[position->ply][position->ply] = moveList->moves[count];

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

                // on quiet moves
                if (getMoveCapture(moveList->moves[count]) == 0) {
                    // store killer moves
                    position->killerMoves[1][position->ply] = position->killerMoves[0][position->ply];
                    position->killerMoves[0][position->ply] = moveList->moves[count];
                }

                // node (move) fails high
                return beta;
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
    // store hash entry with the score equal to alpha
    writeHashEntry(alpha, bestMove, depth, hashFlag, position);

    // node (move) fails low
    return alpha;
}

int getTimeMiliSecond() {
#ifdef WIN64
    return GetTickCount();
#else
    struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}




// init slider piece's attack tables
void initSlidersAttacks(int bishop) {
    // init bishop & rook masks
    for (int square = 0; square < 64; square++) {
        bishopMask[square] = maskBishopAttacks(square);
        rookMask[square] = maskRookAttacks(square);

        // init current mask
        U64 attackMask = bishop ? bishopMask[square] : rookMask[square];

        // init relevant occupancy bit count
        int relevantBitsCount = countBits(attackMask);

        // init occupancyIndicies
        int occupancIndicies = 1 << relevantBitsCount;

        // loop over occupancy indicies
        for (int index = 0; index < occupancIndicies; index++) {
            // bishop
            if (bishop) {
                // int current occupancy variation
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * bishopMagic[square]) >> (64 - bishopRelevantBits[square]);

                // init bishop attacks
                bishopAttacks[square][magicIndex] = bishopAttack(square, occupancy);
            }
                // rook
            else {
                U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);

                // init magic index
                int magicIndex = (occupancy * rookMagic[square]) >> (64 - rookRelevantBits[square]);

                // init bishop attacks
                rookAttacks[square][magicIndex] = rookAttack(square, occupancy);
            }
        }
    }
}

// make move on chess board
static inline int makeMove(int move, int moveFlag, board* position) {
    // quiet moves
    if (moveFlag == allMoves) {
        struct copyposition copyPosition;
        // preserve board state
        copyBoard(position, &copyPosition);

        // parse move
        int sourceSquare = getMoveSource(move);
        int targetSquare = getMoveTarget(move);
        int piece = getMovePiece(move);
        int promotedPiece = getMovePromoted(move);
        int capture = getMoveCapture(move);
        int doublePush = getMoveDouble(move);
        int enpass = getMoveEnpassant(move);
        int castling = getMoveCastling(move);

        // move piece
        popBit(position->bitboards[piece], sourceSquare);
        setBit(position->bitboards[piece], targetSquare);

        // hash piece
        position->hashKey ^= pieceKeys[piece][sourceSquare]; // remove piece from source square in hash key
        position->hashKey ^= pieceKeys[piece][targetSquare]; // set piece to the target square in hash key

        // handling capture moves
        if (capture) {
            int startPiece, endPiece;
            if (position->side == white) {
                startPiece = p;
                endPiece = k;
            } else {
                startPiece = P;
                endPiece = K;
            }
            for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++) {
                if (getBit(position->bitboards[bbPiece], targetSquare)) {
                    // remove it from corresponding bitboard
                    popBit(position->bitboards[bbPiece], targetSquare);

                    // remove the piece from hash key
                    position->hashKey ^= pieceKeys[bbPiece][targetSquare];
                    break;
                }
            }
        }
        // handle pawn promotions
        if (promotedPiece) {
            // white to move
            if (position->side == white) {
                // erase the pawn from the target square
                popBit(position->bitboards[P], targetSquare);

                // remove pawn from hash key
                position->hashKey ^= pieceKeys[P][targetSquare];
            }

                // black to move
            else {
                // erase the pawn from the target square
                popBit(position->bitboards[p], targetSquare);

                // remove pawn from hash key
                position->hashKey ^= pieceKeys[p][targetSquare];
            }

            // set up promoted piece on chess board
            setBit(position->bitboards[promotedPiece], targetSquare);

            // add promoted piece into the hash key
            position->hashKey ^= pieceKeys[promotedPiece][targetSquare];
        }

        // handle enpassant captures
        if (enpass) {
            // erase the pawn depending on side to move
            (position->side == white) ? popBit(position->bitboards[p], targetSquare + 8) :
            popBit(position->bitboards[P], targetSquare - 8);

            // white to move
            if (position->side == white) {
                // remove captured pawn
                popBit(position->bitboards[p], targetSquare + 8);

                // remove pawn from hash key
                position->hashKey ^= pieceKeys[p][targetSquare + 8];
            }

                // black to move
            else {
                // remove captured pawn
                popBit(position->bitboards[P], targetSquare - 8);

                // remove pawn from hash key
                position->hashKey ^= pieceKeys[P][targetSquare - 8];
            }
        }

        // hash enpassant if available (remove enpassant square from hash key )
        if (position->enpassant != no_sq) position->hashKey ^= enpassantKeys[position->enpassant];

        // reset enpassant square
        position->enpassant = no_sq;

        // handle double pawn push
        if (doublePush) {
            // white to move
            if (position->side == white) {
                // set enpassant square
                position->enpassant = targetSquare + 8;

                // hash enpassant
                position->hashKey ^= enpassantKeys[targetSquare + 8];
            }

                // black to move
            else {
                // set enpassant square
                position->enpassant = targetSquare - 8;

                // hash enpassant
                position->hashKey ^= enpassantKeys[targetSquare - 8];
            }
        }

        // handle castling moves
        if (castling) {
            switch (targetSquare) {
                // white castles king side
                case (g1):
                    // move H rook
                    popBit(position->bitboards[R], h1);
                    setBit(position->bitboards[R], f1);

                    // hash rook
                    position->hashKey ^= pieceKeys[R][h1];  // remove rook from h1 from hash key
                    position->hashKey ^= pieceKeys[R][f1];  // put rook on f1 into a hash key
                    break;

                    // white castles queen side
                case (c1):
                    // move A rook
                    popBit(position->bitboards[R], a1);
                    setBit(position->bitboards[R], d1);

                    // hash rook
                    position->hashKey ^= pieceKeys[R][a1];  // remove rook from a1 from hash key
                    position->hashKey ^= pieceKeys[R][d1];  // put rook on d1 into a hash key
                    break;

                    // black castles king side
                case (g8):
                    // move H rook
                    popBit(position->bitboards[r], h8);
                    setBit(position->bitboards[r], f8);

                    // hash rook
                    position->hashKey ^= pieceKeys[r][h8];  // remove rook from h8 from hash key
                    position->hashKey ^= pieceKeys[r][f8];  // put rook on f8 into a hash key
                    break;

                    // black castles queen side
                case (c8):
                    // move A rook
                    popBit(position->bitboards[r], a8);
                    setBit(position->bitboards[r], d8);

                    // hash rook
                    position->hashKey ^= pieceKeys[r][a8];  // remove rook from a8 from hash key
                    position->hashKey ^= pieceKeys[r][d8];  // put rook on d8 into a hash key
                    break;
            }
        }


        // hash castling
        position->hashKey ^= castleKeys[position->castle];

        // update castling rights
        position->castle &= castlingRights[sourceSquare];
        position->castle &= castlingRights[targetSquare];

        // hash castling
        position->hashKey ^= castleKeys[position->castle];

        // reset occupancies
        position->occupancies[white] = 0LL;
        position->occupancies[black] = 0LL,
        position->occupancies[both] = 0LL;

        // loop over white pieces bitboards
        for (int bbPiece = P; bbPiece <= K; bbPiece++) {
            // update white occupancies
            position->occupancies[white] |= position->bitboards[bbPiece];
        }
        // loop over black pieces bitboards
        for (int bbPiece = p; bbPiece <= k; bbPiece++) {
            // update black occupancies
            position->occupancies[black] |= position->bitboards[bbPiece];
        }
        // update both side occupancies
        position->occupancies[both] |= position->occupancies[white];
        position->occupancies[both] |= position->occupancies[black];

        // change side
        position->side ^= 1;

        // hash side
        position->hashKey ^= sideKey;

        U64 hash_from_scratch = generateHashKey(position);

        // make sure that king has not been exposed into a check
        if (isSquareAttacked((position->side == white) ? getLS1BIndex(position->bitboards[k]) : getLS1BIndex(position->bitboards[K]), position->side, position)) {
            // take move back
            takeBack(position, &copyPosition);
            // return illegal move
            return 0;
        }
        return 1;
    } else {
        if (getMoveCapture(move)) {
            makeMove(move, allMoves, position);
        } else {
            return 0;
        }
    }
}

void printAttackedSquares(int whichSide, board* position) {
    printf("\n");
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (!file) {
                printf("  %d  ", 8 - rank);
            }
            printf("%d ", isSquareAttacked(square, whichSide, position) ? 1 : 0);

        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n\n");

}

void initAll() {
    initLeaperAttacks();
    initMagicNumbers();
    initSlidersAttacks(bishop);
    initSlidersAttacks(rook);
    // init random keys for tranposition table
    initRandomKeys();
    // clear hash table
    clearHashTable();
}

void initLeaperAttacks() {
    for (int square = 0; square < 64; square++) {
        // init pawn attacks
        pawnAtacks[white][square] = maskPawnAttacks(white, square);
        pawnAtacks[black][square] = maskPawnAttacks(black, square);

        // init knight attacks
        knightAttacks[square] = maskKnightAttacks(square);

        // init king attacks
        kingAttacks[square] = maskKingAttacks(square);
    }
}