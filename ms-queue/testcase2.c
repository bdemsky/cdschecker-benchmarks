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
unsigned int reclaimNode;

static int procs = 4;
static void main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);
	if (pid % 4 == 0) {
		/*
		atomic_store_explicit(&x[0], 1, memory_order_relaxed);
		enqueue(queue, 0);
		*/
	} else if (pid % 4 == 1) {
		atomic_store_explicit(&x[1], 1, memory_order_relaxed);
		enqueue(queue, 1, false);
	} else if (pid % 4 == 2) {
		succ1 = dequeue(queue, &idx1, &reclaimNode);
		if (succ1) {
			atomic_load_explicit(&x[idx1], memory_order_relaxed);
		}
	} else if (pid % 4 == 3) {
		succ2 = dequeue(queue, &idx2, &reclaimNode);
		if (succ2) {
			atomic_load_explicit(&x[idx2], memory_order_relaxed);
		}
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
