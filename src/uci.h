//
// Created by erena on 3.07.2024.
//

#pragma once
#include "table.h"
#include <string.h>
#include <stdio.h>
#include "time.h"






//void uciProtocol();
void read_input();
int input_waiting();
void communicate();

int __cdecl read(int FileHandle,void *DstBuf,unsigned int MaxCharCount)  __MINGW_ATTRIB_DEPRECATED_MSVC2005;
