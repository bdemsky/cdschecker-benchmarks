#include <threads.h>
#include <atomic>

//#include "wildcard.h" 
using namespace std;

#define acquire memory_order_acquire
#define release memory_order_release
#define relaxed memory_order_relaxed

struct node {
	node(int idx) {
		index.store(idx, relaxed);
		next.store(NULL, relaxed);
	}
	atomic<node*> next;/*@ \label{line:spscNodeNext} @*/
	atomic<int> index;/*@ \label{line:spscNodeIndex} @*/
};/*@ \label{line:spscNodeEnd} @*/

class spsc_queue {/*@ \label{line:spscBegin} @*/
	node *head, *tail;
	public:
	spsc_queue() {
		head = tail = new node(-1);
	}
	void enqueue(int idx) {
		node* n = new node(idx);
		tail->next.store(n, relaxed); // Should be release/*@ \label{line:spscRelease} @*/
		tail = n;
	}
	bool dequeue(int *idx) {
		node *tmp = head;
		node *n = tmp->next.load(relaxed); // Should be acquire/*@ \label{line:spscAcquire} @*/
		if (NULL == n) return false;
		head = n;
		*idx = n->index.load(relaxed);
		delete (tmp);
		return true;
	}
};/*@ \label{line:spscEnd} @*/

spsc_queue *q;/*@ \label{line:testcaseBegin} @*/
atomic_int arr[2];
void thrd1() { // Thread 1/*@ \label{line:thrd1} @*/
	arr[1].store(47, relaxed);/*@ \label{line:spscRelaxedStore} @*/
	q->enqueue(1);
}
void thrd2() { // Thread 2/*@ \label{line:thrd2} @*/
	int idx;
	if (q->dequeue(&idx))
		arr[idx].load(relaxed);/*@ \label{line:spscRelaxedLoad} @*/
}/*@ \label{line:testcaseEnd} @*/

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
