#include <stdatomic.h>

#include "librace.h"
#include "lock.h" 

/**
	This ticket lock implementation is derived from the original Mellor-Crummey
	& Scott paper <Algorithms for Scalable Synchronization on
		SharedMemory Multiprocessors> in 1991.
	It assumes that the ticket and turn counter are large enough to accommodate
	the maximum number of simultaneous requests for the lock.
*/

/** @DeclareState: bool lock; */


void initTicketLock(TicketLock *l) {
	atomic_init(&l->ticket, 0);
	atomic_init(&l->turn, 0);
}

/** @PreCondition: return STATE(lock) == false;
@Transition: STATE(lock) = true; */
void lock(struct TicketLock *l) {
	// First grab a ticket
	unsigned ticket = atomic_fetch_add_explicit(&l->ticket, 1,
		memory_order_relaxed);
	// Spinning for my turn
	while (true) {
        // XXX-injection-#1: Weaken the parameter "memory_order_acquire" to
        // "memory_order_relaxed", run "make" to recompile, and then run:
        // "./run.sh ./ticket-lock/testcase1 -m2 -Y -u3 -tSPEC"
		/**********    Detected Correctness (testcase1 with -Y)    **********/
		unsigned turn = atomic_load_explicit(&l->turn, memory_order_acquire);
		/** @OPDefine: turn == ticket */
		if (turn == ticket) { // Now it's my turn 
			return;
		} else {
			thrd_yield(); // Added for CDSChecker
		}
	}
}

/** @PreCondition: return STATE(lock) == true;
@Transition: STATE(lock) = false; */
void unlock(struct TicketLock *l) {
	unsigned turn = atomic_load_explicit(&l->turn, memory_order_relaxed);
    // XXX-injection-#2: Weaken the parameter "memory_order_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./ticket-lock/testcase1 -m2 -Y -u3 -tSPEC"
	/**********    Detected Correctness (testcase1 with -Y)    **********/
	atomic_store_explicit(&l->turn, turn + 1, memory_order_release);
	/** @OPDefine: true */
}
