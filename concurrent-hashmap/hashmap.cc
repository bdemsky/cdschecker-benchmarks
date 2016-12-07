#include "unrelacy.h" 
#include "hashmap.h" 

/** @DeclareState: IntMap *map; */

int HashMap::get(int key) {
	MODEL_ASSERT (key);
	int hash = hashKey(key);

	// Try first without locking...
	atomic<Entry*> *tab = table;
	int index = hash & (capacity - 1);
	atomic<Entry*> *first = &tab[index];
	Entry *e;
	int res = 0;

	// Should be a load acquire
	// This load action here makes it problematic for the SC analysis, what
	// we need to do is as follows: if the get() method ever acquires the
	// lock, we ignore this operation for the SC analysis, and otherwise we
	// take it into consideration
	
    // XXX-injection-#1: Weaken the parameter "mo_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./concurrent-hashmap/testcase2 -m2 -y -u3 -tSPEC"
	/**********    Detected UL (testcase2) **********/
	Entry *firstPtr = first->load(mo_acquire);

	e = firstPtr;
	while (e != NULL) {
		if (e->hash == hash && eq(key, e->key)) {
            // XXX-injection-#2: Weaken the parameter "mo_seqcst" to
            // "memory_order_acq_rel", run "make" to recompile, and then run:
            // "./run.sh ./concurrent-hashmap/testcase1 -m2 -y -u3 -tSPEC"
			/**********    Detected Correctness (testcase1) **********/
			res = e->value.load(mo_seqcst);
			/** @OPClearDefine: res != 0 */
			if (res != 0)
				return res;
			else
				break;
		}
		// Loading the next entry, this can be relaxed because the
		// synchronization has been established by the load of head
		e = e->next.load(mo_relaxed);
	}

	// Recheck under synch if key apparently not there or interference
	Segment *seg = segments[hash & SEGMENT_MASK];
	seg->lock(); // Critical region begins
	/** @OPClearDefine: true */
	// Not considering resize now, so ignore the reload of table...

	// Synchronized by locking, no need to be load acquire
	Entry *newFirstPtr = first->load(mo_relaxed);
	if (e != NULL || firstPtr != newFirstPtr) {
		e = newFirstPtr;
		while (e != NULL) {
			if (e->hash == hash && eq(key, e->key)) {
				// Protected by lock, no need to be SC
				res = e->value.load(mo_relaxed);
				seg->unlock(); // Critical region ends
				return res;
			}
			// Synchronized by locking
			e = e->next.load(mo_relaxed);
		}
	}
	seg->unlock(); // Critical region ends
	return 0;
}

int HashMap::put(int key, int value) {
	// Don't allow NULL key or value
	MODEL_ASSERT (key && value);

	int hash = hashKey(key);
	Segment *seg = segments[hash & SEGMENT_MASK];
	atomic<Entry*> *tab;

	seg->lock(); // Critical region begins
	tab = table;
	int index = hash & (capacity - 1);

	atomic<Entry*> *first = &tab[index];
	Entry *e;
	int oldValue = 0;

	// The written of the entry is synchronized by locking
	Entry *firstPtr = first->load(mo_relaxed);
	e = firstPtr;
	while (e != NULL) {
		if (e->hash == hash && eq(key, e->key)) {
			// FIXME: This could be a relaxed (because locking synchronize
			// with the previous put())??  no need to be acquire
			oldValue = e->value.load(relaxed);
            // XXX-injection-#3: Weaken the parameter "mo_seqcst" to
            // "memory_order_acq_rel", run "make" to recompile, and then run:
            // "./run.sh ./concurrent-hashmap/testcase1 -m2 -y -u3 -tSPEC"
			/**********    Detected Correctness (testcase1) **********/
			e->value.store(value, mo_seqcst);
			/** @OPClearDefine: true */
			seg->unlock(); // Don't forget to unlock before return
			return oldValue;
		}
		// Synchronized by locking
		e = e->next.load(mo_relaxed);
	}

	// Add to front of list
	Entry *newEntry = new Entry();
	newEntry->hash = hash;
	newEntry->key = key;
	newEntry->value.store(value, relaxed);
	/** @OPClearDefine: true */
	newEntry->next.store(firstPtr, relaxed);
    // XXX-injection-#4: Weaken the parameter "mo_release" to
    // "memory_order_acquire", run "make" to recompile, and then run:
    // "./run.sh ./concurrent-hashmap/testcase2 -m2 -y -u3 -tSPEC"
	/**********    Detected UL (testcase2) **********/
	// Publish the newEntry to others
	first->store(newEntry, mo_release);
	seg->unlock(); // Critical region ends
	return 0;
}

/** @PreCondition: return STATE(map)->get(key) == C_RET; */
int get(HashMap *map, int key) {
	return map->get(key);
}

/** @PreCondition: return STATE(map)->get(key) == C_RET;
	@Transition: STATE(map)->put(key, value); */
int put(HashMap *map, int key, int value) {
	return map->put(key, value);
}

