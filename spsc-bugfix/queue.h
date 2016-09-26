#ifndef _SPSC_QUEUE_H
#define _SPSC_QUEUE_H

#include <unrelacy.h>
#include <atomic>

#include "eventcount.h"

template<typename T>
class spsc_queue
{
public:
	spsc_queue()
	{
		node* n = new node ();
		head = n;
		tail = n;
	}

	~spsc_queue()
	{
		RL_ASSERT(head == tail);
		//delete ((node*)head($));
		delete ((node*)head);
	}

	void enqueue(T data);

	T dequeue();

private:
	struct node
	{
		std::atomic<node*> next;
		//rl::var<T> data;
		T data;

		node(T data = T())
			: data(data)
		{
			next = 0;
		}
	};

	/* Use normal memory access
	rl::var<node*> head;
	rl::var<node*> tail;
	*/
	node *head;
	node *tail;

	eventcount ec;

	T try_dequeue();
};

// C Interface
void enqueue(spsc_queue<int> *q, int data);
int dequeue(spsc_queue<int> *q);

#endif
