CC = gcc

CFLAGS = -Iincluide -g

SRCS = ./btree.c

OBJS = $(SRCS:.c=.o)

TARGET = ./btree

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

debug: $(TARGET)
	gdb -tui $(TARGET)
