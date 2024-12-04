#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include <cstdint>
#include "atomic.hpp"
#include "path.hpp"

#define HEAD 0
#define TAIL 1

#define SENTINEL_HEAD 0xFFFF5555
#define NB_FREE_NODES 256

typedef Path DATA;

class Node
{
public:
	DATA* value;
	atomic_stamped<Node> next;

	Node() 
	{
		value = nullptr;
		next = atomic_stamped<Node>();
	}

	Node(DATA* value) 
	{
		value = value;
		next = atomic_stamped<Node>();
	}

	Node(atomic_stamped<Node> next) {
		value = nullptr;
		next = next;
	}

	Node(DATA* value, atomic_stamped<Node> next) {
		value = value;
		next = next;
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

	// ! Prototypes
	Node* __get_free_node();
	void free_node(Node* node);
	void __enqueue_node(atomic_stamped<Node>* queue, Node* node);
};

LockFreeQueue::LockFreeQueue()
{
	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO

	for (int i = 0; i < NB_FREE_NODES-1; i++)
	{
		fnodes[i].next.set(&fnodes[i+1], 0);
	}

	fifo[0].set(&sentinel, 0); 
	fifo[1].set(&sentinel, 0);

	free_nodes[0].set(&fnodes[0], 0);
	free_nodes[1].set(&fnodes[NB_FREE_NODES-1], 0);
}

LockFreeQueue::~LockFreeQueue()
{
	//uint64_t stamp_osef;
	//delete fifo[HEAD].get(stamp_osef);
	//delete free_nodes[HEAD].get(stamp_osef);
}


bool LockFreeQueue::enqueue(DATA* value)
{
	std::cout << "ENQUEUE" << std::endl;

	//std::cout << "Enqueuing: " << *value << std::endl;
	Node* node = __get_free_node();
	if (!node) return false;

	node->value = value;
	//std::cout << "Node value: " << *node->value << std::endl;
	__enqueue_node(fifo, node);
	std::cout << "END ENQUEUE" << std::endl;

	return true;
}

DATA* LockFreeQueue::dequeue()
{
	std::cout << "DEQUEUE" << std::endl;

	Node* first;
	Node* last;
	Node* next;
	uint64_t first_stamp, last_stamp, next_stamp;

	while (true)
	{
		first = fifo[HEAD].get(first_stamp);
		last = fifo[TAIL].get(last_stamp);
		next = first->next.get(next_stamp);

		if (first == fifo[HEAD].get(first_stamp))
		{
			// ! If the queue is empty
			if (first == last)
			{
				if (!next) 
					return nullptr;

				// ! Finish the operation for the other thread
				fifo[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
			else
			{
				DATA* value = next->value;
				
				// Point the head to its next's next node
				bool ret;
				ret = first->next.cas(next, next->next.get(next_stamp), next_stamp, next_stamp + 1);
				if(!ret)
				{
					std::cout << "CAS" << std::endl;
					continue;
				}

				// Only sentinel remains
				if(first->next.get(first_stamp) == nullptr)
				{
					// set the tail like the head, pointing to the sentinel
					do
					{
						ret = fifo[TAIL].cas(last, first, last_stamp, last_stamp + 1);
					} while (ret == false);
				}

				free_node(next);
				return value;
			}
		}
	}
	std::cout << "END DEQUEUE" << std::endl;

}

// void LockFreeQueue::show_queue()
// {
// 	uint64_t stamp_osef;
// 	uint64_t i = 0;

// 	Node* current = fifo[HEAD].get(stamp_osef);
// 	Node* tail = fifo[TAIL].get(stamp_osef);
	
// 	while (current != fifo[TAIL].get(stamp_osef))
// 	{
// 		current->value.print(std:cout);
// 		//std::cout << "Value[" << i++ << "] = " << *current->value.print() << std::endl;
// 		//std::cout << "NEXT : " << current->next << std::endl;
// 		current = current->next;

// 		// print current and tail
// 		//std::cout << "CURRENT = " << current << std::endl;
// 		//std::cout << "TAIL = " << tail << std::endl;

// 	}
// 	//std::cout << "TAIL = " << *current->value << std::endl;
// 	std::cout << "End of queue" << std::endl;
// }

Node* LockFreeQueue::__get_free_node()
{
	std::cout << "GET FREE NODE" << std::endl;

	Node* first;
	Node* last;
	Node* next;
	uint64_t first_stamp, last_stamp;

	while (true)
	{
		first = free_nodes[HEAD].get(first_stamp);
		last = free_nodes[TAIL].get(last_stamp);
		next = first->next.get(last_stamp);

		if (first == free_nodes[HEAD].get(first_stamp))
		{
			if (first == last)
			{
				// TODO: remove the malloc ?
				if (!next) return new Node();
				free_nodes[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
			else
			{
				if (free_nodes[HEAD].cas(first, next, first_stamp, first_stamp + 1))
					return first;
			}
		}
	}
}

void LockFreeQueue::free_node(Node* node)
{
	__enqueue_node(free_nodes, node);
}

void LockFreeQueue::__enqueue_node(atomic_stamped<Node>* queue, Node* node)
{
	Node* last;
	Node* next;
	uint64_t last_stamp;

	// ? Set the new Node's next to nullptr, because it's the new tail
	node->next.set(nullptr, 0);

	while (true)
	{
		last = queue[TAIL].get(last_stamp);
		next = last->next.get(last_stamp);

		if (last == queue[TAIL].get(last_stamp))
		{
			// If its the last, no operation is in progress
			if (next == nullptr)
			{
				// ? Set the current tail's next to our new node
				bool ret;
				ret = last->next.cas(next, node, last_stamp, last_stamp + 1);
				if(ret)
				{
					queue[TAIL].cas(last, node, last_stamp, last_stamp + 1);
					return;
				}
			}
			else
			{
				// Finish the operation for the other thread
				queue[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
		}
	}
}


#endif // __FIFO_HPP__