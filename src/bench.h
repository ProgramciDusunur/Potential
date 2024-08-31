#pragma once

#include "bit_manipulation.h"
#include <stdio.h>
#include "search.h"
#include "time.h"
#include "fen.h"

static char* benchmarkfens[52];

static inline void benchmark(int depth, board* position, time* time);
