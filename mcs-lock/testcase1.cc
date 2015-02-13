#include <stdio.h>
#include <threads.h>

#include "mcs-lock-wildcard.h"

/* For data race instrumentation */
#include "librace.h"

struct mcs_mutex *mutex;
static atomic_int shared;

void threadA(void *arg)
{
	mcs_mutex::guard g(mutex);
	printf("store: %d\n", 17);
	//store_32(&shared, 17);
	atomic_store_explicit(&shared, 17, relaxed);
	mutex->unlock(&g);
	mutex->lock(&g);
	//printf("load: %u\n", load_32(&shared));
	atomic_load_explicit(&shared, relaxed);
}

void threadB(void *arg)
{
	mcs_mutex::guard g(mutex);
	//printf("load: %u\n", load_32(&shared));
	atomic_load_explicit(&shared, relaxed);
	mutex->unlock(&g);
	mutex->lock(&g);
	atomic_store_explicit(&shared, 17, relaxed);
	//printf("store: %d\n", 17);
	store_32(&shared, 17);
}

int user_main(int argc, char **argv)
{
	thrd_t A, B;

	mutex = new mcs_mutex();

	thrd_create(&A, &threadA, NULL);
	thrd_create(&B, &threadB, NULL);
	thrd_join(A);
	thrd_join(B);
	return 0;
}
