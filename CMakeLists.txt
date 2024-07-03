cmake_minimum_required(VERSION 3.25)
project(Potential C)


# Optimizasyon bayrakları
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native -fwhole-program -ffast-math -fomit-frame-pointer")

include_directories(src)

add_executable(Potential
        src/bit_manipulation.c
        src/bit_manipulation.h
        src/board.c
        src/board.h
        src/board_constants.c
        src/board_constants.h
        src/evaluation.c
        src/evaluation.h
        src/fen.c
        src/fen.h
        src/magic.c
        src/magic.h
        src/mask.c
        src/mask.h
        src/move.c
        src/move.h
        src/potential.c
        src/search.c
        src/search.h
        src/see.c
        src/see.h
        src/table.c
        src/table.h
        src/time.c
        src/time.h
        src/values.c
        src/values.h src/test/see_test.c src/history.c src/history.h src/uci.c src/uci.h)
