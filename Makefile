# Name of the executable
EXEC = lsc

# Build directory
BUILD = build

# Source and object files
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, $(BUILD)/%.o, $(SRCS))
ASM_SRCS = $(wildcard *.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD)/%.o, $(ASM_SRCS))
BUILD_OBJS = $(wildcard $(BUILD)/*)

# Include directory
INCLUDES = -Iincludes

# Compiler flags
CFLAGS = -g -Wall $(INCLUDES)
LDFLAGS = -lm -ldl -fPIC -rdynamic

# Default target
all: $(BUILD) $(EXEC)

# Create build directory
$(BUILD):
	mkdir -p $(BUILD)

# Linking rule
$(EXEC): $(OBJS) $(ASM_OBJS)
	gcc $^ $(LDFLAGS) -o $(BUILD)/$(EXEC)

# Pattern rule for C object files
$(BUILD)/%.o: src/%.c | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

# Pattern rule for ASM object files (assuming nasm)
$(BUILD)/%.o: %.asm | $(BUILD)
	nasm -f elf64 $< -o $@

# Clean up
clean:
	rm -rf $(BUILD_OBJS) $(EXEC) $(ASM_OBJS) $(ASM_SRCS)

.PHONY: all clean