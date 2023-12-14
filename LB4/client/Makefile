CC = gcc
CFLAGS = -O2 -lm

SRCS = client.c ../linked-list.c ../hashfoo.c
OBJS = $(SRCS:.c=.o)
EXECUTABLE = client

.PHONY: build run clean

build: $(EXECUTABLE)

run: build
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)
