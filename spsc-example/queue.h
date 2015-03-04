#include <unrelacy.h>
#include <atomic>

using namespace std;

struct node {
	node(int d): data(d) {
		next.store(0, memory_order_relaxed);
	}
	atomic<node*> next;
	int data;
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
		node *h = head.load(memory_order_relaxed); 
		h->next.store(n, memory_order_release);
		head.store(n, memory_order_relaxed);
	}

	bool dequeue(int *v) {
		node* t = tail.load(memory_order_relaxed);
		node* n = t->next.load(memory_order_acquire);
		if (0 == n)
			return false;
		*v = n->data;
		tail.store(n, memory_order_relaxed);
		delete (t);
		return true;
	}
};
