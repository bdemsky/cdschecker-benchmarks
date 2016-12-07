#include <stdatomic.h>
#include <threads.h>
#include "seqlock.h" 

seqlock::seqlock() {
	atomic_init(&_seq, 0);
	atomic_init(&_data1, 0);
	atomic_init(&_data2, 0);
}

/** @DeclareState: int data1;
		int data2; */

void seqlock::read(int *data1, int *data2) {
	while (true) {
        // XXX-injection-#1: Weaken the parameter "memory_order_acquire" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./seqlock/testcase1 -m2 -y -u3 -tSPEC"

		/**********    Detected Correctness (testcase1)    **********/
		int old_seq = _seq.load(memory_order_acquire); // acquire
		if (old_seq % 2 == 1) {
            thrd_yield();
            continue;
        }
	
		// Acquire ensures that the second _seq reads an update-to-date value
		*data1 = _data1.load(memory_order_relaxed);
		*data2 = _data2.load(memory_order_relaxed);
		/** @OPClearDefine: true */

        // XXX-injection-#2: Weaken the parameter "memory_order_acquire" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./seqlock/testcase1 -m2 -y -u3 -tSPEC"
		/**********    Detected Correctness (testcase1)    **********/
		atomic_thread_fence(memory_order_acquire);
		if (_seq.load(memory_order_relaxed) == old_seq) { // relaxed
            thrd_yield();
			return;
		}
	}
}


void seqlock::write(int data1, int data2) {
	while (true) {
		// #1: either here or #2 must be acquire
        // XXX-injection-#3: Weaken the parameter "memory_order_acquire" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./seqlock/testcase2 -m2 -y -u3 -x1000 -tSPEC"
		/**********    Detected Correctness (testcase2 with -y -x1000)    **********/
		int old_seq = _seq.load(memory_order_acquire); // acquire
		if (old_seq % 2 == 1) {
            thrd_yield();
			continue; // Retry
        }

		// #2
		if (_seq.compare_exchange_strong(old_seq, old_seq + 1,
			memory_order_relaxed, memory_order_relaxed)) {
            thrd_yield();
			break;
		}
	}

	// Update the data. It needs to be release since this version use
	// relaxed CAS in write(). When the first load of _seq reads the realaxed
	// CAS update, it does not form synchronization, thus requiring the data to
	// be acq/rel. The data in practice can be pointers to objects.

    // XXX-injection-#4: Weaken the parameter "memory_order_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./seqlock/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected Correctness (testcase1)    **********/
	atomic_thread_fence(memory_order_release);
	_data1.store(data1, memory_order_relaxed);
	_data2.store(data2, memory_order_relaxed); 
	/** @OPDefine: true */

    // XXX-injection-#5: Weaken the parameter "memory_order_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./seqlock/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected Correctness (testcase1)    **********/
	_seq.fetch_add(1, memory_order_release); // release
}

/** C Interface */

/** @Transition: STATE(data1) = data1;
		STATE(data2) = data2; */
void write(seqlock *lock, int data1, int data2) {
	lock->write(data1, data2);
}


/** @JustifyingPrecondition: return STATE(data1) == *data1 && STATE(data2) == *data2; */
void read(seqlock *lock, int *data1, int *data2) {
	lock->read(data1, data2);
}
