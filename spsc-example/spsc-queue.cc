#include <threads.h>
#include <atomic>

//#include "queue.h"

using namespace std;
#define acquire memory_order_acquire
#define release memory_order_release
#define relaxed memory_order_relaxed

struct node {
	node(int d): data(d) {
		next.store(0, relaxed);
	}
	atomic<node*> next;
	int data;
};

class spsc_queue { 
	atomic<node*> head, tail;/*@ \label{line:spscBegin} @*/
	public:
	spsc_queue() {
		head = tail = new node(0);
	}
	void enqueue(int val) {
		node* n = new node(val);
		node *h = head.load(relaxed); 
		h->next.store(n, release);/*@ \label{line:spscRelease} @*/
		head.store(n, relaxed);
	}
	bool dequeue(int *v) {
		node* t = tail.load(relaxed);
		node* n = t->next.load(acquire);/*@ \label{line:spscAcquire} @*/
		if (0 == n)
			return false;
		*v = n->data;
		tail.store(n, relaxed);
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
