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

	DATA val_head;

	Node sentinel = Node(&val_head, nullptr);

	// ! Free nodes list head and tail with stamped pointers
	// Create a pool of free nodes

	atomic_stamped<Node> fifo[2];
	Node fnodes[NB_FREE_NODES]; // Actually free nodes
	atomic_stamped<Node> free_nodes[NB_FREE_NODES];

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
		fnodes[i].next = &fnodes[i+1];
		free_nodes[i].set(&fnodes[i], 0);
	}
	free_nodes[NB_FREE_NODES-1].set(&fnodes[NB_FREE_NODES-1], 0);

	fifo[0].set(&sentinel, 0); 
	fifo[1].set(&sentinel, 0);

	free_nodes[0].set(&fnodes[0], 0);
	// free_nodes[0].set(&free1, 0); 
	// free_nodes[1].set(&free2, 1);
}

LockFreeQueue::~LockFreeQueue()
{
	uint64_t stamp_osef;
	//delete fifo[HEAD].get(stamp_osef);
	//delete free_nodes[HEAD].get(stamp_osef);
}


bool LockFreeQueue::enqueue(DATA* value)
{

	//std::cout << "Enqueuing: " << *value << std::endl;
	Node* node = __get_free_node();
	if (!node) return false;

	node->value = value;
	//std::cout << "Node value: " << *node->value << std::endl;
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
				fifo[HEAD].next = fifo[HEAD].next->next;
				free_node(next);
				return value;
				// if (fifo[HEAD].cas(first, next, first_stamp, first_stamp + 1))
				// {
					// free_node(first);
					// return value;
				// }
			}
		}
	}
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

	// atomic_stamped<Node*> pnext;


	node->next = nullptr;

	while (true)
	{
		last = queue[TAIL].get(last_stamp);
		next = last->next;

		if (last == queue[TAIL].get(last_stamp))
		{
			// If its the last, no operation is in progress
			if (next == nullptr)
			{
				// std::cout << "here" << std::endl;
				if (queue[TAIL].cas(last, node, last_stamp, last_stamp + 1)) 
				{
					last->next = node;
					break;
				}
			}
			else
			{
				// Finish the operation for the other thread
				queue[TAIL].cas(last, next, last_stamp, last_stamp + 1);
			}
		}
	}
	//queue[TAIL].cas(last, node, last_stamp, last_stamp + 1);

}



// void enq_node(CONVERSION *queue, NODE *node)
// {
// 	CONVERSION last, next, new_node;
// 	node->next.link.ref += 1;
// 	node->next.link.addr = NULL;
// 	while (true) 
// 	{
// 		last.val = queue[TAIL].val;
// 		next.val = last.link.addr->next.val;
// 		if (last.val == queue[TAIL].val)
// 		{
// 			if (next.link.addr == NULL) 
// 			{
// 				new_node.link.addr = node;
// 				new_node.link.ref = next.link.ref + 1;
// 				if (CAS64(&last.link.addr->next.val, next.val, new_node.val))
// 					break;
// 			}
// 			else 
// 			{
// 				next.link.ref = last.link.ref + 1;
// 				CAS64(&queue[TAIL].val, last.val, next.val);
// 			}
// 		}
// 	}

// 	new_node.link.addr = node;
// 	new_node.link.ref = last.link.ref + 1;
// 	CAS64(&queue[TAIL].val, last.val, new_node.val);
// }





#endif // __FIFO_HPP__