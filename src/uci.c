//
// Created by erena on 3.07.2024.
//

#include "uci.h"

/*void uciProtocol() {
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

            //clear history
            clearHistory();
        }
            // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            // call parse position function
            parse_position("position startpos", &position);

            // clear hash table
            clearHashTable();

            //clear history
            clearHistory();
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
}*/
