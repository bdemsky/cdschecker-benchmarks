#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include "queue.h"

static int procs = 3;
Queue *q;

int idx1, idx2;
unsigned int a, b;


atomic_int x[3];

static void main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);
	if (pid % 3 == 0) {
		enq(q, 2);
	} else if (pid % 3 == 1) {
		deq(q);
	} else if (pid % 3 == 2) {
		deq(q);
	}
}

int user_main(int argc, char **argv)
{
	int i;
	int *param;

	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);
	
	/** @Entry */
	q = new Queue;

	int num_threads = 3;

	param = new int[num_threads];
	thrd_t *threads = new thrd_t[num_threads];

	for (i = 0; i < num_threads; i++) {
		param[i] = i;
		thrd_create(&threads[i], main_task, &param[i]);
	}
	for (i = 0; i < num_threads; i++)
		thrd_join(threads[i]);

	delete param;
	delete threads;
	delete q;
	
	return 0;
}
