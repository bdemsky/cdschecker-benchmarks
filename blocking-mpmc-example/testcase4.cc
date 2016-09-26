#include <stdlib.h>
#include <stdio.h>
#include <threads.h>

#include "queue.h"
#include "model-assert.h"

static Queue *queue;
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
	//MODEL_ASSERT(0);
	return -1;
}

bool succ1, succ2;
atomic_int x[3];
int idx1, idx2, idx3;
unsigned int reclaimNode;

static int procs = 2;
static void main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);
	if (pid % procs == 0) {
		enq(queue, 1);
		succ1 = deq(queue);
		enq(queue, 2);
	} else if (pid % procs == 1) {
		enq(queue, 2);
		succ2 = deq(queue);
	}
}

int user_main(int argc, char **argv)
{
	int i;
	int *param;
	unsigned int in_sum = 0, out_sum = 0;

	/** @Entry */
	queue = new Queue;
	MODEL_ASSERT(queue);

	num_threads = procs;
	threads = (thrd_t*) malloc(num_threads * sizeof(thrd_t));
	param = (int*) malloc(num_threads * sizeof(*param));
	input = (unsigned int *) calloc(num_threads, sizeof(*input));
	output = (unsigned int *) calloc(num_threads, sizeof(*output));

	atomic_init(&x[0], 0);
	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);
	queue = new Queue;
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

	return 0;
}
