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
#define LIMIT_MAX_PATH 10

enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

std::mutex g_mutex;

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

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	// Print colored error message
    std::cout << COLOR.RED << "Caught segfault at address " << si->si_addr << COLOR.ORIGINAL << std::endl;
    exit(0);
}

// ! https://github.com/vmarkovtsev/DeathHandler/tree/master


int main(int argc, char* argv[])
{
	//freopen("output.txt","w",stdout);

	// std::cout << "SIZE OF PATH " << sizeof(Path) << std::endl;

	// exit(0);

	struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);

	//Debug::DeathHandler dh;

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
	// branch_and_bound(current);
	// current->add(1);

	// std::cout << "contains 0 : " << current->contains(0) << std::endl;
	// std::cout << "contains 1 : " << current->contains(1) << std::endl;
	// std::cout << "contains 2 : " << current->contains(2) << std::endl;

	// return 0 ;
	// ! Create the FIFO
	//g_fifo = LockFreeQueue();
	// Path* new_p;
	// for(int x = 0; x < g_graph->size(); x++)
	// {
	// 	new_p = new Path(g_graph);
	// 	new_p->copy(current);
	// 	if (!current->contains(x))
	// 	{
	// 		new_p->add(x);
	// 		g_fifo.enqueue(new_p);
	// 	}
	// }
	g_fifo.enqueue(current);

	// Path* p;
	// p = g_fifo.dequeue();
	// std::cout << "main" <<" - " << "p : " << p << std::endl;
	// std::cout << "main" <<" - " << "current size : " << p->size() << std::endl;
	// std::cout << "main" <<" - " << "current max : " << p->max() << std::endl;
	// std::cout << "main" <<" - " << "current distance : " << p->distance() << std::endl;
	// std::cout << "main" <<" - " << "shortest distance : " << global.shortest->distance() << std::endl;



	// ! WORKERS
	std::cout << "Starting " << NB_THREADS << " threads\n";
	std::thread workers[NB_THREADS];
	for (int i = 0; i < NB_THREADS; i++)
		workers[i] = std::thread(worker_routine, i);

	for (int i = 0; i < NB_THREADS; i++)
		workers[i].join();

	std::cout << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

	if (global.verbose & VER_COUNTERS)
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
	int temp = 0; // Remove this
	//uint64_t count = 0; 
	// return;
	// g_mutex.lock();

	//std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
	std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
	std::cout << id  <<" - " << "total : " << global.total << std::endl;
	std::cout << id  <<" - " << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

	while (1)
	{

		// if we processed every possible path, stop the threads
		uint64_t cleared_paths = 0;
		//std::cout << id  <<" - " << "global size " << global.size << std::endl;
		// g_mutex.lock();
		for (uint64_t i=0; i<global.size; i++) 
		{
			uint64_t e = global.fact[i] * global.counter.bound[i];

			// std::cout << id  <<" - " << " e " << e << std::endl;
			cleared_paths += e;
		}
		// g_mutex.unlock();

		// std::cout << id  <<" - " << "here2" << std::endl;

		// std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
		// std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
		// std::cout << id  <<" - " << "total : " << global.total << std::endl;
		// std::cout << id  <<" - " << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';


		if((global.counter.verified + cleared_paths) >= global.total)
		{
			std::cout << id  <<" - " << "exit" << std::endl;
			// g_mutex.unlock();

			std::cout << id  << "EXITING NORMALY" << std::endl;
			std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
			std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
			std::cout << id  <<" - " << "total : " << global.total << std::endl;
			break;
		}

		// std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;

		// ! 1. Dequeue a job
		p = g_fifo.dequeue();

		// std::cout << id  <<" - " << "path : " << p << std::endl;

		if(p == nullptr)
		{
			// std::cout << id  <<" - " << COLOR.RED << "COULD NOT DEQUEUE" << COLOR.ORIGINAL  << std::endl;
			temp++;
			if (temp > 100000)
			{
				std::cout << id  << " - EXITING BECAUSE I COULD NOT GET QUEUE" << std::endl;
				std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
				std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
				std::cout << id  <<" - " << "total : " << global.total << std::endl;
				// std::cout << id  <<" - " << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';
				// g_mutex.unlock();
				break;
			}
			continue;
		// g_mutex.unlock();
		}
		

		// std::cout << id  <<" - " << "dequeue success" << std::endl;

		// ? Check if the distance is already bigger than the min
		// std::cout << id  <<" - " << "p : " << p << std::endl;
		// std::cout << id  <<" - " << "current size : " << p->size() << std::endl;
		// std::cout << id  <<" - " << "current max : " << p->max() << std::endl;
		// std::cout << id  <<" - " << "current distance : " << p->distance() << std::endl;
		// std::cout << id  <<" - " << "shortest distance : " << global.shortest->distance() << std::endl;
		if (p->distance() > global.shortest->distance()) 
		{
			// g_mutex.lock();
			// global.counter.bound[p->size()]++;
			// g_mutex.unlock();
			__atomic_fetch_add(&global.counter.bound[p->size()], 1, __ATOMIC_RELAXED);

			continue; 
		}

		// std::cout << id  <<" - " << "distance" << std::endl;

		// ! 2. Check if we need to split the job
		// if (p->size() >= (p->max() - 8))
		// std::cout << id  <<" - " << "p.size = " << p->size() << std::endl;

		//if (p->size() >= ((p->max() * 3) / 4))


		//if (p->size() >= (p->max() - 8))
		if (p->size() >= (p->max() - LIMIT_MAX_PATH))
		{
			// std::cout << "Created enough paths" << std::endl;
			// Do the job ourselves
			//if((count++ % 100) == 0) 
			// std::cout << id  <<" - " << "branch_and_bound" << std::endl;
			//g_mutex.lock();
			// std::cout << "before branch_and_bound() - Size : " << p->size() << std::endl;
			branch_and_bound(p);
			//g_mutex.unlock();
			// std::cout << id  <<" - " << "branch_and_bound done" << std::endl;
			// std::cout << id  <<" - " << "BAB" << std::endl;
		}
		else
		{
			// std::cout << "worker_routine() - (p->max() - 5) : " << (p->max() - 5) << std::endl;
			// std::cout << "worker_routine() - Size : " << p->size() << std::endl;

			// if((count++ % 10) == 0) 
				// std::cout << id  <<" - " << "split the job" << std::endl;
		
			// Split the job
			// std::cout << id << " - " << "--- PATH : "<< p << std::endl;
			for(int x = 0; x < g_graph->size(); x++)
			{
				if(!p->contains(x))
				{
					// std::cout << id  <<" - " << "adding : " << x << std::endl;
					new_p = new Path(g_graph);
					new_p->copy(p);

					new_p->add(x);

					g_fifo.enqueue(new_p);
					
					// std::cout << id  <<" - " << "enqueing : " << x << std::endl;
				}
				else
				{
					// std::cout << id  <<" - " << "already contains : " << x << std::endl;
				}
			}
		}
	}
	// g_mutex.unlock();
}

static void branch_and_bound(Path* current)
{
	if (global.verbose & VER_ANALYSE)
		std::cout << "analysing " << current << '\n';

	if (current->leaf()) 
	{
		// this is a leaf
		current->add(0);
		// if (global.verbose & VER_COUNTERS)
		// g_mutex.lock();
		// global.counter.verified++;
		// g_mutex.unlock();
		__atomic_fetch_add(&global.counter.verified, 1, __ATOMIC_RELAXED);


		// g_mutex.lock();
		if (current->distance() < global.shortest->distance()) 
		{
			if (global.verbose & VER_SHORTER)
				std::cout << "shorter: " << current << '\n';
			
			// g_mutex.lock();
			std::cout << "new shortest: " << current->distance() << '\n';
			std::cout << "old shortest: " << global.shortest->distance() << '\n';
			global.shortest->copy(current);
			// global.counter.found++;
			// g_mutex.unlock();
			// if (global.verbose & VER_COUNTERS)
			__atomic_fetch_add(&global.counter.found, 1, __ATOMIC_RELAXED);
		}
		// g_mutex.unlock();
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
			// current already >= shortest known so far, bound
			if (global.verbose & VER_BOUND )
				std::cout << "bound " << current << '\n';
			// if (global.verbose & VER_COUNTERS)

			// std::cout << " - bound at size : " << current->size() << std::endl;
			// g_mutex.lock();
			// global.counter.bound[current->size()]++;
			// g_mutex.unlock();
			__atomic_fetch_add(&global.counter.bound[current->size()], 1, __ATOMIC_RELAXED);
			// set(&global.counter.bound[current->size()], global.counter.bound[current->size()], global.counter.bound[current->size()] + 1);
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
//	std::cout << "bound (per level):";
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





// ! GRAVEYARD

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