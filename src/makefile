# Compiler
CC = gcc
CFLAGS = -Ofast -march=native -fwhole-program -ffast-math -fomit-frame-pointer

# Source files
SRC = src/bit_manipulation.c \
      src/board.c \
      src/board_constants.c \
      src/evaluation.c \
      src/fen.c \
      src/magic.c \
      src/mask.c \
      src/move.c \
      src/potential.c \
      src/search.c \
      src/see.c \
      src/table.c \
      src/time.c \
      src/values.c \
      src/test/see_test.c \
      src/history.c \
      src/uci.c \
      src/bench.c

# Object files
OBJ = $(SRC:.c=.o)

# Output executable
EXE = Potential

# Default target
all: $(EXE)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link the executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32

# Clean up
clean:
	rm -f $(OBJ) $(EXE)

# Bench target
bench: $(EXE)
	./$(EXE) bench

.PHONY: all clean bench
