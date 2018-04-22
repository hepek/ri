CXXFLAGS=-std=c++17 -g -Wall

all: test
	./test

test: ri.h 

ri.h:
	touch test.cpp

clean:
	rm test

.PHONY: clean, run
