# Name of the executable
EXEC = lsc
BUILD = build
BIN = bin
USER_BIN = /usr/bin
TARGET = ../target

# Source and object files
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, $(TARGET)/$(BUILD)/%.o, $(SRCS))

# Include directory
INCLUDES = -I../includes

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
all: $(TARGET) $(TARGET)/$(BUILD) $(TARGET)/$(BIN) $(TARGET)/$(BIN)/$(EXEC)

# Bin dir
$(TARGET):
	mkdir -p $(TARGET)

# Build dir
$(TARGET)/$(BUILD):
	mkdir -p $(TARGET)/$(BUILD)

# Bin dir
$(TARGET)/$(BIN):
	mkdir -p $(TARGET)/$(BIN)

# Linking rule - create executable in bin directory
$(TARGET)/$(BIN)/$(EXEC): $(OBJS) | $(TARGET)/$(BIN)
	gcc $^ $(LDFLAGS) -o $@

# Pattern rule for C object files
$(TARGET)/$(BUILD)/%.o: src/%.c | $(TARGET)/$(BUILD)
	gcc $(CFLAGS) -c $< -o $@

# Make sure all object files go to build directory
.SUFFIXES:
.SUFFIXES: .c .o

# Clean up - also remove any stray object files in root directory
clean:
	rm -rf $(TARGET)

# Cleans and builds
rebuild: clean all

install: all
	sudo cp -f $(TARGET)/$(BIN)/$(EXEC) $(USER_BIN)/

uninstall: clean
	sudo rm -f $(USER_BIN)/$(EXEC)

reinstall: uninstall install

.PHONY: all clean rebuild install uninstall