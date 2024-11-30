#pragma once

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <math.h>
#include <cerrno>
#include <cstring>
#include <cstdint>

#define HEAD 0
#define TAIL 1


typedef uint64_t DATA;
struct NODE; 				// Permet d'utiliser addr sans cast

typedef struct ADDRESS { 	// Adresse avec estampille
	uint32_t ref; 			// estampille
	struct NODE *addr; 		// prochain noeud dans la file
} ADDRESS;

typedef union { 			// permet de changer de représentation
	ADDRESS link;
	uint64_t val;
} CONVERSION;

typedef struct NODE { 		// Noeud mis en file
	DATA *value; 			// valeur mise en file
	CONVERSION next; 		// prochain noeud dans la file
} NODE;

static CONVERSION fifo[2];
static CONVERSION free_nodes[2];

bool init_queue(void);
bool enq(DATA *value);
void free_node(NODE *node);
void enq_node(CONVERSION *queue, NODE *node);
DATA *deq(void);
NODE *get_free_node(void);