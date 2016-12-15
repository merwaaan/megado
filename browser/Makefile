# Compile the Genesis emulator to Javascript with Emscripten

MODULES := m68k genesis

SRC_DIR := $(addprefix ../, $(MODULES))
SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ := $(notdir $(SRC:.c=.bc))

BUILD_DIR := build/

# *.c prerequisites will be searched udner those paths
vpath %.c $(addprefix ../,$(MODULES))

EXPORTED_FUNCTIONS := genesis_make genesis_memory genesis_m68k
# f -> '_f'
EXPORTED_FUNCTIONS := $(addprefix '_, $(addsuffix ', $(EXPORTED_FUNCTIONS)))
# '_f1' '_f2' -> ['_f1','_f1']
comma:= ,
empty:=
space:= $(empty) $(empty)
EXPORTED_FUNCTIONS := [$(subst $(space),$(comma),$(EXPORTED_FUNCTIONS))]


all: $(BUILD_DIR) genesis.js
	@echo $(SRC)
	@echo $(OBJ)
	@echo "JS build completed"

# Create the build directory if necessary
$(BUILD_DIR):
	@mkdir -p $@

genesis.js: $(OBJ)
	emcc $(addprefix build/,$?) -o build/genesis.js -s EXPORTED_FUNCTIONS="$(EXPORTED_FUNCTIONS)" -s ALLOW_MEMORY_GROWTH=1

$(OBJ): %.bc : %.c
	emcc $< -I.. -o $(addprefix build/,$@)

clean:
	rm build/*
