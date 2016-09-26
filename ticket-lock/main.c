#include <threads.h>
#include <stdatomic.h>

#include "librace.h"
#include "lock.h" 

TicketLock mylock;
int shareddata;

static void a(void *obj)
{
	int i;
	for(i = 0; i < 2; i++) {
		if ((i % 2) == 0) {
			lock(&mylock);
			//load_32(&shareddata);
			unlock(&mylock);
		} else {
			lock(&mylock);
			//store_32(&shareddata,(unsigned int)i);
			unlock(&mylock);
		}
	}
}

int user_main(int argc, char **argv)
{
	thrd_t t1, t2;
	/** @Entry */
	initTicketLock(&mylock);

	thrd_create(&t1, (thrd_start_t)&a, NULL);
	thrd_create(&t2, (thrd_start_t)&a, NULL);

	thrd_join(t1);
	thrd_join(t2);

	return 0;
}
