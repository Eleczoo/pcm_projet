#  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
CFLAGS=-O3 -Wall
LDFLAGS=-O3 -lm

all: tspcc atomic

tspcc: build/tspcc.o
	c++ -o build/tspcc $(LDFLAGS) build/tspcc.o

build/tspcc.o: src/tspcc.cpp src/graph.hpp src/path.hpp src/tspfile.hpp
	c++ $(CFLAGS) -c src/tspcc.cpp -o $@

atomic: src/atomic.cpp
	g++ -o build/atomic src/atomic.cpp -latomic

omp:
	make tspcc CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f build/*.o build/tspcc build/atomic

run:
	./build/tspcc ./data/dj17.tsp