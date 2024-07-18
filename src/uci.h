//
// Created by erena on 3.07.2024.
//

#pragma once

#include "table.h"
#include <string.h>
#include <stdio.h>
#include "time.h"
#include "search.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif





//void uciProtocol();
void read_input();
int input_waiting();
void communicate();
void goCommand(char *command, board* position);
