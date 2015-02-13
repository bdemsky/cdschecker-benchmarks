#include <stdatomic.h>
#include <atomic>

#include "wildcard.h"

#define release memory_order_release 
#define acquire memory_order_acquire 
#define acq_rel memory_order_acq_rel
#define relaxed memory_order_relaxed

struct node {
	unsigned int value;
	// This field does not have to be atomic, but in the inference analysis, we
	// might have a data race for this field without the proper synchronization.
	//node *next;
	atomic<node*> next;

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
			// n->next = old;
			n->next.store(old, wildcard(2));
		} while (!top.compare_exchange_strong(old, n, wildcard(3), wildcard(4)));
		// relaxed & relaxed
	}

	unsigned int pop() {
		node *old = top.load(wildcard(5)); // acquire
		node *n;
		do {
			if (!old)
				return 0;
			// n = old->next;
			n = old->next.load(relaxed);
		} while (!top.compare_exchange_strong(old, n, wildcard(6), wildcard(7)));
		// relaxed & relaxed 
		return old->value;
	}
};

