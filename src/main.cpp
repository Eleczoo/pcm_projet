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
uint32_t limit_max_path;
volatile uint64_t g_total_verified = 0;



// ! PROTOTYPES
void worker_routine(int id);
static void branch_and_bound(Path* current);
void reset_counters(int size);
void print_counters();

int main(int argc, char* argv[])
{

	char* fname = 0;
	uint32_t nb_threads;
	if (argc == 2) 
	{
		fname = argv[1];
		global.verbose = VER_NONE;
		nb_threads = NB_THREADS;
		limit_max_path = LIMIT_MAX_PATH;
	} 
	else 
	{
		// ! HARDCODED NB THREAD AND LIMIT FOR FURTHER TESTS
		// ! ARGV[2] : NB THREADS
		// ! ARGV[3] : LIMIT MAX PATH
		if (argc == 4 )
		{
			fname = argv[1];
			nb_threads = atoi(argv[2]);
			limit_max_path = atoi(argv[3]);
			global.verbose = VER_NONE;

			if(nb_threads < 1 || limit_max_path < 1)
			{
				fprintf(stderr, "Cannot have less than 1 Thread or less than 1 max path limit");
				exit(1);
			}
		
		} 
		else
		{
			fprintf(stderr, "usage: %s {Data Path} {Number of threads} {Limit max paths}\n", argv[0]);
			exit(1);
		}
	}

	g_graph = TSPFile::graph(fname);
	if(g_graph->size() < 2)
	{
		fprintf(stderr, "Graph size is too small, need more cities\n");
		exit(1);
	}

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
	// g_fifo.enqueue(current);


	Path* new_p;
	for(int x = 0; x < g_graph->size(); x++)
	{
		new_p = new Path(g_graph);
		new_p->copy(current);
		if (!current->contains(x))
		{
			new_p->add(x);
			g_fifo.enqueue(new_p);
		}
	}


	// ! WORKERS
	std::thread workers[nb_threads];
	for (uint32_t i = 0; i < nb_threads; i++)
		workers[i] = std::thread(worker_routine, i);

	for (uint32_t i = 0; i < nb_threads; i++)
		workers[i].join();

	std::cout << "shortest " << global.shortest << '\n';
	
	return 0;
}


void atomic_set(uint32_t* addr, uint32_t curr, uint32_t next)
{
	while (!__atomic_compare_exchange(addr, &curr, &next, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}



void worker_routine(int id)
{
	Path* p;
	Path* new_p;

	while (1)
	{
		//  ! ----- STOP CONDITION ----- ! 
		if(g_total_verified >= global.total)
			break;

		p = g_fifo.dequeue();

		if(p == nullptr)
		{
			usleep(100);
			continue;
		}
		
		// ? Check if the distance is already bigger than the min
		if (p->distance() > global.shortest->distance()) 
		{
			__atomic_fetch_add(&g_total_verified, global.fact[p->size()], __ATOMIC_RELAXED);
			delete p;
			continue; 
		}

		// ! 2. Check if we need to split the job
		if (p->size() >= (p->max() - (int)limit_max_path))
		{
			// ? Limit reached, Actually do the job
			branch_and_bound(p);
		}
		else
		{
			// ? Limit not reached, split the job
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
		current->add(0);
		__atomic_fetch_add(&g_total_verified, 1, __ATOMIC_RELAXED);


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

			__atomic_fetch_add(&g_total_verified, global.fact[current->size()], __ATOMIC_RELAXED);
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
	std::cout << "total: " << global.total << '\n';
	std::cout << "verified: " << global.counter.verified << '\n';
	std::cout << "found shorter: " << global.counter.found << '\n';
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