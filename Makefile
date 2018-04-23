CXXFLAGS=--std=c++17 -g -Wall
CXX=clang
LDFLAGS=-lstdc++

all: test
	./test

test:

test.cpp: ri.h

ri.h:
	touch test.cpp

clean:
	rm test

.PHONY: clean, run
