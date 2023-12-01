CC = gcc
CFLAGS = -g
LDFLAGS = -lzip -lxml2
PATHFLAGS = -I/usr/include/libxml2

SRCS = main.c 
OBJS = $(SRCS:.c=.o)
EXECUTABLE = app

.PHONY: build run clean

build: $(EXECUTABLE)

run: build
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(PATHFLAGS)

clean:
	rm -f $(OBJS) $(EXECUTABLE)
