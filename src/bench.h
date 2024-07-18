#pragma once

#include "bit_manipulation.h"
#include <stdio.h>
#include "search.h"
#include "time.h"
#include "fen.h"

extern char* benchmarkfens[52];

void benchmark(int depth, board* position);
