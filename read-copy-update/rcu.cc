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
    // XXX-injection-#1: Weaken the parameter "memory_order_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./read-copy-update/testcase -m2 -y -u3 -tSPEC"
	/**********    Detected Correctness **********/
	Data *res = dataPtr.load(memory_order_acquire);
	/** @OPDefine: true */
	*data1 = res->data1.load(memory_order_relaxed);
	*data2 = res->data2.load(memory_order_relaxed);
	//load_32(&res->data1);
}

static void inc(Data *newPtr, Data *prev, int d1, int d2) {
	newPtr->data1.store(prev->data1.load(memory_order_relaxed) + d1, memory_order_relaxed);
	newPtr->data2.store(prev->data2.load(memory_order_relaxed) + d2, memory_order_relaxed);
}

/** @Transition: STATE(data1) += data1;
		STATE(data2) += data2; */
void write(int data1, int data2) {
	bool succ = false;
	Data *tmp = new Data;
	do {
        // XXX-injection-#2: Weaken the parameter "memory_order_acquire" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./read-copy-update/testcase -m2 -y -u3 -tSPEC"
		/**********    Detected Correctness **********/
		Data *prev = dataPtr.load(memory_order_acquire);
		inc(tmp, prev, data1, data2);

        // XXX-injection-#3: Weaken the parameter "memory_order_release" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./read-copy-update/testcase -m2 -y -u3 -tSPEC"
		/**********    Detected Correctness **********/
		succ = dataPtr.compare_exchange_strong(prev, tmp,
            memory_order_release, memory_order_relaxed);
		/** @OPClearDefine: succ */
	} while (!succ);
}
