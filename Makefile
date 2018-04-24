CXXFLAGS=--std=c++17 -g -Wall
CXX=g++
LDFLAGS=-lstdc++ -lstdc++fs

all: test
	./test

test: ri.h test.cpp
	$(CXX) $(CXXFLAGS) test.cpp $(LDFLAGS) -o test

test.cpp: ri.h

ri.h:
	touch test.cpp

clean:
	rm test

.PHONY: clean, run
