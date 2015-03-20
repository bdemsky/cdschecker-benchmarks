#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

#include "model-assert.h"

#include "deque.h"

Deque *q;
int a;
int b;
int c;

atomic_int x[2];

/**
	Synchronization between plain push and steal (making w31 release and w35
	acquire)
*/

static void task(void * param) {
	a=steal(q);
	printf("a=%d\n", a);
	if (a != EMPTY && a != ABORT)
		atomic_load_explicit(&x[a], memory_order_relaxed);
}

int user_main(int argc, char **argv)
{
	thrd_t t;
	atomic_store_explicit(&x[0], 0,  memory_order_relaxed);
	atomic_store_explicit(&x[1], 0,  memory_order_relaxed);
	q=create_size(4);
	thrd_create(&t, task, 0);
	//atomic_store_explicit(&x[1], 37,  memory_order_relaxed);
	push(q, 1);
	//push(q, 4);
	//b=take(q);
	//c=take(q);
	thrd_join(t);

/*
	bool correct=true;
	if (a!=1 && a!=2 && a!=4 && a!= EMPTY)
		correct=false;
	if (b!=1 && b!=2 && b!=4 && b!= EMPTY)
		correct=false;
	if (c!=1 && c!=2 && c!=4 && a!= EMPTY)
		correct=false;
	if (a!=EMPTY && b!=EMPTY && c!=EMPTY && (a+b+c)!=7)
		correct=false;
	//if (!correct)
		printf("a=%d b=%d c=%d\n",a,b,c);
	MODEL_ASSERT(correct);
	*/

	return 0;
}
