#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include "model-assert.h"

#include "stack.h"

static int procs = 4;
static stack *s;

unsigned int idx1, idx2;
unsigned int a, b;


atomic_int x[3];

static void main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	if (pid % 4 == 0) {
		atomic_store_explicit(&x[1], 17, relaxed);
	} else if (pid % 4 == 1) {
		atomic_store_explicit(&x[2], 37, relaxed);
		s->push(2);
	} else if (pid % 4 == 2) {
		idx1 = s->pop();
		if (idx1 != 0) {
			a = atomic_load_explicit(&x[idx1], relaxed);
		}
	} else {
		idx2 = s->pop();
		if (idx2 != 0) {
			b = atomic_load_explicit(&x[idx2], relaxed);
		}
	}
}

int user_main(int argc, char **argv)
{
	int i;
	int *param;

	atomic_init(&x[1], 0);
	atomic_init(&x[2], 0);

	s = new stack;

	int num_threads = 4;

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
	delete s;
	
	return 0;
}
