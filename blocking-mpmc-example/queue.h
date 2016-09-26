#ifndef _QUEUE_H
#define _QUEUE_H
#include <atomic>
#include "unrelacy.h"

#define CAS compare_exchange_strong
using namespace std;

typedef struct Node {
	int data;
	atomic<Node*> next;

	Node() {
		data = 0;
		next.store(NULL, relaxed);
	}

	Node(int d) {
		data = d;
		next.store(NULL, relaxed);
	}
} Node;

class Queue {
public: atomic<Node*> tail, head;
Queue() { tail = head = new Node(); }
void enq(int val);
int deq();
};

// Make them C-callable interfaces
void enq(Queue *q, int val);

int deq(Queue *s);

#endif
