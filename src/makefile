# Define make variables
_THIS       := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
_ROOT       := $(_THIS)
TARGET      := Potential
WARNINGS    = -Wall -Wextra -Wno-unused-variable
CFLAGS      := -funroll-loops -fomit-frame-pointer -Ofast -flto -fno-exceptions -DIS_64BIT -DNDEBUG $(WARNINGS)
NATIVE      = -march=native
AVX2FLAGS   = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi
BMI2FLAGS   = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mbmi2
AVX512FLAGS = -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw
NEONFLAGS   = -DUSE_NEON -flax-vector-conversions

NAME        := Potential
TMPDIR      := .tmp

# Detect Clang
ifeq ($(CC), clang)
	CFLAGS = -funroll-loops -Ofast -flto -fno-exceptions -DIS_64BIT -DNDEBUG $(WARNINGS)
endif

# Detect Windows
ifeq ($(OS), Windows_NT)
    # Force CC to gcc on Windows_NT systems using direct assignment
    CC      = gcc
    MKDIR   := mkdir
else
ifeq ($(COMP), MINGW)
    # Also force CC to gcc if MINGW detected using direct assignment
    CC      = gcc
    MKDIR   := mkdir
else
    # For other systems, use conditional assignment (allow override)
    CC      ?= cc
    MKDIR   := mkdir -p
endif
endif

# Detect Windows (Second Block - Retained as per original structure)
ifeq ($(OS), Windows_NT)
    uname_S  := Windows
    SUFFIX   := .exe
    CFLAGS  += -static
    FLAGS    ?= # Ensure FLAGS is defined for Windows, even if empty
else
    FLAGS    = -pthread -lm # Define FLAGS for non-Windows
    SUFFIX  :=
    uname_S := $(shell uname -s)
endif

# Different native flag for macOS
ifeq ($(uname_S), Darwin)
	NATIVE = -mcpu=apple-a14
	FLAGS  =
endif

ARCH_DETECTED =
PROPERTIES = $(shell echo | $(CC) -march=native -E -dM -)
ifneq ($(findstring __AVX512F__, $(PROPERTIES)),)
	ifneq ($(findstring __AVX512BW__, $(PROPERTIES)),)
		ARCH_DETECTED = AVX512
	endif
endif
ifeq ($(ARCH_DETECTED),)
	ifneq ($(findstring __BMI2__, $(PROPERTIES)),)
		ARCH_DETECTED = BMI2
	endif
endif
ifeq ($(ARCH_DETECTED),)
	ifneq ($(findstring __AVX2__, $(PROPERTIES)),)
		ARCH_DETECTED = AVX2
	endif
endif
ifeq ($(ARCH_DETECTED),)
	ifneq ($(findstring __aarch64__, $(PROPERTIES)),)
		ARCH_DETECTED = NEON
	endif
endif

# Remove native for builds
ifdef build
	NATIVE =
else
	ifeq ($(ARCH_DETECTED), AVX512)
		CFLAGS += $(AVX512FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), BMI2)
		CFLAGS += $(BMI2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), AVX2)
		CFLAGS += $(AVX2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), NEON)
		CFLAGS += $(NEONFLAGS)
	endif
endif

# SPECIFIC BUILDS
ifeq ($(build), native)
	NATIVE     = -march=native
	ARCH       = -x86-64-native
	ifeq ($(ARCH_DETECTED), AVX512)
		CFLAGS += $(AVX512FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), BMI2)
		CFLAGS += $(BMI2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), AVX2)
		CFLAGS += $(AVX2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), NEON)
		CFLAGS += $(NEONFLAGS)
	endif
endif

ifeq ($(build), x86-64)
	NATIVE       = -mtune=znver1
	INSTRUCTIONS = -msse -msse2 -mpopcnt
	ARCH         = -x86-64
endif

ifeq ($(build), x86-64-modern)
	NATIVE       = -mtune=znver2
	INSTRUCTIONS = -m64 -msse -msse3 -mpopcnt
	ARCH         = -x86-64-modern
endif

ifeq ($(build), x86-64-avx2)
	NATIVE    = -march=bdver4 -mno-tbm -mno-sse4a -mno-bmi2
	ARCH      = -x86-64-avx2
	CFLAGS   += $(AVX2FLAGS)
endif

ifeq ($(build), x86-64-bmi2)
	NATIVE    = -march=haswell
	ARCH      = -x86-64-bmi2
	CFLAGS   += $(BMI2FLAGS)
endif

ifeq ($(build), x86-64-avx512)
	NATIVE    = -march=x86-64-v4 -mtune=znver4
	ARCH      = -x86-64-avx512
	CFLAGS   += $(AVX512FLAGS)
endif

ifeq ($(build), debug)
	CFLAGS = -O3 -g3 -fno-omit-frame-pointer -std=gnu++2a
	NATIVE   = -msse -msse3 -mpopcnt
	FLAGS    = -lpthread -lstdc++
	ifeq ($(ARCH_DETECTED), AVX512)
		CFLAGS += $(AVX512FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), BMI2)
		CFLAGS += $(BMI2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), AVX2)
		CFLAGS += $(AVX2FLAGS)
	endif
	ifeq ($(ARCH_DETECTED), NEON)
		CFLAGS += $(NEONFLAGS)
	endif
endif

# Get what pgo flags we should be using
ifneq ($(findstring gcc, $(CC)),)
	PGOGEN   = -fprofile-generate
	PGOUSE   = -fprofile-use
endif


SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(TMPDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,$(TMPDIR)/%.d,$(SOURCES))

EXE	    := $(NAME)$(SUFFIX)

all: $(TARGET)
clean:
	@rm -rf $(TMPDIR) *.o *.d $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(NATIVE) -MMD -MP -o $(EXE) $(OBJECTS) $(FLAGS) $(LDFLAGS)

$(TMPDIR)/%.o: %.c | $(TMPDIR)
	$(CC) $(CFLAGS) $(NATIVE) -MMD -MP -c $< -o $@ $(FLAGS)

$(TMPDIR):
	$(MKDIR) "$(TMPDIR)" "$(TMPDIR)/Source"

# Usual disservin yoink for makefile related stuff
pgo:
	$(CC) $(CFLAGS) $(PGOGEN) $(NATIVE) $(INSTRUCTIONS) -MMD -MP -o $(EXE) $(SOURCES) $(LDFLAGS)
	./$(EXE) bench
	$(PGOMERGE)
	$(CC) $(CFLAGS) $(NATIVE) $(INSTRUCTIONS) $(PGOUSE) -MMD -MP -o $(EXE) $(SOURCES) $(LDFLAGS)
	@rm -f *.gcda *.profraw *.o $(DEPENDS) *.d