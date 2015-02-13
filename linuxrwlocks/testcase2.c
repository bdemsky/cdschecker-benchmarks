#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>
#include "wildcard.h" 

#include "librace.h"
#include "linuxrwlocks-wildcard.h"

rwlock_t mylock;
int shareddata;

atomic_int x, y;

static void a(void *obj)
{
	if (!write_can_lock(&mylock))
		return;

	if (write_trylock(&mylock)) {
		atomic_store_explicit(&x, 17, relaxed);
		write_unlock(&mylock);
	}
}

static void b(void *obj)
{
	if (write_trylock(&mylock)) {
		atomic_store_explicit(&x, 16, relaxed);
		write_unlock(&mylock);
	}

	if (!read_can_lock(&mylock))
		return;
	if (read_trylock(&mylock)) {
		atomic_load_explicit(&x, relaxed);
		read_unlock(&mylock);
	}
}

int user_main(int argc, char **argv)
{
	thrd_t t1, t2;
	atomic_init(&mylock.lock, RW_LOCK_BIAS);
	atomic_init(&x, 0);
	atomic_init(&y, 0);

	thrd_create(&t1, (thrd_start_t)&a, NULL);
	thrd_create(&t2, (thrd_start_t)&b, NULL);

	thrd_join(t1);
	thrd_join(t2);

	return 0;
}
