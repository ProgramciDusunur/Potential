# Define make variables
_THIS       := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
_ROOT       := $(_THIS)
TARGET      := Potential
WARNINGS    = -Wall -Wextra -Wno-unused-variable
CFLAGS      := -O3 -flto -fno-exceptions -DIS_64BIT -DNDEBUG $(WARNINGS)
NATIVE      = -march=native

NAME        := Potential
TMPDIR      := .tmp

# Detect Clang
ifeq ($(CC), clang)
	CFLAGS = -O3 -flto -fno-exceptions -DIS_64BIT -DNDEBUG $(WARNINGS)
endif

# Detect Windows
ifeq ($(OS), Windows_NT)
	MKDIR   := mkdir
else
ifeq ($(COMP), MINGW)
	MKDIR   := mkdir
else
	MKDIR   := mkdir -p
endif
endif

# Detect Windows
ifeq ($(OS), Windows_NT)
	uname_S  := Windows
	SUFFIX   := .exe
	CFLAGS  += -static
else
	FLAGS    = -pthread -lm
	SUFFIX  :=
	uname_S := $(shell uname -s)
endif

# Different native flag for macOS
ifeq ($(uname_S), Darwin)
	NATIVE = -mcpu=apple-a14
	FLAGS  =
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