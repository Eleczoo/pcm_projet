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
#define NB_FREE_NODES 10000
// #define DEBUG 1


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

	int64_t size; 

	// ! Prototypes
	Node* __get_free_node();
	void __free_node(Node* node);
	void __enqueue_node(atomic_stamped<Node>* queue, Node* node);
	Node* __dequeue_node(atomic_stamped<Node>* queue);
};

LockFreeQueue::LockFreeQueue()
{
	// ! FIFO is actually the head and the tail
	// ! OF THE FIFO

	size = 0;

    //auto start = std::chrono::system_clock::now();

	for (int i = 0; i < NB_FREE_NODES - 1; i++)
		fnodes[i].next.set(&fnodes[i + 1], 0);

	fnodes[NB_FREE_NODES - 1].next.set(nullptr, 0);

	sentinel.next.set(nullptr, 0);
	fifo[HEAD].set(&sentinel, 0); 
	fifo[TAIL].set(&sentinel, 0);

	free_nodes[HEAD].set(&fnodes[0], 0);
	free_nodes[TAIL].set(&fnodes[NB_FREE_NODES-1], 0);
}

LockFreeQueue::~LockFreeQueue()
{
	//uint64_t stamp_osef;
	//delete fifo[HEAD].get(stamp_osef);
	//delete free_nodes[HEAD].get(stamp_osef);
}

Node* LockFreeQueue::__get_free_node()
{
	//printf("__get_free_node\n");
	Node* n = __dequeue_node(free_nodes); 
	if (n)
	{
		//printf("END __get_free_node\n");
		return n;
	}
	else
	{
		//printf("END __get_free_node GIVING new object\n");
		return new Node();
	}
}

void LockFreeQueue::__free_node(Node* node)
{
	printf("__free_node %p\n", node);
	__enqueue_node(free_nodes, node);
	printf("__free_node end\n");
}


bool LockFreeQueue::enqueue(DATA* value)
{
	#ifdef DEBUG
		std::cout << "ENQUEUE" << std::endl;
	#endif

	//std::cout << "Enqueuing: " << *value << std::endl;
	Node* node = __get_free_node();
	
	if (node == nullptr) 
	{
		printf("GET_FREE_NODE : %p\n", node);
		return false;
	}
	//printf("FREE NODE : %p\n", node);
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
	//std::cout << "ENQUEUE NODE" << std::endl;
	#endif

	Node* tail;
	Node* next;
	uint64_t tail_stamp, next_stamp;

	while (true)
	{
		tail = queue[TAIL].get(tail_stamp);
		// Show tail
		if(tail == nullptr)
		{
			printf("tail : %p - %p\n", tail, queue[TAIL].get(tail_stamp));
			//std::cout << "tail IS NULL" << std::endl;
		}
		next = tail->next.get(next_stamp);

		#ifdef DEBUG
		// std::cout << "------ __enqueue_node() tail : " << tail << std::endl;
		// std::cout << "------ __enqueue_node() NEXT : " << next << std::endl;
		#endif
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
					std::cout << "--- END ENQUEUE NODE" << std::endl;
					#endif
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
		__free_node(n);
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

	while (true)
	{
		head = queue[HEAD].get(head_stamp);
		tail = queue[TAIL].get(tail_stamp);
		next = head->next.get(next_stamp);
		if (next != nullptr)
			next_next = next->next.get(next_next_stamp);

		//printf("[LFQ] head : %p\n", head);
		//printf("[LFQ] tail : %p\n", tail);
		//printf("[LFQ] next : %p\n", next);

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
				head->next.cas(next, next_next, next_stamp, next_stamp + 1);


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








void LockFreeQueue::show_queue()
{
	uint64_t stamp_osef;
	uint64_t i = 0;
	uint32_t max_count = 0;

	Node* current = fifo[HEAD].get(stamp_osef);
	Node* tail = fifo[TAIL].get(stamp_osef);

	printf("show_queue() HEAD : %p\n", current);
	printf("show_queue() TAIL : %p\n", tail);


	while (current != fifo[TAIL].get(stamp_osef))
	{
		if(max_count++ > 20)
			break;
		// current->value.print(std::cout);

		std::cout << "show_queue() current " << current  << std::endl;
		if (i == 0)
		{
			printf("show_queue() current.value[%ld] : %p\n", i++, current->value);
		}
		else
		{
			std::cout << "show_queue() Value[" << i++ << "] = " << current->value << std::endl;
		}
		//std::cout << "show_queue() current->value " << current->value  << std::endl;
		// std::cout << "show_queue() Value[" << i++ << "] = " << current->value << std::endl;
		//std::cout << "NEXT : " << current->next << std::endl;
		//printf(" show_queue() NEXT : %p\n", current->next.get(stamp_osef));
		current = current->next.get(stamp_osef);

		// print current and tail
		//std::cout << "CURRENT = " << current << std::endl;
		//std::cout << "TAIL = " << tail << std::endl;

	}
	// printf("show_queue() current.value[%d] : %p\n", i, current->value);
	if (i != 0)
	{
		std::cout << "show_queue() Value[" << i++ << "] = " << current->value << std::endl;
	}

	//std::cout << "TAIL = " << *current->value << std::endl;
	#ifdef DEBUG
	std::cout << "End of queue" << std::endl;
	#endif
}

#endif // __FIFO_HPP__