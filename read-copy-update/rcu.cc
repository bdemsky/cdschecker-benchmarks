#include "rcu.h"

/**
	This is an example about how to specify the correctness of the
	read-copy-update synchronization mechanism.
*/

atomic<Data*> dataPtr;
	
/** @DeclareState: int data1;
		int data2; */

/** @JustifyingPrecondition: return STATE(data1) == *data1 && STATE(data2) == *data2; */
void read(int *data1, int *data2) {
	/**********    Detected Correctness **********/
	Data *res = dataPtr.load(memory_order_acquire);
	/** @OPDefine: true */
	*data1 = res->data1;
	*data2 = res->data2;
	//load_32(&res->data1);
}

static void inc(Data *newPtr, Data *prev, int d1, int d2) {
	newPtr->data1 = prev->data1 + d1;
	newPtr->data2 = prev->data2 + d2;
}

/** @Transition: STATE(data1) += data1;
		STATE(data2) += data2; */
void write(int data1, int data2) {
	bool succ = false;
	Data *tmp = new Data;
	do {
		/**********    Detected Correctness **********/
		Data *prev = dataPtr.load(memory_order_acquire);
		inc(tmp, prev, data1, data2);
		/**********    Detected Correctness **********/
		succ = dataPtr.compare_exchange_strong(prev, tmp,
            memory_order_release, memory_order_relaxed);
		/** @OPClearDefine: succ */
	} while (!succ);
}
