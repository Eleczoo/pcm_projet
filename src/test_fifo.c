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


#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define NB_THREADS_CONSUMER 5
#define NB_THREADS_PRODUCER 5
#define MAX_PUSH   1500 // per thread

LockFreeQueue g_fifo;
void		  producer_routine(int id);
void		  consumer_routine(int id);

std::mutex g_mutex;

typedef uint32_t DATA;

uint32_t g_dq_count = 0;

DATA p = 69;


int main()
{
	printf("Starting %d producer threads and %d consumer threads\n", NB_THREADS_PRODUCER,
		   NB_THREADS_CONSUMER);
	std::thread workers_producer[NB_THREADS_PRODUCER];
	std::thread workers_consumer[NB_THREADS_CONSUMER];


	for (int i = 0; i < NB_THREADS_PRODUCER; i++)
	{
		workers_producer[i] = std::thread(producer_routine, i);
	}

	// for (int i = 0; i < NB_THREADS_PRODUCER; i++)
	// 	workers_producer[i].join();


	for (int i = 0; i < NB_THREADS_CONSUMER; i++)
	{
		workers_consumer[i] = std::thread(consumer_routine, i);
	}


	for (int i = 0; i < NB_THREADS_PRODUCER; i++)
		workers_producer[i].join();

	for (int i = 0; i < NB_THREADS_CONSUMER; i++)
		workers_consumer[i].join();


	// g_fifo.show_queue();
}

void producer_routine(int id)
{
	printf("Producer thread %d started\n", id);

	for (uint32_t i = 0; i < MAX_PUSH; i++)
	{
		g_fifo.enqueue(&p);
	}

	// if (id == 2) {g_fifo.show_queue();}

	printf(COLOR_RED "Producer thread %d finished\n" COLOR_RESET, id);
}

void consumer_routine(int id)
{
	printf("Consumer thread %d started\n", id);

	while (g_dq_count < (MAX_PUSH * NB_THREADS_PRODUCER))
	{
		if (g_fifo.dequeue() != nullptr)
		{
			 __atomic_fetch_add(&g_dq_count, 1, __ATOMIC_RELAXED);
			// printf("[%d] g_dq_count = %d\n", id, g_dq_count);
			// g_mutex.lock();
			// g_dq_count++;
			// g_mutex.unlock();
		}
		else
		{
			// printf("[%d] Failed to dequeue g_dq_count = %d\n", id, g_dq_count);
			// if (id == 0) {g_fifo.show_queue();}
		}
	}

	printf(COLOR_GREEN "Consumer thread %d finished\n" COLOR_RESET, id);
}
