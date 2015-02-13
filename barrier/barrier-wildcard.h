#include <atomic>
#include "wildcard.h"

class spinning_barrier {
 public:
	spinning_barrier (unsigned int n) : n_ (n) {
		nwait_ = 0;
		step_ = 0;
	}

	// The purpose of wait() is that threads that enter it synchronize with
	// threads when they get out of it.
	/** wildcard(2) is acq_rel, ensuring that all threads hb before other
	 *  threads in the rmw chain order, then the wildcard (4) and (5) are
	 *  release/acquire to make sure the last thread synchronize with all other
	 *  earlier threads. Plus, the (4) and (5) synchronization can make sure the
	 *  reset of nwait_ in wildcard(3) happens-before any other threads in the
	 *  later usage of the barrier.
	*/

	// All orderings are SC originally!!!
	bool wait() {
		// Infer relaxed
		unsigned int step = step_.load (wildcard(1));
		
		// Infer acq_rel 
		if (nwait_.fetch_add (1, wildcard(2)) == n_ - 1) {
			/* OK, last thread to come.  */
			// Infer relaxed 
			nwait_.store (0, wildcard(3)); // XXX: maybe can use relaxed ordering here ??
			// Infer release
			step_.fetch_add (1, wildcard(4));
			return true;
		} else {
			/* Run in circles and scream like a little girl.  */
			// Infer acquire 
			while (step_.load (wildcard(5)) == step)
				thrd_yield();
			return false;
		}
	}

 protected:
	/* Number of synchronized threads. */
	const unsigned int n_;

	/* Number of threads currently spinning.  */
	std::atomic<unsigned int> nwait_;

	/* Number of barrier syncronizations completed so far, 
	 *      * it's OK to wrap.  */
	std::atomic<unsigned int> step_;
};
