# Comiler
CC = gcc

# Compiler flags
CFLAGS = -Iinclude -g

# Source files
SRCS = src/main.c src/repl.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output exectutable
TARGET = ./build/db

# Default rule
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile source code files into objcet files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run program
run: $(TARGET)
	$(TARGET)

# Debug
debug: $(TARGET) 
	gdb -tui $(TARGET)

# Unit Tests
.PHONY: test

test:
	rspec ./test/repl_spec.rb
