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

#define NB_THREADS 6
#define MAX_PUSH   1000 // per thread

LockFreeQueue g_fifo;
void		  worker_routine(int id);

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
	Path	 p;

	std::cout << "Thread " << id << " Started" << "\n";

	while (count < MAX_PUSH)
	{
		bool ret = g_fifo.enqueue(&p);
		if (ret)
		{
			count++;
			// if (count++ % 10000 == 0)
			// std::cout << "Thread " << id << " pushed " << count << "\n";
			//  std::cout << "Thread " << id << " pushed " << count << "\n";
			//  count++;
		}
		else
		{
			// std::cout << "Thread " << id << " failed to push " << count << "\n";
		}
	}
	std::cout << "Thread " << id << " finished "
			  << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
}