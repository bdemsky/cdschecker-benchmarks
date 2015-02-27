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
std::atomic_int var;

void p0(void *arg)
{
	flag0.store(true, wildcard(1));
	std::atomic_thread_fence(wildcard(2)); // seq_cst

	while (flag1.load(wildcard(3)))
	{
		if (turn.load(wildcard(4)) != 0)
		{
			flag0.store(false, wildcard(5));
			while (turn.load(wildcard(6)) != 0)
			{
				thrd_yield();
			}
			flag0.store(true, wildcard(7));
			std::atomic_thread_fence(wildcard(8)); // seq_cst
		} else
			thrd_yield();
	}
	std::atomic_thread_fence(wildcard(9)); // acquire

	// critical section
	//store_32(&var, 1);
	var.store(1, std::memory_order_relaxed);

	turn.store(1, wildcard(10));
	std::atomic_thread_fence(wildcard(11)); // release
	flag0.store(false, wildcard(12));
}

void p1(void *arg)
{
	flag1.store(true, wildcard(13));
	std::atomic_thread_fence(wildcard(14)); // seq_cst

	while (flag0.load(wildcard(15)))
	{
		if (turn.load(wildcard(16)) != 1)
		{
			flag1.store(false, wildcard(17));
			while (turn.load(wildcard(18)) != 1)
			{
				thrd_yield();
			}
			flag1.store(true, wildcard(19));
			std::atomic_thread_fence(wildcard(20)); // seq_cst
		} else
			thrd_yield();
	}
	std::atomic_thread_fence(wildcard(21)); // acquire

	// critical section
	//store_32(&var, 2);
	var.store(2, std::memory_order_relaxed);

	turn.store(0, wildcard(22));
	std::atomic_thread_fence(wildcard(23)); // release
	flag1.store(false, wildcard(24));
}

int user_main(int argc, char **argv)
{
	thrd_t a, b;

	flag0 = false;
	flag1 = false;
	turn = 0;
	atomic_init(&var, 0);

	thrd_create(&a, p0, NULL);
	thrd_create(&b, p1, NULL);

	thrd_join(a);
	thrd_join(b);

	return 0;
}
