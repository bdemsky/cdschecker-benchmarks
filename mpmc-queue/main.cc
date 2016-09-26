#include <threads.h>
#include <unistd.h>
#include <librace.h>

#include "mpmc-queue.h"

atomic_int x;

void threadA(struct mpmc_boundq_1_alt<int32_t> *queue)
{
	int32_t *bin = write_prepare(queue);
	//store_32(bin, 1);
	write_publish(queue, bin);
}

void threadB(struct mpmc_boundq_1_alt<int32_t> *queue)
{
	int32_t *bin;
	if ((bin = read_fetch(queue)) != NULL) {
		//printf("Read: %d\n", load_32(bin));
		read_consume(queue, bin);
	}
}

void threadC(struct mpmc_boundq_1_alt<int32_t> *queue)
{
	int32_t *bin = write_prepare(queue);
	//store_32(bin, 1);
	write_publish(queue, bin);

	while ((bin = read_fetch(queue)) != NULL) {
		//printf("Read: %d\n", load_32(bin));
		read_consume(queue, bin);
	}
}


int user_main(int argc, char **argv)
{
	mpmc_boundq_1_alt<int32_t> *queue = createMPMC(16);
	thrd_t t1, t2;
	/** @Entry */
	/*
	int32_t *bin = write_prepare(queue);
	store_32(bin, 1);
	write_publish(queue, bin);
	*/
	printf("Start threads\n");

	thrd_create(&t1, (thrd_start_t)&threadC, queue);
	thrd_create(&t2, (thrd_start_t)&threadC, queue);

	thrd_join(t1);
	thrd_join(t2);

	printf("Threads complete\n");

	//destroyMPMC(queue);
	return 0;
}
