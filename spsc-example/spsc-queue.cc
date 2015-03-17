#include <threads.h>
#include <atomic>

#include "wildcard.h" 

using namespace std;

struct node {
	node(int d) {
		data.store(d, memory_order_normal);
		next.store(0, wildcard(1));
	}
	atomic<node*> next;
	atomic_int data;
};

class spsc_queue { 
	atomic<node*> head, tail;/*@ \label{line:spscBegin} @*/
	public:
	spsc_queue() {
		head = tail = new node(-1);
	}
	void enqueue(int val) {
		node* n = new node(val);
		node *h = head.load(wildcard(2)); 
		h->next.store(n, wildcard(3));/*@ \label{line:spscRelease} @*/
		head.store(n, wildcard(4));
	}
	bool dequeue(int *v) {
		node* t = tail.load(wildcard(5));
		node* n = t->next.load(wildcard(6));/*@ \label{line:spscAcquire} @*/
		if (0 == n)
			return false;
		*v = n->data.load(memory_order_normal);
		tail.store(n, wildcard(7));
		delete (t);
		return true;
	}
};


spsc_queue *q;/*@ \label{line:testcaseBegin} @*/
atomic_int arr[2];
void thrd1() { // Thread 1/*@ \label{line:thrd1} @*/
	arr[1].store(47, relaxed);
	q->enqueue(1);
}
void thrd2() { // Thread 2/*@ \label{line:thrd2} @*/
	int idx;
	if (q->dequeue(&idx))
		arr[idx].load(relaxed);
}

int user_main(int argc, char **argv)
{
	thrd_t A, B;
	q = new spsc_queue;
	thrd_create(&A, (thrd_start_t)&thrd1, NULL);
	thrd_create(&B, (thrd_start_t)&thrd2, NULL);
	thrd_join(A);
	thrd_join(B);

	//delete q;

	return 0;
}
