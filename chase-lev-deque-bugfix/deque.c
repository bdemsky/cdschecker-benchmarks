#include "deque.h"
#include <stdlib.h>
#include <stdio.h>

/** @Define: 
    bool succ(int res) { return res != EMPTY && res != ABORT; } 
	bool fail(int res) { return !succ(res); } */

/** @DeclareState: IntList *deque;
	@Initial: deque = new IntList;
	@Commutativity: push <-> push (true)
	@Commutativity: push <-> take (true)
	@Commutativity: take <-> take (true) */

Deque * create_size(int size) {
	Deque * q = (Deque *) calloc(1, sizeof(Deque));
	Array * a = (Array *) calloc(1, sizeof(Array)+size*sizeof(atomic_int));
	atomic_store_explicit(&q->array, a, memory_order_relaxed);
	atomic_store_explicit(&q->top, 0, memory_order_relaxed);
	atomic_store_explicit(&q->bottom, 0, memory_order_relaxed);
	atomic_store_explicit(&a->size, size, memory_order_relaxed);
	return q;
}

Deque * create() {
	Deque * q = (Deque *) calloc(1, sizeof(Deque));
	Array * a = (Array *) calloc(1, sizeof(Array)+2*sizeof(atomic_int));

    /**
     * We specifically create the space of the new array in this initialization
     * method just to make up the fact that the new array can potentially be
     * used by other things or contains just junk data. Please see Section 6.4.1
     * for more detail.
     */
    /********** BEGIN **********/
    int new_size = 8;
    Array *new_a = (Array *) calloc(1, new_size * sizeof(atomic_int) + sizeof(Array));
    atomic_store_explicit(&new_a->size, new_size, memory_order_relaxed);
    int i;
    for(i=0; i < new_size; i++) {
        atomic_store_explicit(&new_a->buffer[i % new_size], 0, memory_order_relaxed);
    }
    q->newArray = new_a;
    /********** END **********/

	atomic_store_explicit(&q->array, a, memory_order_relaxed);
	atomic_store_explicit(&q->top, 0, memory_order_relaxed);
	atomic_store_explicit(&q->bottom, 0, memory_order_relaxed);
	atomic_store_explicit(&a->size, 2, memory_order_relaxed);
	return q;
}

/**
	Note:
	1. The expected way to use the deque is that we have a main thread where we
	call push() and take(); and then we have other stealing threads that only
	call steal().
	2. Bottom stores the index that push() is ready to update on; Top stores the
	index that steal() is ready to read from.
	3. take() greedly decreases the bottom, and then later check whether it is
	going to take the last element; If so, it will race with the corresponding
	stealing threads.
	4. In this implementation, there are two places where we update the Top: a)
	take() racing the last element and steal() consumes an element. We need to
	have seq_cst for all the updates because we need to have a total SC order
	between them such that the SC fences in take() and steal() can prevent the
	load of Top right after the fence in take() will read the update-to-date
	value.
	5. Note that the steal() really can bail any time since it never retries!!!

*/


/** @PreCondition: return succ(C_RET) ? !STATE(deque)->empty() &&
		STATE(deque)->back() == C_RET : true;
	@JustifyingPrecondition: if (!succ(C_RET) && !STATE(deque)->empty()) {
		// Make sure there are concurrent stealers who take those items
		ForEach (item, STATE(deque))
			if (!HasItem(CONCURRENT, Guard(EQ(NAME, "steal") && C_RET(steal) == item)))
				return false;
	}
    @Transition: if (succ(C_RET)) {
		if (STATE(deque)->empty()) return false;
	    STATE(deque)->pop_back();
	} */
int take(Deque *q) {
	// take() greedly decrease the Bottom, then check later
	size_t b = atomic_load_explicit(&q->bottom, memory_order_relaxed) - 1;
	Array *a = (Array *) atomic_load_explicit(&q->array, memory_order_relaxed);
	atomic_store_explicit(&q->bottom, b, memory_order_relaxed);

    // XXX-injection-#1: Weaken the parameter "memory_order_seq_cst" to
    // "memory_order_acq_rel", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase2 -m2 -y -u3 -tSPEC"
	/**********    Detected Correctness (testcase2)    **********/
	atomic_thread_fence(memory_order_seq_cst);
	size_t t = atomic_load_explicit(&q->top, memory_order_relaxed);
	int x;
	if (t <= b) {
		/* Non-empty queue. */
		int sz = atomic_load_explicit(&a->size,memory_order_relaxed);
		// Reads the buffer value before racing
		x = atomic_load_explicit(&a->buffer[b % sz], memory_order_relaxed);
		if (t == b) {
			/* Single last element in queue. */
			// XXX-overly-strong-#1: This was confirmed by the original authors
            // that the first parameter doesn't have to be memory_order_seq_cst
            // (which was the original one in the PPoPP'13 paper).
			if (!atomic_compare_exchange_strong_explicit(&q->top, &t, t + 1,
				memory_order_relaxed, memory_order_relaxed)) {
				/* Failed race. */
				x = EMPTY;
			}
			// Restore the Bottom
			atomic_store_explicit(&q->bottom, b + 1, memory_order_relaxed);
		}
	} else { /* Empty queue. */
		x = EMPTY;
		// Restore the Bottom
		atomic_store_explicit(&q->bottom, b + 1, memory_order_relaxed);
	}
	// Make sure we have one ordering point (push <-> take) when it's empty
	/** @OPClearDefine: true */
	return x;
}

void resize(Deque *q) {
	Array *a = (Array *) atomic_load_explicit(&q->array, memory_order_relaxed);
	size_t size=atomic_load_explicit(&a->size, memory_order_relaxed);
	size_t new_size=size << 1;

    // Suppose the allocation returns a pack of memroy that was used by
    // something else before.
    // Array *new_a = (Array *) calloc(1, new_size * sizeof(atomic_int) + sizeof(Array)); // This is the original line
    Array *new_a = (Array *) q->newArray;

	size_t top=atomic_load_explicit(&q->top, memory_order_relaxed);
	size_t bottom=atomic_load_explicit(&q->bottom, memory_order_relaxed);
	atomic_store_explicit(&new_a->size, new_size, memory_order_relaxed);
	size_t i;
	for(i=top; i < bottom; i++) {
		atomic_store_explicit(&new_a->buffer[i % new_size], atomic_load_explicit(&a->buffer[i % size], memory_order_relaxed), memory_order_relaxed);
	}
	
    // XXX-injection-#2: Weaken the parameter "memory_order_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected UL    **********/
	atomic_store_explicit(&q->array, new_a, memory_order_release);
	printf("resize\n");
}

/** @Transition: STATE(deque)->push_back(x); */
void push(Deque *q, int x) {
	size_t b = atomic_load_explicit(&q->bottom, memory_order_relaxed);
    // XXX-injection-#3: Weaken the parameter "memory_order_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase1 -x1000 -m2 -y -u3 -tSPEC"
	/**********    Detected Correctness (testcase1 -x1000)    **********/
	size_t t = atomic_load_explicit(&q->top, memory_order_acquire);
	Array *a = (Array *) atomic_load_explicit(&q->array, memory_order_relaxed);
	if (b - t > atomic_load_explicit(&a->size, memory_order_relaxed) - 1) /* Full queue. */ {
		resize(q);
		/**********    Also Detected (testcase1)   **********/
		//Bug in paper...should have next line...
		a = (Array *) atomic_load_explicit(&q->array, memory_order_relaxed);
	}
	// Update the buffer (this is the ordering point)
	atomic_store_explicit(&a->buffer[b % atomic_load_explicit(&a->size, memory_order_relaxed)], x, memory_order_relaxed);
	/** @OPDefine: true */

    // XXX-injection-#4: Weaken the parameter "memory_order_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected UL (testcase1)    **********/
	atomic_thread_fence(memory_order_release);
	atomic_store_explicit(&q->bottom, b + 1, memory_order_relaxed);
}

/**  @PreCondition: return succ(C_RET) ? !STATE(deque)->empty() &&
	STATE(deque)->front() == C_RET : true;
    @Transition: if (succ(C_RET)) {
		if (STATE(deque)->empty()) return false;
	    STATE(deque)->pop_front();
	} */
int steal(Deque *q) {
    // XXX-overly-strong-#2: This was found by AutoMO and was confirmed by the
    // original authors that the parameter doesn't have to be
    // memory_order_acquire (which was the original one in the PPoPP'13 paper).
    // The reason is that this load is followed by an SC fence (discussed in
    // AutoMO).
	size_t t = atomic_load_explicit(&q->top, memory_order_relaxed);

    // XXX-injection-#5: Weaken the parameter "memory_order_seq_cst" to
    // "memory_order_acq_rel", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase3 -m2 -y -u3 -x200 -tSPEC"
	/**********    Detected Correctness (testcase3)    **********/
	atomic_thread_fence(memory_order_seq_cst);

    // XXX-injection-#6: Weaken the parameter "memory_order_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./chase-lev-deque-bugfix/testcase1 -x100 -m2 -y -u3 -tSPEC"
	/**********    Detected UL (testcase1 -x100)    **********/
	size_t b = atomic_load_explicit(&q->bottom, memory_order_acquire);
	int x = EMPTY;
	if (t < b) {
		/* Non-empty queue. */
        // XXX-known-bug-#1: To reproduce the bug, weaken the parameter
        // "memory_order_acquire" to "memory_order_relaxed", run "make" to
        // recompile, and then run:
        // "./run.sh ./chase-lev-deque-bugfix/testcase1 -x100 -m2 -y -u3 -tSPEC"
		/**********    Detected Correctness after initializing the array (testcase1 -x100)    **********/
		Array *a = (Array *) atomic_load_explicit(&q->array, memory_order_acquire);
		int sz = atomic_load_explicit(&a->size, memory_order_relaxed);
		x = atomic_load_explicit(&a->buffer[t % sz], memory_order_relaxed);
        // XXX-injection-#7: Weaken the parameter "memory_order_seq_cst" to
        // "memory_order_acq_rel", run "make" to recompile, and then run:
        // "./run.sh ./chase-lev-deque-bugfix/testcase3 -x500 -m2 -y -u3 -tSPEC"
		/**********    Detected Correctness (testcase3 -x500)    **********/
		bool succ = atomic_compare_exchange_strong_explicit(&q->top, &t, t + 1,
			memory_order_seq_cst, memory_order_relaxed);
		/** @OPDefine: true */
		if (!succ) {
			/* Failed race. */
			return ABORT;
		}
	}
	return x;
}
