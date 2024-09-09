//
// Created by erena on 3.07.2024.
//

#pragma once

#include "time.h"


#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif





//void uciProtocol();
extern void communicate(time* time);
