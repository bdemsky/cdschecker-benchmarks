#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

#include "librace.h"
#include "linuxrwlocks.h"

rwlock_t mylock;
int shareddata;

atomic_int x, y;

static void a(void *obj)
{
	int i;
	for(i = 0; i < 2; i++) {
		if ((i % 2) == 0) {
			read_lock(&mylock);
			load_32(&shareddata);
			read_unlock(&mylock);
		} else {
			write_lock(&mylock);
			store_32(&shareddata,(unsigned int)i);
			write_unlock(&mylock);
		}
	}
}


int user_main(int argc, char **argv)
{
	thrd_t t1, t2;
	atomic_init(&mylock.lock, RW_LOCK_BIAS);
	atomic_init(&x, 0);
	atomic_init(&y, 0);

	/** @Entry */
	thrd_create(&t1, (thrd_start_t)&a, NULL);
	thrd_create(&t2, (thrd_start_t)&a, NULL);

	thrd_join(t1);
	thrd_join(t2);

	return 0;
}
