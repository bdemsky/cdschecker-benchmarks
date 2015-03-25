#include <stdlib.h>
#include <stdio.h>
#include <threads.h>

#include "queue.h"
#include "model-assert.h"

static queue_t *queue;
static thrd_t *threads;
static unsigned int *input;
static unsigned int *output;
static int num_threads;

int get_thread_num()
{
	thrd_t curr = thrd_current();
	int i;
	for (i = 0; i < num_threads; i++)
		if (curr.priv == threads[i].priv)
			return i;
	MODEL_ASSERT(0);
	return -1;
}

bool succ1, succ2;
atomic_int x[3];
int idx1, idx2;
unsigned int reclaimNode1, reclaimNode2;

static int procs = 2;


/** This testcase can infer w2->release & w4->acquire.
	The initial node that Head and Tail points to is 1, so when T3 enqueue with
	node 2, and dequeue(get node 1), and enqueue node 1 again, the second time
	it enqueues node 1 it actually first initialize node1->next. At the same
	time in T2, it reads that node1->next (because it reads the old Tail at the
	very beginning), then loads the Tail agian (w4), it can actully reads an old
	value. And this is a bug because if node 1 is again dequeued, then for T2 to
	update node1->next, it can potentially contaminate the memory...
*/

static void main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);
	if (pid % 4 == 0) {
		enqueue(queue, 0, 3);
	} else if (pid % 4 == 1) {
		enqueue(queue, 1, 2);
		succ1 = dequeue(queue, &idx1, &reclaimNode1);
		enqueue(queue, 1, 1);
		//succ1 = dequeue(queue, &idx1, &reclaimNode1);
		//enqueue(queue, 1, false, 2);
	} else if (pid % 4 == 2) {

	} else if (pid % 4 == 3) {
	
	}
}

int user_main(int argc, char **argv)
{
	int i;
	int *param;
	unsigned int in_sum = 0, out_sum = 0;

	queue = calloc(1, sizeof(*queue));
	MODEL_ASSERT(queue);

	num_threads = procs;
	threads = malloc(num_threads * sizeof(thrd_t));
	param = malloc(num_threads * sizeof(*param));
	input = calloc(num_threads, sizeof(*input));
	output = calloc(num_threads, sizeof(*output));

	atomic_init(&x[0], 0);
	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);
	init_queue(queue, num_threads);
	for (i = 0; i < num_threads; i++) {
		param[i] = i;
		thrd_create(&threads[i], main_task, &param[i]);
	}
	for (i = 0; i < num_threads; i++)
		thrd_join(threads[i]);
/*
	for (i = 0; i < num_threads; i++) {
		in_sum += input[i];
		out_sum += output[i];
	}
	for (i = 0; i < num_threads; i++)
		printf("input[%d] = %u\n", i, input[i]);
	for (i = 0; i < num_threads; i++)
		printf("output[%d] = %u\n", i, output[i]);
	if (succ1 && succ2)
		MODEL_ASSERT(in_sum == out_sum);
	else
		MODEL_ASSERT (false);
*/
	free(param);
	free(threads);
	free(queue);

	return 0;
}
