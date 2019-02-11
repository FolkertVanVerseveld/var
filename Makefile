.PHONY: default clean

CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu99 -DDEBUG
LDLIBS=-lgmp

BIN=test rsa

default: $(BIN)
test: test.c
rsa: rsa.c
run: rsa
	./rsa
	@#valgrind --leak-check=full ./rsa

clean:
	rm -f $(BIN) *.o
