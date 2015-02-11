#include <iostream>
#include <threads.h>
#include "hashmap_wildcard.h"

HashMap *table;

Key *k1, *k2;
Value *r1, *r2, *r3, *r4, *v1, *v2;

void printKey(Key *key) {
	if (key)
		printf("pos = (%d, %d, %d)\n", key->x, key->y, key->z);
	else
		printf("pos = NULL\n");
}

void printValue(Value *value) {
	if (value)
		printf("velocity = (%d, %d, %d)\n", value->vX, value->vY, value->vZ);
	else
		printf("velocity = NULL\n");
}

void threadA(void *arg) {
	k1 = new Key(1, 1, 1);
	k2 = new Key(3, 4, 5);
	v1 = new Value(10, 10, 10);
	r1 = table->put(k1, v1);
	//printValue(r1);
	r2 = table->get(k2);
	printf("Thrd A:\n");
	printValue(r2);
}

void threadB(void *arg) {
	k1 = new Key(1, 1, 1);
	k2 = new Key(3, 4, 5);
	v2 = new Value(30, 40, 50);
	r3 = table->put(k2, v2);
	//printValue(r3);
	r4 = table->get(k1);
	printf("Thrd B:\n");
	printValue(r4);
}

int user_main(int argc, char *argv[]) {
	thrd_t t1, t2;
	table = new HashMap;

	thrd_create(&t1, threadA, NULL);
	thrd_create(&t2, threadB, NULL);
	thrd_join(t1);
	thrd_join(t2);
	
	return 0;
}


