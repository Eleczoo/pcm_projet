#  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
#CFLAGS=-O3 -Wall
CFLAGS= -O2 -Wall -g
#LDFLAGS=-O3 -lm
LDFLAGS= -O2 -lm -g

#all: main fifo atomic 
all: main

main: build/main.o build/death_handler.o
	c++ -o build/main $(LDFLAGS) build/main.o build/death_handler.o -latomic -ldl

#build/main.o: src/main.cpp src/graph.hpp src/path.hpp src/tspfile.hpp src/fifo.hpp src/atomic.hpp
build/main.o: src/death_handler.h src/main.cpp src/graph.hpp src/path.hpp src/tspfile.hpp src/atomic.hpp src/fifo.hpp
	c++ $(CFLAGS) -c src/main.cpp -o $@ -latomic

build/death_handler.o: src/death_handler.cc
	c++ $(CFLAGS) -c src/death_handler.cc -o $@

#atomic: src/atomic.cpp
#	g++ -o build/atomic src/atomic.cpp -latomic

#fifo: src/fifo.cpp
#	g++ -o build/fifo src/fifo.cpp

omp:
	make main CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f build/*.o build/main build/atomic

run: all
	./build/main ./data/dj8.tsp
	
runv: all
	./build/main -v255 ./data/dj8.tsp