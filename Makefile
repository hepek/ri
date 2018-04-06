CXXFLAGS=-std=c++17

main: main.cpp

run: main
	./main

clean:
	rm main

.PHONY: clean, run
