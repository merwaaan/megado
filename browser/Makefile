# THE EMSCRIPTEN path must be defined ???

SOURCES = bit_utils.c\
instruction.c\
instructions_arithmetic.c\
instructions_bit.c\
instructions_control.c\
instructions_logic.c\
instructions_shift.c\
instructions_transfer.c\
m68k.c\
operands.c

OBJECTS = $(SOURCES:.c=.bc)

all: m68k
	@echo "JS build completed"

m68k: $(OBJECTS)
	emcc $(addprefix build/,$?) -o build/m68k.html -s EXPORTED_FUNCTIONS="['_bin_parse', '_m68k_make']"

%.bc : $(addprefix ../m68k/,%.c)
	emcc $<s -o $(addprefix build/,$@)

clean:
	rm build/*

#%.bc: %.cpp
#    $(CXX) $(OPTS) -I$(ACTIVE) $< -o $@
#box2d.bc: $(OBJECTS)
#    $(CXX) $(OPTS) -I$(ACTIVE) -o $@ $(OBJECTS)
#box2d_glue.cpp: $(ACTIVE).idl
#    $(PYTHON) "$(EMSCRIPTEN)/tools/webidl_binder.py" $(ACTIVE).idl box2d_glue
#box2d_glue.h: box2d_glue.cpp
#box2d.js: box2d.bc box2d_glue.cpp box2d_glue.h
#    $(CXX) $(LINK_OPTS) -I$(ACTIVE) -s EXPORT_BINDINGS=1 -s RESERVED_FUNCTION_POINTERS=20 --post-js box2d_glue.js --memory-init-file 0 -s NO_EXIT_RUNTIME=1 glue_stub.cpp $< -o build/$(ACTIVE)_$(BUILD).js
#clean: remove
#    rm -f $(OBJECTS)
#remove:
#    rm -f box2d.bc box2d_bindings.cpp box2d_bindings.bc box2d.clean.h box2d_glue.js box2d_glue.cpp WebIDLGrammar.pkl parser.out
