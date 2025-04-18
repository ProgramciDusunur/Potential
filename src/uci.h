//
// Created by erena on 3.07.2024.
//

#pragma once

#include "structs.h"

#include <string.h>
#include <stdio.h>
#include "move.h"
#include "time.h"
#include "search.h"
#include "fen.h"
#include "bench.h"


#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

void uciProtocol(int argc, char *argv[], board *position, time *time_ctrl);
void scaleTime(time* time, uint8_t bestMoveStability, uint8_t evalStability);
int parse_move(char *move_string, board* position);
void parse_position(char *command, board* position);
void goCommand(char *command, board* position, time* time);
void printMoveList(moves *moveList);
int areSubStringsEqual(char *command, char *uciCommand, int stringSize);
void read_input(time* time);
void communicate(time* time, board *pos);






