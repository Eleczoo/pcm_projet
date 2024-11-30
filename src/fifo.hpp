#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include <cstdint>
#include "atomic.hpp"

#define HEAD 0
#define TAIL 1

#define SENTINEL_HEAD 0xFFFF5555
#define SENTINEL_TAIL 0xFFFFAAAA

typedef uint64_t DATA;

class Node
{
public:
	DATA* value;
	Node* next;

	Node() {
		value = nullptr;
		next = nullptr;
	}

	Node(Node* next) {
		value = nullptr;
		this->next = next;
	}

	Node(DATA* value, Node* next) {
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

	const DATA val_head = SENTINEL_HEAD;
	const DATA val_tail = SENTINEL_TAIL;

	//Node ntail;
	//Node nhead;
	Node ntail = Node(&val_tail, nullptr);
	Node nhead = Node(&val_head, &ntail);

	atomic_stamped<Node> fifo[2];
	atomic_stamped<Node> free_nodes[2];

	// ! Prototypes
	Node* __get_free_node();
	void free_node(Node* node);
	void __enqueue_node(atomic_stamped<Node>* queue, Node* node);
};

LockFreeQueue::LockFreeQueue()
{
	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO



	fifo[0].set(&nhead, 0); 
	fifo[1].set(&ntail, 1);

	// ! Free nodes list head and tail with stamped pointers
	Node free2 = Node();
	Node free1 = Node(&free2);

	free_nodes[0].set(&free1, 0); 
	free_nodes[1].set(&free2, 1);

}

LockFreeQueue::~LockFreeQueue()
{
	uint64_t stamp_osef;
	delete fifo[HEAD].get(stamp_osef);
	delete free_nodes[HEAD].get(stamp_osef);
}


bool LockFreeQueue::enqueue(DATA* value)
{

	std::cout << "Enqueuing: " << *value << std::endl;
	Node* node = __get_free_node();
	if (!node) return false;

	node->value = value;
	std::cout << "Node value: " << *node->value << std::endl;
	__enqueue_node(fifo, node);
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

void LockFreeQueue::show_queue()
{
	uint64_t stamp_osef;
	uint64_t i = 0;

	Node* current = fifo[HEAD].get(stamp_osef);
	Node* tail = fifo[TAIL].get(stamp_osef);

	std::cout << current << std::endl;
	std::cout << tail << std::endl;
	while (current != fifo[TAIL].get(stamp_osef))
	{
		std::cout << "Value[" << i++ << "] = " << *current->value << std::endl;
		//std::cout << current->next << std::endl;
		current = current->next;
		if(current == 0)
			break;
	}
	std::cout << "End of queue" << std::endl;
}

Node* LockFreeQueue::__get_free_node()
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
	__enqueue_node(free_nodes, node);
}

void LockFreeQueue::__enqueue_node(atomic_stamped<Node>* queue, Node* node)
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
				if (queue[TAIL].cas(last, node, last_stamp, last_stamp + 1)) 
				{
					break;
				}
			} 
			else
			{
				queue[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
		}
	}
	queue[TAIL].cas(last, node, last_stamp, last_stamp + 1);
}



#endif // __FIFO_HPP__