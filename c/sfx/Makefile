.PHONY: default clean

libs=openal

CFLAGS=-Wall -Wextra -pedantic
LDFLAGS=$(shell pkg-config --cflags $(libs))
LDLIBS=$(shell pkg-config --libs $(libs))

default: stat
clean:
	rm -f stat
