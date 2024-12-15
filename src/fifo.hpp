#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include <cstdint>
#include <mutex>
#include <chrono>
#include "atomic.hpp"
#include "path.hpp"


#define HEAD 0
#define TAIL 1

#define SENTINEL_HEAD 0xFFFF5555
#define NB_FREE_NODES 250
// #define DEBUG 1


// typedef uint32_t DATA;
typedef Path DATA;

class Node
{
public:
	DATA* value;
	atomic_stamped<Node> next;


	Node() 
	{
		this->value = nullptr;
		this->next = atomic_stamped<Node>();
	}

	Node(DATA* value) 
	{
		this->value = value;
		this->next = atomic_stamped<Node>();
	}

	Node(atomic_stamped<Node> next) {
		this->value = nullptr;
		this->next = next;
	}

	Node(DATA* value, atomic_stamped<Node> next) {
		this->value = value;
		this->next = next;
	}
};

class LockFreeQueue
{
public:
	LockFreeQueue();
	~LockFreeQueue();

	bool enqueue(DATA* value);
	DATA* dequeue();
	void show_queue();


private:
	Node sentinel = Node();


	// ! Free nodes list head and tail with stamped pointers
	// Create a pool of free nodes

	atomic_stamped<Node> fifo[2];
	atomic_stamped<Node> free_nodes[2];
	Node fnodes[NB_FREE_NODES]; // Actually free nodes
	// DATA values[NB_FREE_NODES]; // VALUES 

	int64_t size;

	// ! Prototypes
	Node* __get_free_node();
	void __free_node(Node* node);
	void __enqueue_node(atomic_stamped<Node>* queue, Node* node);
	Node* __dequeue_node(atomic_stamped<Node>* queue);
	void __show_queue(atomic_stamped<Node>* fifo);
};

LockFreeQueue::LockFreeQueue()
{
	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO

	size = 0;

    //auto start = std::chrono::system_clock::now();

	for (int i = 0; i < NB_FREE_NODES - 1; i++)
	{
		fnodes[i].next.set(&fnodes[i + 1], 0);
		// values[i] = 100 + i;
		// fnodes[i].value = &values[i];
	}

	fnodes[NB_FREE_NODES - 1].next.set(nullptr, 0);
	// values[NB_FREE_NODES - 1] = 100 + NB_FREE_NODES - 1;
	// fnodes[NB_FREE_NODES - 1].value = &values[NB_FREE_NODES - 1];

	sentinel.next.set(nullptr, 0);
	fifo[HEAD].set(&sentinel, 0); 
	fifo[TAIL].set(&sentinel, 0);

	free_nodes[HEAD].set(&fnodes[0], 0);
	free_nodes[TAIL].set(&fnodes[NB_FREE_NODES-1], 0);

	// printf("---- JUST AFTER CREATING THE FIFO ----\n");
	// __show_queue(fifo);
}

LockFreeQueue::~LockFreeQueue()
{
	//uint64_t stamp_osef;
	//delete fifo[HEAD].get(stamp_osef);
	//delete free_nodes[HEAD].get(stamp_osef);
}

Node* LockFreeQueue::__get_free_node()
{
	return new Node();

	//printf("__get_free_node\n");
	// Node* n = __dequeue_node(free_nodes); 
	// if (n)
	// {
	// 	//printf("END __get_free_node\n");
	// 	return n;
	// }
	// else
	// {
	// 	//printf("END __get_free_node GIVING new object\n");
	// 	return new Node();
	// }
}

void LockFreeQueue::__free_node(Node* node)
{
	// printf("__free_node %p\n", node);
	// printf("SHOWING FREE NODE\n");
	// __show_queue(free_nodes);
	// printf("DONE SHOWING FREE NODE\n");

	node->next.set(nullptr, 0);
	node->value = nullptr;

	__enqueue_node(free_nodes, node);
	// printf("__free_node end\n");
}


bool LockFreeQueue::enqueue(DATA* value)
{
	#ifdef DEBUG
		printf("%s\n", __func__);
	#endif

	//std::cout << "Enqueuing: " << *value << std::endl;
	Node* node = __get_free_node();
	
	if (node == nullptr) 
	{
		printf("GET_FREE_NODE : %p\n", node);
		return false;
	}
	//printf("value : %p\n", value);
	node->value = value;
	node->next.set(nullptr, 0);
	
	#ifdef DEBUG
		std::cout << "Node value: " << value << std::endl;
	#endif
	__enqueue_node(fifo, node);

	#ifdef DEBUG
		std::cout << "--- END ENQUEUE" << std::endl;
	#endif

	this->size++;
	// std::cout << "Size  - " << this->size << std::endl;

	return true;
}

void LockFreeQueue::__enqueue_node(atomic_stamped<Node>* queue, Node* node)
{
	#ifdef DEBUG
	printf("%s\n", __func__);
	#endif

	Node* tail;
	Node* next;
	uint64_t tail_stamp, next_stamp;

	while (true)
	{
		// printf("BLOQUE - ENQUEUE !!!!\n");

		if(queue[HEAD].get(tail_stamp) == nullptr)
		{
			printf("HEAD IS NULL\n");
			__show_queue(fifo);
			//printf("tail : %p - %p\n", tail, queue[TAIL].get(tail_stamp));
			// std::cout << "tail IS NULL" << std::endl;
		}
		tail = queue[TAIL].get(tail_stamp);
		// Show tail
		if(tail == nullptr)
		{
			__show_queue(fifo);
			printf("tail : %p - %p\n", tail, queue[TAIL].get(tail_stamp));
			// std::cout << "tail IS NULL" << std::endl;
		}
		next = tail->next.get(next_stamp);

		//#ifdef DEBUG
		//std::cout << "------ __enqueue_node() tail : " << tail << std::endl;
		//std::cout << "------ __enqueue_node() NEXT : " << next << std::endl;
		//#endif
		if (tail == queue[TAIL].get(tail_stamp))
		{
			// If its the tail (tail), no operation is in progress
			if (next == nullptr)
			{
				// ? Set the current tail's next to our new node
				bool ret;
				ret = tail->next.cas(next, node, next_stamp, next_stamp + 1);
				if(ret)
				{
					// ? Set the tail to the new node
					queue[TAIL].cas(tail, node, tail_stamp, tail_stamp + 1);
					#ifdef DEBUG
					#endif
					// std::cout << "--- END ENQUEUE NODE" << std::endl;
					//std::cout << "!";

					return;
				}
			}
			else // The previous action has not been completed
			{
				// Finish the operation for the other thread
				if(next == tail)
					printf("! SAME VALUES ! %p %p\n", next, tail);	

				//printf("CAS TAIL %p %p\n", tail, next);
				
				queue[TAIL].cas(tail, next, tail_stamp, tail_stamp + 1);
			}
		}
	}
}

DATA* LockFreeQueue::dequeue()
{
	DATA* data;
	Node* n = __dequeue_node(fifo);
	if(n != nullptr)
	{	
		data = n->value;
		delete n;
		// __free_node(n);
		return data;
	}
	return nullptr;
}


Node* LockFreeQueue::__dequeue_node(atomic_stamped<Node>* queue)
{
	Node* head;
	Node* tail;
	Node* next;
	Node* next_next;
	uint64_t head_stamp, tail_stamp, next_stamp, next_next_stamp;

	#ifdef DEBUG
	printf("%s\n", __func__);
	#endif
	
	while (true)
	{
		head = queue[HEAD].get(head_stamp);
		tail = queue[TAIL].get(tail_stamp);
		next = head->next.get(next_stamp);
		if (next != nullptr)
			next_next = next->next.get(next_next_stamp);
		else
			next_next = nullptr;

		//printf("[LFQ] head : %p\n", head);
		//printf("[LFQ] tail : %p\n", tail);
		//printf("[LFQ] next : %p\n", next);
		// printf("BLOQUE - DEQUEUE !!!!\n");

		if (head == queue[HEAD].get(head_stamp))
		{
			// ! If the fifo is empty
			if (head == tail)
			{
				if (!next) 
				{
					#ifdef DEBUG
					std::cout << "--- END DEQUEUE NULL PTR" << std::endl;
					#endif
					return nullptr;
				}

				// ! Finish the operation for the other thread
				#ifdef DEBUG
				std::cout << "--- DEQUEUE HELP" << std::endl;
				#endif
				queue[TAIL].cas(tail, next, tail_stamp, tail_stamp + 1);
			}
			else
			{
				// Only sentinel remains
				if(next_next == nullptr)
				{
					// set the tail like the head, pointing to the sentinel
					if (!queue[TAIL].cas(tail, head, tail_stamp, tail_stamp + 1))
						continue;
				}
				
				// Point the head to its next's next node
				if (!head->next.cas(next, next_next, next_stamp, next_stamp + 1))
					continue;


				#ifdef DEBUG
				std::cout << "--- END DEQUEUE" << std::endl;
				#endif

				this->size--;
				// std::cout << "Size  - " << this->size << std::endl;
				return next;
			}
		}
	}

}



void LockFreeQueue::__show_queue(atomic_stamped<Node>* fifo)
{
	uint64_t stamp_osef, current_stamp;
	uint64_t i = 0;

	Node* current = fifo[HEAD].get(current_stamp);
	Node* tail = fifo[TAIL].get(stamp_osef);

	printf("-------\n");
	printf("show_queue() HEAD  :     (%p)\n", current);
	printf("show_queue() TAIL  :     (%p)\n", tail);

	while (current != fifo[TAIL].get(stamp_osef))
	{
		printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, 0, current, current->next);
		// if(i == 0)
		// {
		// }
		// else
		// {
			// printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, *current->value, current, current->next);
		// }
		current = current->next.get(current_stamp);
	}
	printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, 0, current, current->next);
	// if(i == 0)
	// {
	// }
	// else
	// {
	// 	printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, *current->value, current, current->next);
	// }
	printf("--------------------------------\n");
}




void LockFreeQueue::show_queue()
{
	__show_queue(fifo);
}

#endif // __FIFO_HPP__