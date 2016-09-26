#include "mcs-lock.h"

void mcs_mutex::lock(guard * I) {
	mcs_node * me = &(I->m_node);

	// set up my node :
	// not published yet so relaxed :
	me->next.store(NULL, std::mo_relaxed );
	me->gate.store(1, std::mo_relaxed );

	/**********  Detected Correctness **********/
	/** Run this in the -Y mode to expose the HB bug */
	// publish my node as the new tail :
	mcs_node * pred = m_tail.exchange(me, std::mo_acq_rel);
	/** @OPDefine: pred == NULL */
	if ( pred != NULL ) {
		// (*1) race here
		// unlock of pred can see me in the tail before I fill next

		// FIXME: detection miss, execution never ends
		// If this is relaxed, the store 0 to gate will be read before and
		// that lock will never ends.
		// publish me to previous lock-holder :
		pred->next.store(me, std::mo_release );

		// (*2) pred not touched any more       

		// now this is the spin -
		// wait on predecessor setting my flag -
		rl::linear_backoff bo;
		/**********  Detected Correctness *********/
	    /** Run this in the -Y mode to expose the HB bug */
		while ( me->gate.load(std::mo_acquire) ) {
			thrd_yield();
		}
		/** @OPDefine: true */
	}
}

void mcs_mutex::unlock(guard * I) {
	mcs_node * me = &(I->m_node);

	// FIXME: detection miss, execution never ends
	mcs_node * next = me->next.load(std::mo_acquire);
	if ( next == NULL )
	{
		mcs_node * tail_was_me = me;
		/**********  Detected Correctness **********/
	    /** Run this in the -Y mode to expose the HB bug */
		if ( m_tail.compare_exchange_strong(
			tail_was_me,NULL,std::mo_acq_rel) ) {
			// got null in tail, mutex is unlocked
			/** @OPDefine: true */
			return;
		}

		// (*1) catch the race :
		rl::linear_backoff bo;
		for(;;) {
			// FIXME: detection miss, execution never ends
			next = me->next.load(std::mo_acquire);
			if ( next != NULL )
				break;
			thrd_yield();
		}
	}

	// (*2) - store to next must be done,
	//  so no locker can be viewing my node any more        

	/**********  Detected Correctness **********/
	/** Run this in the -Y mode to expose the HB bug */
	// let next guy in :
	next->gate.store( 0, std::mo_release);
	/** @OPDefine: true */
}

// C-callable interface

/** @DeclareState: bool lock;
	@Initial: lock = false; */


/** @PreCondition: return STATE(lock) == false;
@Transition: STATE(lock) = true; */
void mcs_lock(mcs_mutex *mutex, CGuard guard) {
	mcs_mutex::guard *myGuard = (mcs_mutex::guard*) guard;
	mutex->lock(myGuard);
}

/** @PreCondition: return STATE(lock) == true;
@Transition: STATE(lock) = false; */
void mcs_unlock(mcs_mutex *mutex, CGuard guard) {
	mcs_mutex::guard *myGuard = (mcs_mutex::guard*) guard;
	mutex->unlock(myGuard);
}
