# Name of the executable
EXEC = lsc
BUILD = build

# Source and object files
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, $(BUILD)/%.o, $(SRCS))
ASM_SRCS = $(wildcard *.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD)/%.o, $(ASM_SRCS))

# Include directory
INCLUDES = -Iincludes

# Important: Define implementation flags for header-only libraries
IMPL_FLAGS = -DSKULL_LIST_H_IMPLEMENTATION -DSKULL_AST_H_IMPLEMENTATION \
 -DSKULL_TOKEN_H_IMPLEMENTATION -DSKULL_LEXER_H_IMPLEMENTATION \
 -DSKULL_PARSER_H_IMPLEMENTATION -DSKULL_TYPES_H_IMPLEMENTATION \
 -DSKULL_UTILS_H_IMPLEMENTATION -DSKULL_ASM_H_IMPLEMENTATION \
 -DSKULL_H_IMPLEMENTATION

# Compiler flags
CFLAGS = -g -Wall $(INCLUDES) $(IMPL_FLAGS)
LDFLAGS = -lm -ldl -fPIC -rdynamic

# Default target
all: $(BUILD) $(EXEC)

# Build dir
$(BUILD):
	mkdir -p $(BUILD)

# Linking rule - create executable in root directory
$(EXEC): $(OBJS) $(ASM_OBJS)
	gcc $^ $(LDFLAGS) -o $@

# Pattern rule for C object files
$(BUILD)/%.o: src/%.c | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

# Pattern rule for ASM object files
$(BUILD)/%.o: %.asm | $(BUILD)
	nasm -f elf64 $< -o $@

# Make sure all object files go to build directory
.SUFFIXES:
.SUFFIXES: .c .asm .o

# Clean up - also remove any stray object files in root directory
clean:
	rm -rf $(EXEC) $(BUILD) *.o

# Cleans and builds
rebuild: clean all

.PHONY: all clean redo