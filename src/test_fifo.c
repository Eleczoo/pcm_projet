// This tests the FIFO
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


#define NB_THREADS 60
#define MAX_PUSH 100000 // per thread


LockFreeQueue g_fifo;
void worker_routine(int id);
int main()
{
	std::cout << "Starting " << NB_THREADS << " threads\n";
	std::thread workers[NB_THREADS];
	for (int i = 0; i < NB_THREADS; i++)
		workers[i] = std::thread(worker_routine, i);

	for (int i = 0; i < NB_THREADS; i++)
		workers[i].join();
}

void worker_routine(int id)
{
	uint64_t count = 0;
	Path p;

	while (count < MAX_PUSH)
	{
		bool ret = g_fifo.enqueue(&p);
		if (ret)
		{
			if(count++ % 10000 == 0)
				std::cout << "Thread " << id << " pushed " << count << "\n";
			// std::cout << "Thread " << id << " pushed " << count << "\n";
			//count++;
		}
		else
		{
			std::cout << "Thread " << id << " failed to push " << count << "\n";
		}
	}
	std::cout << "Thread " << id << " finished\n";
}