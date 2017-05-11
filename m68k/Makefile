CC := clang
CFLAGS := -Wall -Wextra -pedantic -std=c11 -O3

BUILD_DIR := ../build

SRC := $(filter-out main.c,$(wildcard *.c))
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

.PHONY: all
all: $(OBJS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm --force $(OBJS)
