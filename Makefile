CXXFLAGS=-std=c++17 -g -Wall

all: main test
	./test

test: ri.h

main: main.cpp ri.h

ri.h:
	touch main.cpp


run: main
	./main

clean:
	rm main

.PHONY: clean, run
