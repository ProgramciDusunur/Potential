#ifndef POTENTIAL_DATAGEN_H
#define POTENTIAL_DATAGEN_H

#pragma once

#include "structs.h"
#include <stdio.h>
#include "generate_fen.h"
#include "search.h"
#include "timeman.h"
#include "threads.h"
#include "perft.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <inttypes.h>

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t);

#endif //POTENTIAL_DATAGEN_H
