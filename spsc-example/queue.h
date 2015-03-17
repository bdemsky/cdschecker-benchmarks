#include <unrelacy.h>
#include <atomic>
#include "wildcard.h"

using namespace std;

struct node {
	node(int d) {
		data.store(0, memory_order_normal);
		next.store(0, wildcard(1));
	}
	atomic<node*> next;
	atomic_int data;
};

class spsc_queue
{
	atomic<node*> head, tail;
	public:
	spsc_queue() {
		head = tail = new node(0);
	}

	void enqueue(int val) {
		node* n = new node(val);
		node *h = head.load(wildcard(2)); 
		h->next.store(n, wildcard(3));
		head.store(n, wildcard(4));
	}

	bool dequeue(int *v) {
		node* t = tail.load(wildcard(5));
		node* n = t->next.load(wildcard(6));
		if (0 == n)
			return false;
		*v = n->data.load(memory_order_normal);
		tail.store(n, wildcard(7));
		delete (t);
		return true;
	}
};
