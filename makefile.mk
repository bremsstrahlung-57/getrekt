CC      := gcc
CFLAGS  := -Wall -Wextra -g
LDFLAGS := -lsqlite3
SRCS    := main.c Server.c response.c todo.c db.c
OBJS    := $(SRCS:.c=.o)
TARGET  := main

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
