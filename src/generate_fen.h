#pragma once

#include "structs.h"
#include "board_constants.h"
#include "move.h"
#include "utils.h"


#include <stdio.h>
extern void default_fen_generation(board *pos, int current_ply, FILE *out_file);
extern FenString get_fen(board *pos);
