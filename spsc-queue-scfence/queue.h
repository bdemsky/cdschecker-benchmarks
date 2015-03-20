#include <unrelacy.h>
#include <atomic>

#include "eventcount.h"
#include "wildcard.h"

class spsc_queue
{
public:
	spsc_queue()
	{
		node* n = new node (-1);
		head = n;
		tail = n;
	}

	~spsc_queue()
	{
		RL_ASSERT(head == tail);
		delete ((node*)head($));
	}

	void enqueue(int data)
	{
		node* n = new node (data);
		head($)->next.store(n, wildcard(6));
		head = n;
		ec.signal_relaxed();
	}

	int dequeue()
	{
		int data = try_dequeue();
		while (0 == data)
		{
			int cmp = ec.get();
			data = try_dequeue();
			if (data)
				break;
			ec.wait(cmp);
			data = try_dequeue();
			if (data)
				break;
		}
		return data;
	}

private:
	struct node
	{
		std::atomic<node*> next;
		std::atomic<int> data;

		node(int d)
		{
			data.store(d, memory_order_normal);
			next = 0;
		}
	};

	rl::var<node*> head;
	rl::var<node*> tail;

	eventcount ec;

	int try_dequeue()
	{
		node* t = tail($);
		node* n = t->next.load(wildcard(7));
		if (0 == n)
			return 0;
		int data = n->data.load(memory_order_normal);
		delete (t);
		tail = n;
		return data;
	}
};
