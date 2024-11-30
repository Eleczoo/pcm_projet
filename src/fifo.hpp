#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include <cstdint>
#include "atomic.hpp"

#define HEAD 0
#define TAIL 1

typedef uint64_t DATA;

struct Node
{
	DATA* value;
	Node* next;

	Node() {
		value = nullptr;
		next = nullptr;
	}
};

class LockFreeQueue
{
public:
	LockFreeQueue();
	~LockFreeQueue();

	bool enqueue(DATA* value);
	DATA* dequeue();


private:

	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO
	atomic_stamped<Node> fifo[2] = { 
		atomic_stamped<Node>(new Node(), 0), 
		atomic_stamped<Node>(new Node(), 1) 
	};

	// ! Free nodes list head and tail with stamped pointers
	atomic_stamped<Node> free_nodes[2] = { 
		atomic_stamped<Node>(new Node(), 0), 
		atomic_stamped<Node>(new Node(), 1) 
	};

	// ! Prototypes
	Node* get_free_node();
	void free_node(Node* node);
	void enqueue_node(atomic_stamped<Node>* queue, Node* node);
};

LockFreeQueue::LockFreeQueue()
{
	Node* node = new Node();
	fifo[HEAD].set(node, 0);
	fifo[TAIL].set(node, 0);
	
	Node* free_node = new Node();
	free_nodes[HEAD].set(free_node, 0);
	free_nodes[TAIL].set(free_node, 0);
}

LockFreeQueue::~LockFreeQueue()
{
	uint64_t stamp_osef;
	delete fifo[HEAD].get(stamp_osef);
	delete free_nodes[HEAD].get(stamp_osef);
}


bool LockFreeQueue::enqueue(DATA* value)
{
	Node* node = get_free_node();
	if (!node) return false;

	node->value = value;
	enqueue_node(fifo, node);
	return true;
}

DATA* LockFreeQueue::dequeue()
{
	Node* first;
	Node* last;
	Node* next;
	uint64_t first_stamp, last_stamp;

	while (true)
	{
		first = fifo[HEAD].get(first_stamp);
		last = fifo[TAIL].get(last_stamp);
		next = first->next;

		if (first == fifo[HEAD].get(first_stamp))
		{
			if (first == last)
			{
				if (!next) return nullptr;
				fifo[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			} else
			{
				DATA* value = next->value;
				if (fifo[HEAD].cas(first, next, first_stamp, first_stamp + 1))
				{
					free_node(first);
					return value;
				}
			}
		}
	}
}

Node* LockFreeQueue::get_free_node()
{
	Node* first;
	Node* last;
	Node* next;
	uint64_t first_stamp, last_stamp;

	while (true)
	{
		first = free_nodes[HEAD].get(first_stamp);
		last = free_nodes[TAIL].get(last_stamp);
		next = first->next;

		if (first == free_nodes[HEAD].get(first_stamp))
		{
			if (first == last)
			{
				if (!next) return new Node();
				free_nodes[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			} else
			{
				if (free_nodes[HEAD].cas(first, next, first_stamp, first_stamp + 1))
					return first;
			}
		}
	}
}

void LockFreeQueue::free_node(Node* node)
{
	enqueue_node(free_nodes, node);
}

void LockFreeQueue::enqueue_node(atomic_stamped<Node>* queue, Node* node)
{
	Node* last;
	Node* next;
	uint64_t last_stamp;

	node->next = nullptr;

	while (true)
	{
		last = queue[TAIL].get(last_stamp);
		next = last->next;

		if (last == queue[TAIL].get(last_stamp))
		{
			if (!next)
			{
				if (queue[TAIL].cas(last, node, last_stamp, last_stamp + 1)) break;
			} else
			{
				queue[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
		}
	}
	queue[TAIL].cas(last, node, last_stamp, last_stamp + 1);
}

#endif // __FIFO_HPP__