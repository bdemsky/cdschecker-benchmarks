#include <iostream>

#include "cliffc_hashtable.h"

#ifndef NORMAL
#include "threads.h"
#endif

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
slot* const cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::MATCH_ANY = new slot(false, NULL);

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
slot* const cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::NO_MATCH_OLD = new slot(false, NULL);

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
slot* const cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::TOMBPRIME = new slot(true, NULL);

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
slot* const cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::TOMBSTONE = new slot(false, NULL);

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
Hash cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::hashFunc;

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
KeyEqualsTo cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::keyEqualsTo;

template<typename TypeK, typename TypeV, class Hash, class KeyEqualsTo, class ValEqualsTo>
ValEqualsTo cliffc_hashtable<TypeK, TypeV, Hash, KeyEqualsTo, ValEqualsTo>::valEqualsTo;

class HashInt {
	public:
	int operator()(const int &val) const {
		return val;
	}
};

class EqualsToInt {
	public:
	bool operator()(const int &val1, const int &val2) const {
		return val1 == val2;
	}
};

int *k1, *k2, *v1, *v2;
cliffc_hashtable<int, int, HashInt, EqualsToInt, EqualsToInt> *table;

void threadA(void *arg) {
	table->put(*k1, *v1);
	int *r1 = table->get(*k2);
	if (r1) {
		printf("r1=%d\n", *r1);
	} else {
		printf("r1=NULL\n");
	}
}

void threadB(void *arg) {
	table->put(*k2, *v2);
	int *r2 = table->get(*k1);
	if (r2) {
		printf("r2=%d\n", *r2);
	} else {
		printf("r2=NULL\n");
	}
}


#ifdef NORMAL
int main(int argc, char *argv[]) {
	table = new cliffc_hashtable<int, int, HashInt, EqualsToInt, EqualsToInt>();
	k1 = new int(3);
	k2 = new int(4);
	v1 = new int(1);
	v2 = new int(2);
	
	table->put(*k1, *v1);
	table->put(*k2, *v2);
	int *r1 = table->get(*k2);
	if (r1) {
		printf("r1=%d\n", *r1);
	} else {
		printf("r1=NULL\n");
	}

	return 0;
}
#else
int user_main(int argc, char *argv[]) {
	table = new cliffc_hashtable<int, int, HashInt, EqualsToInt, EqualsToInt>();
	k1 = new int(3);
	k2 = new int(4);
	v1 = new int(1);
	v2 = new int(2);

	thrd_t A, B, C;
	thrd_create(&A, &threadA, NULL);
	thrd_create(&B, &threadB, NULL);

	thrd_join(A);
	thrd_join(B);

	return 0;
}
#endif
