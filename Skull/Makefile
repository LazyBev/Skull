# Name of the executable
EXEC = lsc

# Build directory
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

# Create build directory
$(BUILD):
	mkdir -p $(BUILD)

# Linking rule
$(EXEC): $(OBJS) $(ASM_OBJS)
	gcc $^ $(LDFLAGS) -o $(EXEC)

# Pattern rule for C object files
$(BUILD)/%.o: src/%.c | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

# Pattern rule for ASM object files (assuming nasm)
$(BUILD)/%.o: %.asm | $(BUILD)
	nasm -f elf64 $< -o $@

# Clean up
clean:
	rm -rf $(BUILD) $(EXEC)

.PHONY: all clean