# Compiler
CC = gcc
CFLAGS = -Ofast -march=native -fwhole-program -ffast-math -fomit-frame-pointer

# Source files
SRC = bit_manipulation.c \
      board.c \
      board_constants.c \
      evaluation.c \
      fen.c \
      magic.c \
      mask.c \
      move.c \
      potential.c \
      search.c \
      see.c \
      table.c \
      time.c \
      values.c \
      test/see_test.c \
      history.c \
      uci.c \
      bench.c

# Object files
OBJ = $(SRC:.c=.o)

# Output executable
EXE = Potential

# Default target
all: $(EXE)

# Compile object files
src/%.o: src/%.c
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

# Additional Targets and Flags for OpenBench compatibility
EXE_NAME = Potential

# Support for setting EXE output name via make EXE=...
ifeq ($(EXE),)
    EXE = $(EXE_NAME)
endif

# Flags
STD    = -std=gnu11
LIBS   = -pthread -lm
WARN   = -Wall -Wextra -Wshadow
FLAGS   = $(STD) $(WARN) -g -O3 -flto $(DEFS)
M64     = -m64 -mpopcnt
MSSE2   = $(M64) -msse -msse2
MSSSE3  = $(MSSE2) -mssse3
MAVX2   = $(MSSSE3) -msse4.1 -mbmi -mfma -mavx2
MAVX512 = $(MAVX2) -mavx512f -mavx512bw

# Detecting windows
ifeq ($(shell echo "test"), "test")
	FLAGS += -static
endif

# Detecting Apple Silicon (ARM64)
UNAME := $(shell uname -m)
ifeq ($(UNAME), arm64)
    ARCH = arm64
endif

# Setup arch
ifeq ($(ARCH), )
   ARCH = native
endif

ifeq ($(ARCH), native)
	CFLAGS += $(FLAGS) -march=native
else ifeq ($(ARCH), arm64)
	CFLAGS += $(FLAGS) -arch arm64
else ifeq ($(findstring x86-64, $(ARCH)), x86-64)
	CFLAGS += $(FLAGS) $(M64)
else ifeq ($(findstring sse2, $(ARCH)), sse2)
	CFLAGS += $(FLAGS) $(MSSE2)
else ifeq ($(findstring ssse3, $(ARCH)), ssse3)
	CFLAGS += $(FLAGS) $(MSSSE3)
else ifeq ($(findstring avx2, $(ARCH)), avx2)
	CFLAGS += $(FLAGS) $(MAVX2)
else ifeq ($(findstring avx512, $(ARCH)), avx512)
	CFLAGS += $(FLAGS) $(MAVX512)
endif

ifeq ($(ARCH), native)
	PROPS = $(shell echo | $(CC) -march=native -E -dM -)
	ifneq ($(findstring __BMI2__, $(PROPS)),)
		ifeq ($(findstring __znver1, $(PROPS)),)
			ifeq ($(findstring __znver2, $(PROPS)),)
				CFLAGS += -DUSE_PEXT
			endif
		endif
	endif
else ifeq ($(findstring -pext, $(ARCH)), -pext)
	CFLAGS += -DUSE_PEXT -mbmi2
endif

openbench:
	$(MAKE) ARCH=avx2 all

build:
	$(MAKE) ARCH=$(ARCH) all

pgo:
ifeq ($(findstring gcc, $(CC)), gcc)
	$(MAKE) ARCH=$(ARCH) PGOFLAGS="-fprofile-generate=pgo" all

	./$(EXE) bench 13 > pgo.out 2>&1
	grep Results pgo.out

	$(MAKE) ARCH=$(ARCH) PGOFLAGS="-fprofile-use=pgo" all

	@rm -rf pgo pgo.out
else ifeq ($(findstring clang, $(CC)), clang)
	$(MAKE) ARCH=$(ARCH) PGOFLAGS="-fprofile-instr-generate" all

	./$(EXE) bench 13 > pgo.out 2>&1
	grep Results pgo.out

	llvm-profdata merge -output=berserk.profdata *.profraw
	$(MAKE) ARCH=$(ARCH) PGOFLAGS="-fprofile-instr-use=berserk.profdata" all

	@rm -rf pgo pgo.out berserk.profdata *.profraw
else
	@echo "PGO builds not supported for $(CC)"
endif

.PHONY: all clean bench build openbench pgo
