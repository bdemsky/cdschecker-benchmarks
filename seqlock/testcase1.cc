#include <threads.h>
#include "seqlock.h"

seqlock *lock;

static void a(void *obj) {
	write(lock, 1, 2);
}

static void b(void *obj) {
	write(lock, 3, 4);
}

static void c(void *obj) {
	int *data1 = new int;
	int *data2 = new int;
	read(lock, data1, data2);
}

int user_main(int argc, char **argv) {
	thrd_t t1, t2, t3;
	/** @Entry */
	lock = new seqlock;

	thrd_create(&t1, (thrd_start_t)&a, NULL);
	//thrd_create(&t2, (thrd_start_t)&b, NULL);
	thrd_create(&t3, (thrd_start_t)&c, NULL);

	thrd_join(t1);
	//thrd_join(t2);
	thrd_join(t3);
	return 0;
}
