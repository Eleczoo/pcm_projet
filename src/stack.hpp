#ifndef __FIFO_HPP__
#define __FIFO_HPP__

#include <cstdint>
#include <mutex>
#include <chrono>
#include "atomic.hpp"
#include "path.hpp"


#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"
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

class LockFreeStack
{
public:
	LockFreeStack();
	~LockFreeStack();

	bool enqueue(DATA* value);
	DATA* dequeue();
	void show_queue();


private:
	// ! Free nodes list head and tail with stamped pointers
	// Create a pool of free nodes

	atomic_stamped<Node> fifo[2];
	atomic_stamped<Node> free_nodes[2];
	//Node fnodes[NB_FREE_NODES]; // Actually free nodes
	// DATA values[NB_FREE_NODES]; // VALUES 

	int64_t size;

	// ! Prototypes
	Node* __get_free_node();
	void __free_node(Node* node);
	void __enqueue_node(atomic_stamped<Node>* queue, Node* node);
	Node* __dequeue_node(atomic_stamped<Node>* queue);
	void __show_queue(atomic_stamped<Node>* fifo);
};

LockFreeStack::LockFreeStack()
{
	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO

	size = 0;
	//printf("TESTING \n");

    //auto start = std::chrono::system_clock::now();

	//for (int i = 0; i < NB_FREE_NODES - 1; i++)
	//{
	//	fnodes[i].next.set(&fnodes[i + 1], 0);
	//	// values[i] = 100 + i;
	//	// fnodes[i].value = &values[i];
	//}

	//fnodes[NB_FREE_NODES - 1].next.set(nullptr, 0);
	// values[NB_FREE_NODES - 1] = 100 + NB_FREE_NODES - 1;
	// fnodes[NB_FREE_NODES - 1].value = &values[NB_FREE_NODES - 1];

	fifo[HEAD].set(nullptr, 0); 

	//free_nodes[HEAD].set(&fnodes[0], 0);
	//free_nodes[TAIL].set(&fnodes[NB_FREE_NODES-1], 0);

	// printf("---- JUST AFTER CREATING THE FIFO ----\n");
	// __show_queue(fifo);
}

LockFreeStack::~LockFreeStack()
{
	//uint64_t stamp_osef;
	//delete fifo[HEAD].get(stamp_osef);
	//delete free_nodes[HEAD].get(stamp_osef);
}

Node* LockFreeStack::__get_free_node()
{
	return new Node();
}

void LockFreeStack::__free_node(Node* node)
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


bool LockFreeStack::enqueue(DATA* value)
{

	Node* node = __get_free_node();
	if (node == nullptr) 
		return false;

	node->value = value;
	node->next.set(nullptr, 0);
	
	__enqueue_node(fifo, node);

	this->size++;
	//__atomic_fetch_add(&this->size, 1, __ATOMIC_RELAXED);

	return true;
}

void LockFreeStack::__enqueue_node(atomic_stamped<Node>* queue, Node* node)
{
	uint64_t head_stamp;
	uint64_t next_stamp = 0;

	while (true)
	{
		// __sync_synchronize();
		Node* head = queue[HEAD].get(head_stamp);
		node->next.set(head, next_stamp);
		if(queue[HEAD].cas(head, node, head_stamp, head_stamp + 1))
		{
			break;
		}
	}
}

DATA* LockFreeStack::dequeue()
{
	DATA* data;
	Node* n = __dequeue_node(fifo);
	if(n != nullptr)
	{	
		data = n->value;
		return data;
	}
	return nullptr;
}


Node* LockFreeStack::__dequeue_node(atomic_stamped<Node>* queue)
{
	Node* head;
	Node* new_head;
	uint64_t head_stamp, next_stamp;

	#ifdef DEBUG
	printf("%s\n", __func__);
	#endif
	
	while (true)
	{
		// __sync_synchronize();
		head = queue[HEAD].get(head_stamp);
		
		if(head == nullptr)
		{
			return nullptr;
		}

		new_head = head->next.get(next_stamp);
		
		if(queue[HEAD].cas(head, new_head, head_stamp, head_stamp + 1))
		{
			return head;
		}
	}

}



void LockFreeStack::__show_queue(atomic_stamped<Node>* fifo)
{
	uint64_t stamp_osef, current_stamp;
	uint64_t i = 0;

	Node* current = fifo[HEAD].get(current_stamp);
	Node* tail = fifo[TAIL].get(stamp_osef);

	printf("-------\n");
	// printf("show_queue() HEAD  :     (%p) -> %p\n", fifo[HEAD], current);
	// printf("show_queue() TAIL  :     (%p) -> %p\n", fifo[TAIL], tail);
	// printf("show_queue() HEAD  :     (%p)\n", current);
	// printf("show_queue() TAIL  :     (%p)\n", tail);
	printf("show_queue() HEAD  :     (%p) -> %p\n", &fifo[HEAD], current);
	printf("show_queue() TAIL  :     (%p) -> %p -> %p\n", &fifo[TAIL], tail, tail->next.get(stamp_osef));

	while (current != fifo[TAIL].get(stamp_osef))
	{
		printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, 0, current, current->next.get(stamp_osef));
		
		if (current->next.get(current_stamp) == nullptr)
		{
			break;
		}
		current = current->next.get(current_stamp);
	}
	printf("show_queue() [%03lu] : %03d (%p) -> %p\n", i++, 0, current, current->next.get(stamp_osef));


	current = fifo[HEAD].get(current_stamp);
	tail = fifo[TAIL].get(stamp_osef);
	printf("after show_queue() HEAD  :     (%p) -> %p\n", &fifo[HEAD], current);
	printf("after show_queue() TAIL  :     (%p) -> %p -> %p\n", &fifo[TAIL], tail, tail->next.get(stamp_osef));

	printf("--------------------------------\n");
}




void LockFreeStack::show_queue()
{
	__show_queue(fifo);
}

#endif // __FIFO_HPP__