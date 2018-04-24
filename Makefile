CXXFLAGS=--std=c++17 -g -Wall
CXX=g++
LDFLAGS=-lstdc++

all: test
	./test

test: ri.h

test.cpp: ri.h

ri.h:
	touch test.cpp

clean:
	rm test

.PHONY: clean, run
