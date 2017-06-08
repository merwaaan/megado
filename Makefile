# Configurables
CC := clang
# Disable unused parameter warning since not everything is implemented yet
CFLAGS := -Wall -Wextra -pedantic -std=c11 -Wno-unused-parameter -Wno-unused-variable
RELEASE_FLAGS := -O3
DEBUG_FLAGS := -DDEBUG -g

BUILD_DIR := build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug

INCLUDES := -I./ -Ideps/cimgui/ -Ideps/glfw/include -Ideps/glew/include
LIBS := -Ldeps/glew/build/lib -lGLEW -lGLU -lGL -Ldeps/cimgui/cimgui -l:cimgui.so -Ldeps/glfw/build/src -lglfw

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
	@echo -e '#!/bin/sh\nenv LD_LIBRARY_PATH=deps/cimgui/cimgui:deps/glfw/build/src:deps/glew/build/lib $(RELEASE_DIR)/genesis "$$@"' > $(BUILD_DIR)/release.sh
	@chmod u+x $(BUILD_DIR)/release.sh

.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_DIR) $(DEBUG_DIR)/genesis
	@echo -e '#!/bin/sh\nenv LD_LIBRARY_PATH=deps/cimgui/cimgui:deps/glfw/build/src:deps/glew/build/lib $(DEBUG_DIR)/genesis "$$@"' > $(BUILD_DIR)/debug.sh
	@chmod u+x $(BUILD_DIR)/debug.sh

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
