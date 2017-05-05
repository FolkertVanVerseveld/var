.PHONY: default clean

default: longopt myopt
longopt: longopt.c
	gcc -Wall -Wextra -pedantic -std=gnu99 -g -DDEBUG -o longopt longopt.c
myopt: myopt.cpp
	g++ -Wall -Wextra -pedantic -std=c++11 -g -DDEBUG -o myopt myopt.cpp

clean:
	rm -f longopt myopt
