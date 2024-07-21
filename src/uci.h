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
extern void read_input();
extern void communicate();
extern void goCommand(char *command, board* position);
