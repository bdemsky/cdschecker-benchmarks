/*
 * Dekker's critical section algorithm, implemented with fences.
 *
 * URL:
 *   http://www.justsoftwaresolutions.co.uk/threading/
 */

#include <atomic>
#include <threads.h>

#include "librace.h"
#include "wildcard.h"

std::atomic<bool> flag0, flag1;
std::atomic<int> turn;

//uint32_t var = 0;
std::atomic_int var1;
std::atomic_int var2;

void p0(void *argv)
{
	int val = *((int*)argv);
	int myTurn = val;
	int otherTurn = 1 - val;
	std::atomic<bool> *myFlag = val == 0 ? &flag0 : &flag1;
	std::atomic<bool> *otherFlag = val == 0 ? &flag1 : &flag0;
	myFlag->store(true, wildcard(1));
	std::atomic_thread_fence(wildcard(2)); // seq_cst

	while (otherFlag->load(wildcard(3)))
	{
		if (turn.load(wildcard(4)) != myTurn)
		{
			myFlag->store(false, wildcard(5));
			while (turn.load(wildcard(6)) != myTurn)
			{
				thrd_yield();
			}
			myFlag->store(true, wildcard(7));
			std::atomic_thread_fence(wildcard(8)); // seq_cst
		} else
			thrd_yield();
	}
	std::atomic_thread_fence(wildcard(9)); // acquire

	// critical section
	//store_32(&var, 1);
	if (val) {
		var1.store(1, std::memory_order_relaxed);
		var2.load(std::memory_order_relaxed);
	} else {
		var2.store(2, std::memory_order_relaxed);
		var1.load(std::memory_order_relaxed);
	}

	turn.store(otherTurn, wildcard(10));
	std::atomic_thread_fence(wildcard(11)); // release
	myFlag->store(false, wildcard(12));
}

void p1(void *arg)
{
	p0(arg);
}

int user_main(int argc, char **argv)
{
	thrd_t a, b;

	flag0 = false;
	flag1 = false;
	turn = 1;
	atomic_init(&var1, 0);
	atomic_init(&var2, 0);

	int thrd1 = 0, thrd2 = 1;
	thrd_create(&a, p0, &thrd1);
	thrd_create(&b, p1, &thrd2);

	thrd_join(a);
	thrd_join(b);

	return 0;
}
