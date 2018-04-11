CXXFLAGS=-std=c++17 -O2

main: main.cpp funiter.h

funiter.h:
	touch main.cpp

run: main
	./main

clean:
	rm main

.PHONY: clean, run
