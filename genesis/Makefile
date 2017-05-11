CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c11 -O3

BUILD_DIR := ../build
INCLUDES := -I../ `sdl2-config --cflags`

SRC := $(wildcard *.c)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

.PHONY: all
all: $(OBJS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

.PHONY: clean
clean:
	rm --force $(OBJS)
