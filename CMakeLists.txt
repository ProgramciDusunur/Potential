cmake_minimum_required(VERSION 3.25)
project(Potential C)

# General warning flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")

# Debug mode
set(CMAKE_C_FLAGS_DEBUG "-Ofast -g3 -DDEBUG")

# Release mode
set(CMAKE_C_FLAGS_RELEASE "-Ofast -march=native -ffast-math -fomit-frame-pointer -DNDEBUG")

# Source files
include_directories(src)

add_executable(Potential
        src/potential.c
        src/uci.c src/uci.h src/structs.h
        src/perft.c src/perft.h
        src/move.c src/move.h
        src/bit_manipulation.c src/bit_manipulation.h
        src/mask.c src/mask.h
        src/board_constants.c src/board_constants.h
        src/magic.c src/magic.h
        src/table.c src/table.h
        src/fen.c src/fen.h
        src/history.c src/history.h
        src/time.c src/time.h
        src/evaluation.c src/evaluation.h
        src/values.h
        src/search.c src/search.h
        src/bench.c src/bench.h
        src/utils.c src/utils.h
        src/see.c
        src/see.h)