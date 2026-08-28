#include "structs.h"
#include <string.h>
#include <assert.h>
#include "board_constants.h"
#include "table.h"
#include "move.h"
#include "mask.h"


#define CUCKOO_TABLE_SIZE 8192

extern U64 cuckoo_keys[CUCKOO_TABLE_SIZE];
extern uint16_t cuckoo_moves[CUCKOO_TABLE_SIZE];

void cuckoo_init(void);
bool has_game_cycle(board *pos, int ply);
