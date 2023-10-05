CC = gcc
CFLAGS = -O2

SRCS = main.c utf.c
OBJS = $(SRCS:.c=.o)
EXECUTABLE = app

.PHONY: build run clean test

test: 
	diff --normal ./test/test1-original.txt ./test/test1-decrypted.txt
	diff --normal ./test/vigenere-test1.txt ./test/vigenere-decrypted.txt

build: $(EXECUTABLE)

run: build
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)
