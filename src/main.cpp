#include <thread>
#include "graph.hpp"
#include "atomic.hpp"
#include "path.hpp"
#include "tspfile.hpp"
#include "fifo.hpp"

#define NB_THREADS 1


enum Verbosity {
	VER_NONE = 0,
	VER_GRAPH = 1,
	VER_SHORTER = 2,
	VER_BOUND = 4,
	VER_ANALYSE = 8,
	VER_COUNTERS = 16,
};

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
void worker_routine();
static void branch_and_bound(Path* current);
void reset_counters(int size);
void print_counters();

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
	//branch_and_bound(current);
	// current->add(1);

	// std::cout << "contains 0 : " << current->contains(0) << std::endl;
	// std::cout << "contains 1 : " << current->contains(1) << std::endl;
	// std::cout << "contains 2 : " << current->contains(2) << std::endl;

	// return 0 ;
	// ! Create the FIFO
	g_fifo = LockFreeQueue();
	g_fifo.enqueue(current);

	// ! WORKERS
	std::cout << "Starting " << NB_THREADS << " threads\n";
	std::thread workers[NB_THREADS];
	for (int i = 0; i < NB_THREADS; i++)
		workers[i] = std::thread(worker_routine);

	for (int i = 0; i < NB_THREADS; i++)
		workers[i].join();

	std::cout << COLOR.RED << "shortest " << global.shortest << COLOR.ORIGINAL << '\n';

	if (global.verbose & VER_COUNTERS)
		print_counters();

	return 0;
}


void worker_routine()
{
	// std::cout << "here" << std::endl;
	Path* p;
	Path* new_p;
	while(1)
	{
		// if we processed every possible path, stop the threads
		int cleared_paths = 0;
		//std::cout << "global size " << global.size << std::endl;
		for (int i=0; i<global.size; i++) 
		{
			int e = global.fact[i] * global.counter.bound[i];

			//std::cout << " e " << e << std::endl;
			cleared_paths += e;
		}

		// std::cout << "here2" << std::endl;

		std::cout << "cleared_paths: " << cleared_paths << std::endl;
		std::cout << "verified : " << global.counter.verified << std::endl;
		std::cout << "total : " << global.total << std::endl;

		if((global.counter.verified + cleared_paths) >= global.total)
		{
			std::cout << "exit" << std::endl;
			break;
		}

		// std::cout << "cleared_paths: " << cleared_paths << std::endl;

		// ! 1. Dequeue a job
		p = g_fifo.dequeue();

		// std::cout << "path : " << p << std::endl;

		if(p == nullptr)
		{
			// std::cout << "COULD NOT DEQUEUE" << std::endl;
			continue;
		}
		

		// std::cout << "dequeue" << std::endl;

		// ? Check if the distance is already bigger than the min
		// std::cout << "p : " << p << std::endl;
		std::cout << "current size : " << p->size() << std::endl;
		std::cout << "current max : " << p->max() << std::endl;
		std::cout << "current distance : " << p->distance() << std::endl;
		std::cout << "shortest distance : " << global.shortest->distance() << std::endl;
		if (p->distance() > global.shortest->distance()) 
		{
			continue; 
		}

		// std::cout << "distance" << std::endl;
		// ! 2. Check if we need to split the job
		if (p->size() >= (p->max() - 5))
		{
			// Do the job ourselves
			// std::cout << "branch_and_bound" << std::endl;
			branch_and_bound(p);
			// std::cout << "branch_and_bound done" << std::endl;
			// std::cout << "BAB" << std::endl;
		}
		else
		{
			// Split the job
			for(int x = 0; x < g_graph->size(); x++)
			{
				new_p = new Path(g_graph);
				new_p->copy(p);
				if (!p->contains(x))
				{
					new_p->add(x);
					g_fifo.enqueue(new_p);
					std::cout << "enqueing : " << x << std::endl;
				}
			}
		}
	}
}

static void branch_and_bound(Path* current)
{
	if (global.verbose & VER_ANALYSE)
		std::cout << "analysing " << current << '\n';

	if (current->leaf()) {
		// this is a leaf
		current->add(0);
		if (global.verbose & VER_COUNTERS)
			global.counter.verified++;
		if (current->distance() < global.shortest->distance()) {
			if (global.verbose & VER_SHORTER)
				std::cout << "shorter: " << current << '\n';
			global.shortest->copy(current);
			if (global.verbose & VER_COUNTERS)
				global.counter.found++;
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
			// current already >= shortest known so far, bound
			if (global.verbose & VER_BOUND )
				std::cout << "bound " << current << '\n';
			if (global.verbose & VER_COUNTERS)
				global.counter.bound[current->size()]++;
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
	std::cout << "total: " << global.total << '\n';
	std::cout << "verified: " << global.counter.verified << '\n';
	std::cout << "found shorter: " << global.counter.found << '\n';
	std::cout << "bound (per level):";
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