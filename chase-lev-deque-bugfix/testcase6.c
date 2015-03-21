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

/** Making w15 release */

static void task1(void * param) {
	b=steal(q);
	//c=steal(q);
}

static void task2(void * param) {
	b=steal(q);
	//c=steal(q);
}

int user_main(int argc, char **argv)
{
	thrd_t t1, t2;
	q=create();

	push(q, 1);
	thrd_create(&t1, task1, 0);
	thrd_create(&t2, task2, 0);
	a=take(q);
	push(q, 2);
	c=take(q);
	//push(q, 2);
	//push(q, 3);
	thrd_join(t1);
	thrd_join(t2);

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
