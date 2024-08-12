//
// Created by erena on 29.05.2024.
//
#pragma once

#include <stdio.h>
#include <stddef.h>

#ifdef WIN64

#include <windows.h>
#include <stdbool.h>

#else
#include <sys/time.h>
#endif

// exit from engine flag
static int quit = 0;

// UCI "movestogo" command moves counter
static int movestogo = 15;

// UCI "movetime" command time counter
static int movetime = -1;

// UCI "time" command holder (ms)
static int time = -1;

// UCI "inc" command's time increment holder
static int inc = 0;

// UCI "starttime" command time holder
static int starttime = 0;

// UCI "stoptime" command time holder
static int stoptime = 0;

// variable to flag time control availability
static int timeset = 0;

// variable to flag when the time is up
static int stopped = 0;

// reset time control variables
static inline void resetTimeControl() {
    // reset timing
    quit = 0;
    movestogo = 15;
    movetime = -1;
    time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

static inline int getTimeMiliSecond() {
#ifdef WIN64
    return GetTickCount();
#else
    struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}

static inline int input_waiting() {
#ifndef WIN32
    fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
#else
    int init = 0, pipe;
    HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
}
