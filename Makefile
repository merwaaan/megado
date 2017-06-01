# Configurables
CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c11
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -DDEBUG -g

RELEASE_DIR := build/release
DEBUG_DIR := build/debug

INCLUDES := -I./ `sdl2-config --cflags`
LIBS := `sdl2-config --libs`

MODULES := m68k genesis

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# There can be only one main, and that is test/main.c
SRC := $(filter-out m68k/main.c,$(foreach sdir,$(MODULES),$(wildcard $(sdir)/*.c)))
# Strip the module folder, and put all objects directly into the build dir
RELEASE_OBJS := $(patsubst %.c,$(RELEASE_DIR)/%.o,$(notdir $(SRC)))
DEBUG_OBJS := $(patsubst %.c,$(DEBUG_DIR)/%.o,$(notdir $(SRC)))

.PHONY: release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(RELEASE_DIR) $(RELEASE_DIR)/genesis

.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_DIR) $(DEBUG_DIR)/genesis

$(RELEASE_DIR)/genesis: test/main.c $(RELEASE_OBJS)
	$(CC) $^ $(CFLAGS) $(INCLUDES) $(LIBS) -o $@

$(DEBUG_DIR)/genesis: test/main.c $(DEBUG_OBJS)
	$(CC) $^ $(CFLAGS) $(INCLUDES) $(LIBS) -o $@

# Needed so the next rule finds the C files
# see https://stackoverflow.com/q/231229
VPATH = $(MODULES)

$(RELEASE_DIR)/%.o: %.c
	$(CC) $< -c $(CFLAGS) $(INCLUDES) -o $@

$(DEBUG_DIR)/%.o: %.c
	$(CC) $< -c $(CFLAGS) $(INCLUDES) -o $@

$(RELEASE_DIR):
	mkdir -p $@

$(DEBUG_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	rm --force $(RELEASE_OBJS)
	rm --force $(RELEASE_DIR)/genesis
	rm --force $(DEBUG_OBJS)
	rm --force $(DEBUG_DIR)/genesis
