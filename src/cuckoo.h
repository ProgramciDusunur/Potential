#ifndef POTENTIAL_CUCKOO_H
#define POTENTIAL_CUCKOO_H

#pragma once

#include "structs.h"
#include <stdbool.h>

void initCuckoo(void);
int hasUpcomingRepetition(board *pos);

#endif //POTENTIAL_CUCKOO_H
