CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c11 -O3

BUILD_DIR := build
INCLUDES := -I./ `sdl2-config --cflags`
LIBS := `sdl2-config --libs`

MODULES := m68k genesis

# There can be only one main, and that is test/main.c
SRC := $(filter-out m68k/main.c,$(foreach sdir,$(MODULES),$(wildcard $(sdir)/*.c)))
# Strip the module folder, and put all objects directly into the build dir
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))

.PHONY: main
main: $(BUILD_DIR)/genesis

$(BUILD_DIR)/genesis: test/main.c $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -o $@ $^

# Needed so the next rule finds the C files
# see https://stackoverflow.com/q/231229
VPATH = $(MODULES)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	rm --force $(OBJS)
	rm --force $(BUILD_DIR)/genesis
