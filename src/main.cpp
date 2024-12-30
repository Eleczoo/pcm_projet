#include <thread>
#include "graph.hpp"
#include "atomic.hpp"
#include "path.hpp"
#include "tspfile.hpp"
#include "fifo.hpp"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
//#include "death_handler.h"


#define NB_THREADS 6
#define LIMIT_MAX_PATH 8

enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

std::mutex g_mutex;
uint64_t count_non_nul = 0;
uint64_t count_enqueue = 0;

volatile static struct {
	Path* shortest;
	Verbosity verbose;
	struct {
		uint64_t verified;	// # of paths checked
		uint64_t found;	// # of times a shorter path was found
		uint64_t* bound;	// # of bound operations per level
	} counter;
	uint64_t size;
	uint64_t total;		// number of paths to check
	uint64_t* fact;
} global;


static const struct {
	char RED[6];
	char BLUE[6];
	char ORIGINAL[6];
} COLOR = {
	.RED = { 27, '[', '3', '1', 'm', 0 },
	.BLUE = { 27, '[', '3', '6', 'm', 0 },
	.ORIGINAL = { 27, '[', '3', '9', 'm', 0 },
};

LockFreeQueue g_fifo;
Graph* g_graph;

// ! PROTOTYPES
void worker_routine(int id);
static void branch_and_bound(Path* current);
void reset_counters(int size);
void print_counters();

//void segfault_sigaction(int signal, siginfo_t *si, void *arg)
//{
//	// Print colored error message
//    std::cout << COLOR.RED << "Caught segfault at address " << si->si_addr << COLOR.ORIGINAL << std::endl;
//    exit(0);
//}

// ! https://github.com/vmarkovtsev/DeathHandler/tree/master


int main(int argc, char* argv[])
{

	char* fname = 0;
	if (argc == 2) 
	{
		fname = argv[1];
		global.verbose = VER_NONE;
	} 
	else 
	{
		if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'v')
		{
			global.verbose = (Verbosity) (argv[1][2] ? atoi(argv[1]+2) : 1);
			fname = argv[2];
		} 
		else
		{
			fprintf(stderr, "usage: %s [-v#] filename\n", argv[0]);
			exit(1);
		}
	}

	g_graph = TSPFile::graph(fname);
	if (global.verbose & VER_GRAPH)
		std::cout << COLOR.BLUE << g_graph << COLOR.ORIGINAL;

	// if (global.verbose & VER_COUNTERS)
	reset_counters(g_graph->size());

	global.shortest = new Path(g_graph);
	for (int i=0; i<g_graph->size(); i++) 
	{
		global.shortest->add(i);
	}
	global.shortest->add(0);

	Path* current = new Path(g_graph);
	current->add(0);

	g_fifo.enqueue(current);

	// ! WORKERS
	std::cout << "Starting " << NB_THREADS << " threads\n";
	std::thread workers[NB_THREADS];
	for (int i = 0; i < NB_THREADS; i++)
		workers[i] = std::thread(worker_routine, i);

	for (int i = 0; i < NB_THREADS; i++)
		workers[i].join();

	std::cout << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

	// if (global.verbose & VER_COUNTERS)
	print_counters();

	return 0;
}


void set(volatile uint64_t* addr, uint64_t curr, uint64_t next)
{
	while (!__atomic_compare_exchange(addr, &curr, &next, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}



void worker_routine(int id)
{
	// Print timestamp in hh:mm:ss::ms
	std::cout << id  <<" - " << "STARTED WORKER " << id << std::endl;
	Path* p;
	Path* new_p;

	while (1)
	{

		// if we processed every possible path, stop the threads
		uint64_t cleared_paths = 0;
		// g_mutex.lock();

		//auto start = std::chrono::system_clock::now();
		for (uint64_t i=0; i<global.size; i++) 
		{
			uint64_t e = global.fact[i] * global.counter.bound[i];

			cleared_paths += e;
		}
		//auto end = std::chrono::system_clock::now();

		//printf("Time taken by factorial (ns) : %lld\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

		//  ! ----- STOP CONDITION ----- ! 
		if((global.counter.verified + cleared_paths) >= global.total)
			break;


		// ! 1. Dequeue a job
		p = g_fifo.dequeue();


		if(p == nullptr)
		{
			usleep(50);
			continue;
		}
		


		//auto start = std::chrono::system_clock::now();
		// ? Check if the distance is already bigger than the min
		if (p->distance() > global.shortest->distance()) 
		{
			__atomic_fetch_add(&global.counter.bound[p->size()], 1, __ATOMIC_RELAXED);
			delete p;
			continue; 
		}
		//auto end = std::chrono::system_clock::now();

		//printf("Time taken by bound (ns) : %lld\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());


		// ! 2. Check if we need to split the job
		if (p->size() >= (p->max() - LIMIT_MAX_PATH))
		{
			branch_and_bound(p);
		}
		else
		{
			for(int x = 0; x < g_graph->size(); x++)
			{
				if(!p->contains(x))
				{
					new_p = new Path(g_graph);
					new_p->copy(p);

					new_p->add(x);

					g_fifo.enqueue(new_p);
				}
			}
		}
		delete p;
	}
}

static void branch_and_bound(Path* current)
{
	if (global.verbose & VER_ANALYSE)
		std::cout << "analysing " << current << '\n';

	if (current->leaf()) 
	{
		// this is a leaf
		current->add(0);
		__atomic_fetch_add(&global.counter.verified, 1, __ATOMIC_RELAXED);


		if (current->distance() < global.shortest->distance()) 
		{
			if (global.verbose & VER_SHORTER)
				std::cout << "shorter: " << current << '\n';
			
			global.shortest->copy(current);
			__atomic_fetch_add(&global.counter.found, 1, __ATOMIC_RELAXED);
		}
		current->pop();
	}
	else 
	{
		// not yet a leaf
		if (current->distance() < global.shortest->distance()) 
		{
			// continue branching
			for (int i=1; i<current->max(); i++) 
			{
				if (!current->contains(i)) 
				{
					current->add(i);
					branch_and_bound(current);
					current->pop();
				}
			}
		} 
		else 
		{
			if (global.verbose & VER_BOUND )
				std::cout << "bound " << current << '\n';
			__atomic_fetch_add(&global.counter.bound[current->size()], 1, __ATOMIC_RELAXED);
		}

	}
}


void reset_counters(int size)
{
	global.size = size;
	global.counter.verified = 0;
	global.counter.found = 0;
	global.counter.bound = new uint64_t[global.size];
	global.fact = new uint64_t[global.size];
	for (uint64_t i=0; i<global.size; i++) {
		global.counter.bound[i] = 0;
		if (i) {
			int pos = global.size - i;
			//global.fact[pos] = (i-1) ? (i * global.fact[pos+1]) : 1;

			if((i - 1) != 0)
				global.fact[pos] = i * global.fact[pos+1];
			else
				global.fact[pos] = 1;
		}
	}
	global.total = global.fact[0] = global.fact[1];
}

void print_counters()
{
//	std::cout << "total: " << global.total << '\n';
//	std::cout << "verified: " << global.counter.verified << '\n';
//	std::cout << "found shorter: " << global.counter.found << '\n';
	std::cout << "bound (per level):";
	for (uint64_t i=0; i<global.size; i++)
		std::cout << ' ' << global.counter.bound[i];
	std::cout << "\nbound equivalent (per level): ";
	int equiv = 0;
	for (uint64_t i=0; i<global.size; i++) {
		int e = global.fact[i] * global.counter.bound[i];
		std::cout << ' ' << e;
		equiv += e;
	}
	std::cout << "\nbound equivalent (total): " << equiv << '\n';
	std::cout << "check: total " << (global.total==(global.counter.verified + equiv) ? "==" : "!=") << " verified + total bound equivalent\n";
}



// ! GRAVEYARD

//int main()
//{
//// 	DATA data = 12;
//// 	DATA data2 = 69;

// 	LockFreeQueue fifo = LockFreeQueue();

//	// std::cout << "-----" << std::endl;
//	// fifo.show_queue();
//	// std::cout << "-----" << std::endl;

//	// fifo.enqueue(&data);
//	// fifo.show_queue();
//	// std::cout << "-----" << std::endl;


//	DATA data[20];
//	for (int x = 0; x < 20; x++)
//	{
//		data[x] = x;
//		fifo.enqueue(&data[x]);
//	}
//	std::cout << "Done queuing" << std::endl;

//	// std::cout << "deq : " << *deq << std::endl;

//	DATA* deq;
//	for (int x = 0; x < 25; x++)
//	{
//		deq = fifo.dequeue();
//		if (deq)
//		{
//			std::cout << "deq : " << *deq << std::endl;
//		}
//		else
//		{
//			std::cout << "fifo empty" << std::endl;
//		}
//	}
//}






// int main()
// {
// 	// DATA data = 12;
// 	// DATA data2 = 69;

// 	LockFreeQueue fifo = LockFreeQueue();

// 	// std::cout << "-----" << std::endl;
// 	// fifo.show_queue();
// 	// std::cout << "-----" << std::endl;

// 	// fifo.enqueue(&data);
// 	// fifo.show_queue();
// 	// std::cout << "-----" << std::endl;

// 	// fifo.enqueue(&data2);
// 	// fifo.show_queue();
// 	// std::cout << "-----" << std::endl;

// 	DATA data[20];
// 	for (int x = 0; x < 20; x++)
// 	{
// 		data[x] = x;
// 		fifo.enqueue(&data[x]);
// 	}

// 	fifo.show_queue();
	
// 	// std::cout << "show_queue" << std::endl;
// }