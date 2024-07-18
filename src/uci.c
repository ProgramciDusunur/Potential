//
// Created by erena on 3.07.2024.
//

#include "uci.h"





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
    searchPosition(depth, position, false);

}
