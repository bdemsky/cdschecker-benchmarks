#include <threads.h>
#include "hashmap.h"

HashMap *table;

/** Making w4 & w11 seq_cst */

int k1 = 1;
int k2 = 3;
int v1 = 10;
int v2 = 30;

void threadA(void *arg) {
	int r1 = put(table, k1, v1);
	int r2 = get(table, k2);
}

void threadB(void *arg) {
	int r3 = put(table, k2, v2);
	int r4 = get(table, k1);
}

int user_main(int argc, char *argv[]) {
	
	thrd_t t1, t2;
	/** @Entry */
	table = new HashMap;

	thrd_create(&t1, threadA, NULL);
	thrd_create(&t2, threadB, NULL);
	thrd_join(t1);
	thrd_join(t2);
	
	return 0;
}


