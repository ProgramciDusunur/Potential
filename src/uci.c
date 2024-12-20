//
// Created by erena on 3.07.2024.
//

#include "uci.h"


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



void goCommand(char *command, board* position, time* time) {

    // reset time control
    resetTimeControl(time);

    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command, "infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command, "binc")) && position->side == black)
        // parse black time increment
        time->inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command, "winc")) && position->side == white)
        // parse white time increment
        time->inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command, "wtime")) && position->side == white)
        // parse white time limit
        time->time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command, "btime")) && position->side == black)
        // parse black time limit
        time->time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command, "movestogo")))
        // parse number of moves to go
        time->movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command, "movetime")))
        // parse amount of time allowed to spend to make a move
        time->movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command, "depth")))
        // parse search depth
        depth = atoi(argument + 6);

    // if move time is not available
    if (time->movetime != -1) {
        // set time equal to move time
        time->time = time->movetime;

        // set moves to go to 1
        time->movestogo = 1;
    }

    // init start time
    time->starttime = getTimeMiliSecond();

    // init search depth
    //depth = depth;

    // if time control is available
    if (time->time != -1) {
        // flag we're playing with time control
        time->timeset = 1;

        // set up timing
        time->time /= time->movestogo;

        // init stoptime
        time->stoptime = time->starttime + time->time + (time->inc/2);
    }

    // if depth is not available
    if (depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time->time, time->starttime, time->stoptime, depth, time->timeset);

    // search position
    searchPosition(depth, position, false, time);

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
        printf(" \n  %s%s%c  %s       %d         %d        %d           %d",            squareToCoordinates[getMoveSource(move)],
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


int areSubStringsEqual(char *command, char *uciCommand, int stringSize) {
    if ((size_t)stringSize > strlen(command)) {
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




// read GUI/user input
void read_input(time* time) {
    // bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting()) {
        // tell engine to stop calculating
        time->stopped = 1;

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
                time->quit = 1;
            }

                // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4)) {
                // tell engine to terminate exacution
                time->quit = 1;
            }
        }
    }
}

void communicate(time* time) {
    // if time is up break here
    if (time->timeset == 1 && getTimeMiliSecond() > time->stoptime) {
        // tell engine to stop calculating
        time->stopped = 1;
    }
    read_input(time);
}


void uciProtocol(int argc, char *argv[]) {
    board *position = (board *)malloc(sizeof(board));

    position->ply = 0;

    time *time_ctrl = (time *)malloc(sizeof(time));

    //SearchStack *ss = (SearchStack *)malloc(sizeof(SearchStack));

    // init time control
    initTimeControl(time_ctrl);

    // max hash MB
    int max_hash = 32768;

    // default hash MB
    int default_hash_size = 64;

    // reset STDIN & STDOUT buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define user / GUI input buffer
    char input[2000];

    // print engine info
    printf("Potential by ProgramciDusunur\n");

    if (argc >= 2 && strncmp(argv[1], "bench", 5) == 0) {
        printf("bench running..");
        benchmark(8, position, time_ctrl);
        return;
    }

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
            parse_position(input, position);

            // clear hash table
            clearHashTable();

            //clear history
            clearQuietHistory();

            //clear static eval history
            clearStaticEvaluationHistory(position);

            //clear counter moves
            clearCounterMoves();
        }
            // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            // call parse position function
            parse_position("position startpos", position);

            // clear hash table
            clearHashTable();

            //clear history
            clearQuietHistory();

            //clear static eval history
            clearStaticEvaluationHistory(position);

            //clear counter moves
            clearCounterMoves();
        }
            // parse UCI "go" command
        else if (strncmp(input, "go", 2) == 0) {
            // call parse go function
            goCommand(input, position,  time_ctrl);

            // clear hash table
            clearHashTable();

            //clear history
            clearQuietHistory();

            //clear static eval history
            clearStaticEvaluationHistory(position);

            //clear counter moves
            clearCounterMoves();
        }
        else if (!strncmp(input, "setoption name Hash value ", 26)) {
            // init MB
            int mb;

            sscanf(input,"%*s %*s %*s %*s %d", &mb);

            // adjust MB if going beyond the aloowed bounds
            if(mb < 4) mb = 4;
            if(mb > max_hash) mb = max_hash;

            // set hash table size in MB
            printf("Set hash table size to %dMB\n", mb);
            init_hash_table(mb);
        }
            // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0) {
            // quit from the chess engine program executions
            // free SearchStack struct
            //free(ss);
            // free board struct
            free(position);
            // free time struct
            free(time_ctrl);
            break;
        }
            // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0)
        {
            // print engine info
            printf("id name Potential\n");
            printf("id author ProgramciDusunur\n");
            printf("option name Hash type spin default %d min 4 max %d\n",
                   default_hash_size, max_hash);
            printf("option name Threads type spin default %d min %d max %d\n", 1, 1,
                   1);
            printf("uciok\n");
        }
        else if (strncmp(input, "bench", 5) == 0) {
            benchmark(8, position, time_ctrl);
        }
    }
}


