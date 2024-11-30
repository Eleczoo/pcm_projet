#include "fifo.hpp"


bool init_queue(void)
{
	NODE *node = (NODE *)malloc(sizeof(NODE));
	if (node == NULL)
		return false;
	
	node->next.link.addr = NULL;
	fifo[HEAD].link.addr = fifo[TAIL].link.addr = node;
	if ((node = (NODE *)malloc(sizeof(NODE))) == NULL) 
	{
		free(fifo[HEAD].link.addr);
		return false;
	}
	
	node->next.link.addr = NULL;
	free_nodes[HEAD].link.addr = node;
	free_nodes[TAIL].link.addr = node;
	return true;
}

bool enq(DATA *value)
{       
	NODE *node = get_free_node();
	if (node == NULL)
		return false;      
	node->value = value; 
	enq_node(fifo, node); 
	return true;         
} 

void free_node(NODE *node)
{ 
	enq_node(free_nodes, node); 
}

void enq_node(CONVERSION *queue, NODE *node)
{
	CONVERSION last, next, new_node;
	node->next.link.ref += 1;
	node->next.link.addr = NULL;

	while (true) 
	{
		last.val = queue[TAIL].val;
		next.val = last.link.addr->next.val;
		if (last.val == queue[TAIL].val)
		{
			if (next.link.addr == NULL) 
			{
				new_node.link.addr = node;
				new_node.link.ref = next.link.ref + 1;
				if (CAS64(&last.link.addr->next.val, next.val, new_node.val))
					break;
			}
			else 
			{
				next.link.ref = last.link.ref + 1;
				CAS64(&queue[TAIL].val, last.val, next.val);
			}
		}
	}
	
	new_node.link.addr = node;
	new_node.link.ref = last.link.ref + 1;
	CAS64(&queue[TAIL].val, last.val, new_node.val);
}

DATA *deq(void)
{
	CONVERSION first, last, next;
	DATA *value;
	while (true) 
	{
		first.val = fifo[HEAD].val;
		last.val = fifo[TAIL].val;
		next.val = first.link.addr->next.val;
		if (first.val == fifo[HEAD].val)
		{
			if (first.link.addr == last.link.addr) 
			{
				if (next.link.addr == NULL)
					return NULL;
				next.link.ref = last.link.ref + 1;
				CAS64(&fifo[TAIL].val, last.val, next.val);
			} 
			else 
			{
				value = next.link.addr->value;
				next.link.ref = first.link.ref + 1;
				if (CAS64(&fifo[HEAD].val, first.val, next.val)) 
				{
					free_node(first.link.addr);
					return value;
				}
			}
		}
	}
}

NODE *get_free_node(void) 
{
	CONVERSION first, last, next;
	while (true) {
		first.val = free_nodes[HEAD].val;
		last.val = free_nodes[TAIL].val;
		next.val = first.link.addr->next.val;
		if (first.val == free_nodes[HEAD].val)
		{
			if (first.link.addr == last.link.addr) {
				if (next.link.addr == NULL)
					return (NODE *)malloc(sizeof(NODE));
				
				next.link.ref = last.link.ref + 1;
				CAS64(&free_nodes[TAIL].val, last.val, next.val);
			} 
			else 
			{
				next.link.ref = first.link.ref + 1;
				if (CAS64(&free_nodes[HEAD].val, first.val, next.val))
					return first.link.addr;
			}
		}
	}
}