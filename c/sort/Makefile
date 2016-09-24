.PHONY: clean default

CFLAGS = -Wall -Wextra -std=c99 -pedantic-errors -g -o
CC = cc $(CFLAGS)

OBJECTS = quick merge heap bubble insert select bucket radix hybrid

default: $(OBJECTS)
quick: quick.c
	$(CC) quick quick.c
merge: merge.c
	$(CC) merge merge.c
heap: heap.c
	$(CC) heap heap.c
bubble: bubble.c
	$(CC) bubble bubble.c
insert: insert.c
	$(CC) insert insert.c
select: select.c
	$(CC) select select.c
bucket: bucket.c
	$(CC) bucket bucket.c
radix: radix.c
	$(CC) radix radix.c
hybrid: hybrid.c
	$(CC) hybrid hybrid.c
clean:
	rm -f $(OBJECTS)
