#pragma once

#include "structs.h"
#include "board_constants.h"
#include "move.h"
#include "utils.h"


extern void default_fen_generation(board *pos, int current_ply);
extern FenString get_fen(board *pos);
