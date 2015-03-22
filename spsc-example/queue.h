#include <unrelacy.h>
#include <atomic>
#include "wildcard.h"

using namespace std;

#define acquire memory_order_acquire
#define release memory_order_release
#define relaxed memory_order_relaxed

struct node {
	node(int idx) {
		index.store(idx, relaxed);
		next.store(0, relaxed);
	}
	atomic<node*> next;
	atomic<int> index;
};

class spsc_queue
{
	node *head, *tail;
	public:
	spsc_queue() {
		head = tail = new node(-1);
	}
	void enqueue(int idx) {
		node* n = new node(idx);
		tail->next.store(n, release);
		tail = n;
	}
	bool dequeue(int *idx) {
		node *tmp = head;
		node *n = tmp->next.load(acquire);
		if (NULL == n) return false;
		head = n;
		*idx = n->index.load(relaxed);
		delete (tmp);
		return true;
	}
};
