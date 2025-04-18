# Name of the executable
EXEC = lsc

# Source and object files
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

# Include directory
INCLUDES = -Isrc/include

# Compiler flags
CFLAGS = -g $(INCLUDES)
LDFLAGS = -lm -ldl -fPIC -rdynamic

# Default target
all: $(EXEC)

# Linking rule
$(EXEC): $(OBJS)
	gcc $(OBJS) $(LDFLAGS) -o $@

# Pattern rule for object files
%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(EXEC) $(OBJS)

.PHONY: all clean