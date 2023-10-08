CC = gcc
CFLAGS = -O2

SRCS = main.c 
OBJS = $(SRCS:.c=.o)
EXECUTABLE = app

.PHONY: build run clean

build: $(EXECUTABLE)

run: build
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)
