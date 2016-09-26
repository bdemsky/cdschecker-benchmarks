#include "queue.h"

template<typename T>
void spsc_queue<T>::enqueue(T data)
{
	node* n = new node (data);
	/**********    Detected Correctness    **********/
	//head($)->next.store(n, std::memory_order_release);
	head->next.store(n, std::memory_order_release);
	/** @OPDefine: true */
	head = n;
	ec.signal();
}


template<typename T>
T spsc_queue<T>::dequeue()
{
	T data = try_dequeue();
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

template<typename T>
T spsc_queue<T>::try_dequeue()
{
	//node* t = tail($);
	node* t = tail;
	/**********    Detected Correctness    **********/
	node* n = t->next.load(std::memory_order_acquire);
	/** @OPClearDefine: true */
	if (0 == n)
		return 0;
	//T data = n->data($);
	T data = n->data;
	delete (t);
	tail = n;
	return data;
}

/** @DeclareState: IntList *q; */

/** @Transition: STATE(q)->push_back(data); */
void enqueue(spsc_queue<int> *q, int data) {
	q->enqueue(data);
}

/** @PreCondition: return STATE(q)->empty() ? !C_RET : STATE(q)->front() == C_RET;
	@Transition: if (!C_RET) STATE(q)->pop_front(); */
int dequeue(spsc_queue<int> *q) {
	return q->dequeue();
}
