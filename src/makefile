_THIS       := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
_ROOT       := $(_THIS)
TARGET      := Potential
WARNINGS    = -Wall -Wextra -Wno-unused-variable
CFLAGS      := -funroll-loops -fomit-frame-pointer -O3 -flto -fno-exceptions -DIS_64BIT -DNDEBUG $(WARNINGS)
NATIVE      = -march=native
uname_M := $(shell uname -m 2>/dev/null || echo unknown)
ifneq (,$(findstring aarch64,$(uname_M)))
    NATIVE = -mcpu=native
endif
ifneq (,$(findstring arm64,$(uname_M)))
    NATIVE = -mcpu=native
endif
AVX2FLAGS   = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi
BMI2FLAGS   = -DUSE_AVX2 -DUSE_SIMD -mavx2 -mbmi -mbmi2
AVX512FLAGS = -DUSE_AVX512 -DUSE_SIMD -mavx512f -mavx512bw
NEONFLAGS   = -DUSE_NEON -flax-vector-conversions
COMMIT_SHA := $(shell git rev-parse --short HEAD 2>/dev/null)

ifeq ($(COMMIT_SHA),)
    COMMIT_SHA := unknown
endif

CFLAGS += -DCOMMIT_SHA=$(COMMIT_SHA)
NAME        := Potential

NET_TAG  := ti-2
NET      := potential-384hl-ti-2.bin
NET_URL  := https://github.com/ProgramciDusunur/Potential-nets/releases/download/$(NET_TAG)/$(NET)
EVALFILE ?= $(NET)

CFLAGS      += -DEVALFILE=$(EVALFILE)
OBJDIR      := .tmp

CFLAGS += -funroll-loops -O3 -flto -fno-exceptions -DIS_64BIT -DNDEBUG

ifeq ($(OS), Windows_NT)
    uname_S  := Windows
    SUFFIX   := .exe
    MKDIR    := mkdir
    CFLAGS  += -static
    LDFLAGS += -Wl,--stack,16777216
    FLAGS    = -lpthread
else
    FLAGS    = -pthread -lm
    SUFFIX  :=
    uname_S := $(shell uname -s 2>/dev/null || echo unknown)
ifeq ($(COMP), MINGW)
    MKDIR   := mkdir
else
    MKDIR   := mkdir -p
endif
endif

ifeq ($(uname_S), Darwin)
	NATIVE = -mcpu=apple-a14
	FLAGS  =
endif

ifneq ($(uname_S), Darwin)
    LDFLAGS += -fuse-ld=lld
endif

ifneq ($(wildcard C:/msys64/mingw64/bin/clang.exe),)
    DEFAULT_CC := C:/msys64/mingw64/bin/clang.exe
else
    DEFAULT_CC := clang
endif

ifeq ($(origin CC),default)
    CLANG_CC ?= $(DEFAULT_CC)
else
    CLANG_CC ?= $(CC)
endif

CLANG_PATH    := $(shell command -v $(CLANG_CC) 2>/dev/null)
ifeq ($(CLANG_PATH),)
    CLANG_PATH := $(CLANG_CC)
endif
CLANG_DIR     := $(dir $(CLANG_PATH))
CLANG_VERSION := $(shell echo "__clang_major__" | $(CLANG_CC) -E -P -x c - 2>/dev/null | grep -E '^[0-9]+$$' | tail -n 1)
ifeq ($(CLANG_VERSION),)
    CLANG_VERSION := $(shell $(CLANG_CC) -dumpversion 2>/dev/null | cut -d. -f1)
endif

LLVM_PROFDATA ?= $(shell \
    if [ -n "$(CLANG_VERSION)" ] && [ -x "$(CLANG_DIR)llvm-profdata-$(CLANG_VERSION)" ]; then \
        echo "$(CLANG_DIR)llvm-profdata-$(CLANG_VERSION)"; \
    elif [ -x "$(CLANG_DIR)llvm-profdata" ]; then \
        echo "$(CLANG_DIR)llvm-profdata"; \
    elif [ -n "$(CLANG_VERSION)" ] && command -v llvm-profdata-$(CLANG_VERSION) >/dev/null 2>&1; then \
        command -v llvm-profdata-$(CLANG_VERSION); \
    elif command -v llvm-profdata >/dev/null 2>&1; then \
        command -v llvm-profdata; \
    elif [ "$(uname_S)" = "Darwin" ] && command -v xcrun >/dev/null 2>&1; then \
        echo "xcrun llvm-profdata"; \
    else \
        echo llvm-profdata; \
    fi)

ARCH_DETECTED =
PROPERTIES = $(shell echo | $(CLANG_CC) $(NATIVE) -E -dM -)
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

ifeq ($(build), haswell)
    NATIVE    = -march=haswell
    ARCH      = -x86-64-haswell
    CFLAGS   += $(BMI2FLAGS)
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

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,$(OBJDIR)/%.d,$(SOURCES))

EXE     := $(NAME)$(SUFFIX)

default: pgo

build_core: $(EVALFILE) $(TARGET)

clean:
	@rm -rf $(OBJDIR) *.o *.d $(TARGET) *.gcda

$(TARGET): $(OBJECTS)
	$(CLANG_CC) $(CFLAGS) $(NATIVE) -MMD -MP -o $(EXE) $(OBJECTS) $(FLAGS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CLANG_CC) $(CFLAGS) $(NATIVE) -MMD -MP -c $< -o $@ $(FLAGS)

$(OBJDIR)/nnue.o: $(EVALFILE)

$(OBJDIR):
	$(MKDIR) "$(OBJDIR)" "$(OBJDIR)/Source"

$(NET):
	@echo "Downloading $(NET)..."
	curl -sfL $(NET_URL) -o $(NET)
	@echo "Downloaded $(NET)."

net: $(NET)

pgo:
	@$(MAKE) clean
	@$(MAKE) build_core CFLAGS="$(CFLAGS) -fprofile-instr-generate"
	LLVM_PROFILE_FILE="$(OBJDIR)/default.profraw" ./$(EXE) bench
	$(LLVM_PROFDATA) merge -output=$(OBJDIR)/default.profdata $(OBJDIR)/default.profraw
	@rm -f $(OBJECTS) $(EXE)
	@$(MAKE) build_core CFLAGS="$(CFLAGS) -fprofile-instr-use=$(OBJDIR)/default.profdata"
	@rm -f $(OBJDIR)/default.profraw $(OBJDIR)/default.profdata

fast:
	@$(MAKE) build_core

release-avx2:
	@$(MAKE) pgo build=x86-64-avx2
	@mv $(EXE) $(NAME)-$(COMMIT_SHA)-zen2$(SUFFIX)
	@echo "Built: $(NAME)-$(COMMIT_SHA)-zen2$(SUFFIX)"

release-haswell:
	@$(MAKE) pgo build=haswell
	@mv $(EXE) $(NAME)-$(COMMIT_SHA)-avx2-pext$(SUFFIX)
	@echo "Built: $(NAME)-$(COMMIT_SHA)-avx2-pext$(SUFFIX)"

release-avx512:
	@$(MAKE) pgo build=x86-64-avx512
	@mv $(EXE) $(NAME)-$(COMMIT_SHA)-avx512$(SUFFIX)
	@echo "Built: $(NAME)-$(COMMIT_SHA)-avx512$(SUFFIX)"

release: release-avx2 release-haswell release-avx512
	@echo "All release binaries ready."

.PHONY: release release-avx2 release-haswell release-avx512 net
