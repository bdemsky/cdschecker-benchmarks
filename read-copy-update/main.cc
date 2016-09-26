#include <threads.h>
#include "rcu.h"

void threadA(void *arg) {
	write(1, 0);
}

void threadB(void *arg) {
	write(0, 2);
}

void threadC(void *arg) {
	write(2, 2);
}

void threadD(void *arg) {
	int *d1 = new int;
	int *d2 = new int;
	read(d1, d2);
	printf("ThreadD: d1=%d, d2=%d\n", *d1, *d2);
}

int user_main(int argc, char **argv) {
	thrd_t t1, t2, t3, t4;
	/** @Entry */
	Data *dataInit = new Data;
	dataInit->data1 = 0;
	dataInit->data2 = 0;
	atomic_init(&dataPtr, dataInit);

	thrd_create(&t1, threadA, NULL);
	thrd_create(&t2, threadB, NULL);
	//thrd_create(&t3, threadC, NULL);
	thrd_create(&t4, threadD, NULL);

	thrd_join(t1);
	thrd_join(t2);
	//thrd_join(t3);
	thrd_join(t4);

	return 0;
}
