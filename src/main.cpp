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


#define NB_THREADS 4

enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

std::mutex g_mutex;

static struct {
	Path* shortest;
	Verbosity verbose;
	struct {
		int verified;	// # of paths checked
		int found;	// # of times a shorter path was found
		int* bound;	// # of bound operations per level
	} counter;
	int size;
	int total;		// number of paths to check
	int* fact;
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


int main(int argc, char* argv[])
{
	//freopen("output.txt","w",stdout);

	struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);


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
	g_fifo = LockFreeQueue();
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


void set(int* addr, int curr, int next)
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
	while (1)
	{
		// if we processed every possible path, stop the threads
		int cleared_paths = 0;
		//std::cout << id  <<" - " << "global size " << global.size << std::endl;
		for (int i=0; i<global.size; i++) 
		{
			int e = global.fact[i] * global.counter.bound[i];

			// std::cout << id  <<" - " << " e " << e << std::endl;
			cleared_paths += e;
		}

		// std::cout << id  <<" - " << "here2" << std::endl;

		// std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
		// std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
		// std::cout << id  <<" - " << "total : " << global.total << std::endl;
		// std::cout << id  <<" - " << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';


		if((global.counter.verified + cleared_paths) >= global.total)
		{
			std::cout << id  <<" - " << "exit" << std::endl;
			break;
		}

		// std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;

		// ! 1. Dequeue a job
		p = g_fifo.dequeue();

		// std::cout << id  <<" - " << "path : " << p << std::endl;

		if(p == nullptr)
		{
			//std::cout << id  <<" - " << COLOR.RED << "COULD NOT DEQUEUE" << COLOR.ORIGINAL  << std::endl;
			//temp++;
			if (temp > 10)
			{
				std::cout << id  << "EXITING BECAUSE I COULD NOT GET QUEUE" << std::endl;
				std::cout << id  <<" - " << "cleared_paths: " << cleared_paths << std::endl;
				std::cout << id  <<" - " << "verified : " << global.counter.verified << std::endl;
				std::cout << id  <<" - " << "total : " << global.total << std::endl;
				std::cout << id  <<" - " << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

				break;
			}
			continue;
		}
		

		// std::cout << id  <<" - " << "dequeue" << std::endl;

		// ? Check if the distance is already bigger than the min
		// std::cout << id  <<" - " << "p : " << p << std::endl;
		// std::cout << id  <<" - " << "current size : " << p->size() << std::endl;
		// std::cout << id  <<" - " << "current max : " << p->max() << std::endl;
		// std::cout << id  <<" - " << "current distance : " << p->distance() << std::endl;
		// std::cout << id  <<" - " << "shortest distance : " << global.shortest->distance() << std::endl;
		if (p->distance() > global.shortest->distance()) 
		{
			g_mutex.lock();
			global.counter.bound[p->size()]++;
			g_mutex.unlock();
			// set(&global.counter.bound[p->size()], global.counter.bound[p->size()], global.counter.bound[p->size()] + 1);
			continue; 
		}

		// std::cout << id  <<" - " << "distance" << std::endl;
		// ! 2. Check if we need to split the job
		// if (p->size() >= (p->max() - 8))
		// std::cout << id  <<" - " << "p.size = " << p->size() << std::endl;

		if (p->size() >= ((p->max() * 3) / 4))
		{
			// Do the job ourselves
			//if((count++ % 100) == 0) 
			//	std::cout << id  <<" - " << "branch_and_bound | " << count << std::endl;
			g_mutex.lock();
			branch_and_bound(p);
			g_mutex.unlock();
			// std::cout << id  <<" - " << "branch_and_bound done" << std::endl;
			// std::cout << id  <<" - " << "BAB" << std::endl;
		}
		else
		{ 
			// if((count++ % 10) == 0) 
			// 	std::cout << id  <<" - " << "split the job | " << count << std::endl;
		
			// Split the job
			for(int x = 0; x < g_graph->size(); x++)
			{
				new_p = new Path(g_graph);
				new_p->copy(p);
				if (!p->contains(x))
				{
					new_p->add(x);
					g_fifo.enqueue(new_p);
					//std::cout << id  <<" - " << "enqueing : " << x << std::endl;
				}
			}
		}
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
		// if (global.verbose & VER_COUNTERS)
		// g_mutex.lock();
		global.counter.verified++;
		// g_mutex.unlock();
		// set(&global.counter.verified, global.counter.verified, global.counter.verified + 1);

		// g_mutex.lock();
		if (current->distance() < global.shortest->distance()) 
		{
			if (global.verbose & VER_SHORTER)
				std::cout << "shorter: " << current << '\n';
			
			//g_mutex.lock();
			global.shortest->copy(current);
			global.counter.found++;
			//g_mutex.unlock();
			// if (global.verbose & VER_COUNTERS)
			// set(&global.counter.found, global.counter.found, global.counter.found + 1);
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

			// g_mutex.lock();
			global.counter.bound[current->size()]++;
			// g_mutex.unlock();
			//set(&global.counter.bound[current->size()], global.counter.bound[current->size()], global.counter.bound[current->size()] + 1);
		}

	}
}


void reset_counters(int size)
{
	global.size = size;
	global.counter.verified = 0;
	global.counter.found = 0;
	global.counter.bound = new int[global.size];
	global.fact = new int[global.size];
	for (int i=0; i<global.size; i++) {
		global.counter.bound[i] = 0;
		if (i) {
			int pos = global.size - i;
			global.fact[pos] = (i-1) ? (i * global.fact[pos+1]) : 1;
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
	for (int i=0; i<global.size; i++)
		std::cout << ' ' << global.counter.bound[i];
	std::cout << "\nbound equivalent (per level): ";
	int equiv = 0;
	for (int i=0; i<global.size; i++) {
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