#include <threads.h>

#include "queue.h"

spsc_queue *q;
atomic_int arr[2];

void thread(unsigned thread_index)
{
	if (0 == thread_index)
	{
		arr[1].store(memory_order_relaxed);
		q->enqueue(1);
	}
	else
	{
		int val = 0;
		bool succ = q->dequeue(&val);
		arr[val].load(memory_order_relaxed);
	}
}

int user_main(int argc, char **argv)
{
	thrd_t A, B;

	q = new spsc_queue();

	thrd_create(&A, (thrd_start_t)&thread, (void *)0);
	thrd_create(&B, (thrd_start_t)&thread, (void *)1);
	thrd_join(A);
	thrd_join(B);

	delete q;

	return 0;
}
