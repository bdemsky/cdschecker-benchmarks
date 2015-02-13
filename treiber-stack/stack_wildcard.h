#include <stdatomic.h>
#include <atomic>

#include "wildcard.h"

#define release memory_order_release 
#define acquire memory_order_acquire 
#define acq_rel memory_order_acq_rel
#define relaxed memory_order_relaxed

struct node {
	unsigned int value;
	node *next;

	node(unsigned int v) {
		value = v;
		next = NULL;
	}
};

struct stack {
	atomic<node*> top;

	stack() {
		atomic_init(&top, NULL);
	}

	void push(unsigned int val) {
		node *n = new node(val);
		node *old = top.load(wildcard(1)); // acquire
		do {
			n->next = old;
		} while (!top.compare_exchange_strong(old, n, wildcard(2), wildcard(3)));
		// acq_rel & relaxed
	}

	unsigned int pop() {
		node *old = top.load(wildcard(4)); // acquire
		node *n;
		do {
			if (!old)
				return 0;
			n = old->next;
		} while (!top.compare_exchange_strong(old, n, wildcard(5), wildcard(6)));
		// acq_rel & acquire
		return old->value;
	}
};

