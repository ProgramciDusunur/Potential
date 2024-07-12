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
