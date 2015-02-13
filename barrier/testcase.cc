#include <stdio.h>
#include <threads.h>

#include "barrier-wildcard.h"

#include "librace.h"
#include <atomic> 

spinning_barrier *barr;
std::atomic_int var;

void threadA(void *arg)
{
	//store_32(&var, 1);
	var.store(1, std::memory_order_relaxed);
	barr->wait();
}

void threadB(void *arg)
{
	barr->wait();
	//printf("var = %d\n", load_32(&var));
	var.load(std::memory_order_relaxed);
}

#define NUMREADERS 2
int user_main(int argc, char **argv)
{
	thrd_t A, B[NUMREADERS];
	int i;

	barr = new spinning_barrier(NUMREADERS + 1);

	thrd_create(&A, &threadA, NULL);
	for (i = 0; i < NUMREADERS; i++)
		thrd_create(&B[i], &threadB, NULL);

	for (i = 0; i < NUMREADERS; i++)
		thrd_join(B[i]);
	thrd_join(A);

	return 0;
}
