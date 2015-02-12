#include <stdatomic.h>
#include <threads.h>

#include "seqlock-wildcard.h"

seqlock_t *lock;

static void a(void *obj) {
	lock->write(3);
}

static void b(void *obj) {
	lock->write(2);
}

static void c(void *obj) {
	int r1 = lock->read();
}

int user_main(int argc, char **argv) {
	thrd_t t1, t2, t3, t4;
	lock = new seqlock_t();

	thrd_create(&t1, (thrd_start_t)&a, NULL);
	thrd_create(&t2, (thrd_start_t)&b, NULL);
	thrd_create(&t3, (thrd_start_t)&c, NULL);

	thrd_join(t1);
	thrd_join(t2);
	thrd_join(t3);
	return 0;
}
