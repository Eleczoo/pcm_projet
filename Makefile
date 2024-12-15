#  Copyright (c) 2012 Marcelo Pasin. All rights reserved.

CC=clang++
#CC=g++
# LDFLAGS= -O3 -lm -g
# CFLAGS= -O3 -Wall -g

# LDFLAGS= -O0 -Wall -Wno-format -lm -g
# CFLAGS= -O0 -Wall -Wno-format -g

CFLAGS= -O3 -Wall -fsanitize=address,alignment -Watomic-alignment -g
LDFLAGS= -O3 -lm -fsanitize=address,alignment -Watomic-alignment -g
#CFLAGS= -O3 -Wall -fsanitize=address,alignment  -g
#LDFLAGS= -O3 -lm -fsanitize=address,alignment  -g

#all: main fifo atomic 
all: fifo

main: build/main.o
	$(CC) -o build/main $(LDFLAGS) build/main.o -latomic -ldl

build/main.o: src/main.cpp src/graph.hpp src/path.hpp src/tspfile.hpp src/atomic.hpp src/fifo.hpp
	$(CC) $(CFLAGS) -c src/main.cpp -o $@ -latomic



fifo: build/fifo_test.o
	$(CC) -o build/fifo_test $(LDFLAGS) build/fifo_test.o -latomic -ldl

build/fifo_test.o: src/test_fifo.c src/graph.hpp src/path.hpp src/tspfile.hpp src/atomic.hpp src/fifo.hpp
	$(CC) $(CFLAGS) -c src/test_fifo.c -o $@ -latomic


omp:
	make main CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f build/*.o build/main build/atomic

run: all
	./build/main ./data/dj8.tsp
	
runv: all
	./build/main -v255 ./data/dj8.tsp