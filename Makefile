CXXFLAGS=-std=c++17 -g -Wall

main: main.cpp funiter.h

funiter.h:
	touch main.cpp

run: main
	./main

clean:
	rm main

.PHONY: clean, run
