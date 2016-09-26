#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

#include "librace.h"
#include "linuxrwlocks.h"

/** Example implementation of linux rw lock along with 2 thread test
 *  driver... */

/**
	Properties to check:
	1. At most 1 thread can acquire the write lock, and at the same time, no
		other threads can acquire any lock (including read/write lock).
	2. At most RW_LOCK_BIAS threads can successfully acquire the read lock.
	3. A read_unlock release 1 read lock, and a write_unlock release the write
	lock. They can not release a lock that they don't acquire.
	###
	4. Read_lock and write_lock can not be grabbed at the same time.
	5. Happpens-before relationship should be checked and guaranteed, which
	should be as the following:
		a. read_unlock hb-> write_lock
		b. write_unlock hb-> write_lock
		c. write_unlock hb-> read_lock
*/

/**
	Interesting point for locks:
	a. If the users unlock() before any lock(), then the model checker will fail.
	For this case, we can not say that the data structure is buggy, how can we
	tell them from a real data structure bug???
	b. We should specify that for a specific thread, successful locks and
	unlocks should always come in pairs. We could make this check as an
	auxiliary check since it is an extra rule for how the interfaces should called.
*/

/** @DeclareState: bool writerLockAcquired;
		int readerLockCnt; */

int read_can_lock(rwlock_t *lock)
{
	return atomic_load_explicit(&lock->lock, memory_order_relaxed) > 0;
}

int write_can_lock(rwlock_t *lock)
{
	return atomic_load_explicit(&lock->lock, memory_order_relaxed) == RW_LOCK_BIAS;
}


/** @PreCondition: return !STATE(writerLockAcquired);
@Transition: STATE(readerLockCnt)++; */
void read_lock(rwlock_t *rw)
{
	
	/**********  Detected Correctness (testcase1) **********/
	int priorvalue = atomic_fetch_sub_explicit(&rw->lock, 1, memory_order_acquire);
	/** @OPDefine: priorvalue > 0 */
	while (priorvalue <= 0) {
		atomic_fetch_add_explicit(&rw->lock, 1, memory_order_relaxed);
		while (atomic_load_explicit(&rw->lock, memory_order_relaxed) <= 0) {
			thrd_yield();
		}
	    /**********  Detected Correctness (testcase1) **********/
		priorvalue = atomic_fetch_sub_explicit(&rw->lock, 1, memory_order_acquire);
		/** @OPDefine: priorvalue > 0 */
	}
}


/** @PreCondition: return !STATE(writerLockAcquired) && STATE(readerLockCnt) == 0;
@Transition: STATE(writerLockAcquired) = true; */
void write_lock(rwlock_t *rw)
{
	/**********  Detected Correctness (testcase1) **********/
	int priorvalue = atomic_fetch_sub_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_acquire);
	/** @OPDefine: priorvalue == RW_LOCK_BIAS */
	while (priorvalue != RW_LOCK_BIAS) {
		atomic_fetch_add_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_relaxed);
		while (atomic_load_explicit(&rw->lock, memory_order_relaxed) != RW_LOCK_BIAS) {
			thrd_yield();
		}
	    /**********  Detected Correctness (testcase1) **********/
		priorvalue = atomic_fetch_sub_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_acquire);
		/** @OPDefine: priorvalue == RW_LOCK_BIAS */
	}
}

/** @PreCondition: return !C_RET || !STATE(writerLockAcquired);
@Transition: if (C_RET) STATE(readerLockCnt)++; */
int read_trylock(rwlock_t *rw)
{
	/**********  Detected Correctness (testcase2) **********/
	int priorvalue = atomic_fetch_sub_explicit(&rw->lock, 1, memory_order_acquire);
	/** @OPDefine: true */
	if (priorvalue > 0) {
		return 1;
	}
	atomic_fetch_add_explicit(&rw->lock, 1, memory_order_relaxed);
	return 0;
}

/** @PreCondition: return !C_RET || !STATE(writerLockAcquired) && STATE(readerLockCnt) == 0;
@Transition: if (C_RET) STATE(writerLockAcquired) = true; */
int write_trylock(rwlock_t *rw)
{
	/**********  Detected Correctness (testcase2) **********/
	int priorvalue = atomic_fetch_sub_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_acquire);
	/** @OPDefine: true */
	if (priorvalue == RW_LOCK_BIAS) {
		return 1;
	}
	atomic_fetch_add_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_relaxed);
	return 0;
}

/** @PreCondition: return STATE(readerLockCnt) > 0 && !STATE(writerLockAcquired);
@Transition: STATE(readerLockCnt)--; */
void read_unlock(rwlock_t *rw)
{
	/**********  Detected Correctness (testcase1) **********/
	atomic_fetch_add_explicit(&rw->lock, 1, memory_order_release);
	/** @OPDefine: true */
}

/** @PreCondition: return STATE(readerLockCnt) == 0 && STATE(writerLockAcquired);
@Transition: STATE(writerLockAcquired) = false; */
void write_unlock(rwlock_t *rw)
{
	/**********  Detected Correctness (testcase1) **********/
	atomic_fetch_add_explicit(&rw->lock, RW_LOCK_BIAS, memory_order_release);
	/** @OPDefine: true */
}
