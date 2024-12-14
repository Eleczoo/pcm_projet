// This tests the FIFO
#include "atomic.hpp"
#include "fifo.hpp"
#include "graph.hpp"
#include "path.hpp"
#include "tspfile.hpp"

#include <chrono>
#include <mutex>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#define NB_THREADS 20
#define MAX_PUSH   10000 // per thread

LockFreeQueue g_fifo;
void		  worker_routine(int id);

typedef uint32_t DATA;

// DATA p = 69;

DATA  p = 169;
DATA* p2;

int main()
{
	std::cout << "Starting " << NB_THREADS << " threads\n";
	std::thread workers[NB_THREADS];


	// for (int i = 0; i < MAX_PUSH * (NB_THREADS + 1); i++)
	//{
	//	g_fifo.enqueue(&p);
	// }

	for (int i = 0; i < NB_THREADS; i++)
		workers[i] = std::thread(worker_routine, i);

	for (int i = 0; i < NB_THREADS; i++)
		workers[i].join();
}

void worker_routine(int id)
{
	uint64_t count			  = 0;
	uint64_t counter_dequeued = 0;
	bool	 toggle			  = false;


	std::cout << "Thread " << id << " Started" << "\n";

	while (count < MAX_PUSH)
	{
		// if (!toggle)
		//{
		//  printf("[%d] enqueuing\n", id);
		bool ret = g_fifo.enqueue(&p);
		if (ret)
			count++;
		//	// printf("[%d] enqueued\n", id);
		//}
		// else // Dequeue
		//{
		//	printf("[%d] dequeuing\n", id);
		//	p2 = g_fifo.dequeue();
		//	// printf("[%d] ptr : %p\n", id, p2);
		//	if (p2 != nullptr)
		//	{
		//		// Dequeued properly
		//		// printf("[%d] Dequeued ok : %d (%d)\n", id, *p2, counter_dequeued);
		//		counter_dequeued++;
		//	}
		//	else
		//	{
		//		// printf("[%d] Couldnt dequeue\n", id);
		//	}
		//}

		// toggle = !toggle;
	}

	printf("[%d]Number of queued elements : %ld\n", id, count);
	// printf("[%d]Number of dequeued elements : %ld\n", id, counter_dequeued);
	// std::cout << "Thread " << id << " finished "
	//  << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
}
