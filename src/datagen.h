#ifndef POTENTIAL_DATAGEN_H
#define POTENTIAL_DATAGEN_H

#pragma once

#include "structs.h"
#include <stdio.h>

int play_selfgen_game(FILE *out_file, FILE *illegal_file, int nodes_limit, int use_book, ThreadData *t);

#endif //POTENTIAL_DATAGEN_H
