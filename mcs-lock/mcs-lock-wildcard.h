// mcs on stack

#include <stdatomic.h>
#include <unrelacy.h>
#include "wildcard.h" 


struct mcs_node {
	std::atomic<mcs_node *> next;
	std::atomic<int> gate;

	mcs_node() {
		next.store(0);
		gate.store(0);
	}
};

struct mcs_mutex {
public:
	// tail is null when lock is not held
	std::atomic<mcs_node *> m_tail;

	mcs_mutex() {
		m_tail.store( NULL);
	}
	~mcs_mutex() {
		//ASSERT( m_tail.load() == NULL );
	}

	// Each thread will have their own guard.
	class guard {
	public:
		mcs_mutex * m_t;
		mcs_node    m_node; // node held on the stack

		guard(mcs_mutex * t) : m_t(t) { t->lock(this); }
		~guard() { m_t->unlock(this); }
	};

	void lock(guard * I) {
		mcs_node * me = &(I->m_node);

		// set up my node :
		// not published yet so relaxed :
		me->next.store(NULL, wildcard(1) ); // relaxed
		me->gate.store(1, wildcard(2) ); // relaxed

		// publish my node as the new tail :
		mcs_node * pred = m_tail.exchange(me, wildcard(3)); // acq_rel
		if ( pred != NULL ) {
			// (*1) race here
			// unlock of pred can see me in the tail before I fill next

			// publish me to previous lock-holder :
			pred->next.store(me, wildcard(4) ); // release

			// (*2) pred not touched any more       

			// now this is the spin -
			// wait on predecessor setting my flag -
			rl::linear_backoff bo;
			int my_gate = 1;
			while ( (my_gate = me->gate.load(wildcard(5))) ) { // acquire
				thrd_yield();
			}
		}
	}

	void unlock(guard * I) {
		mcs_node * me = &(I->m_node);

		mcs_node * next = me->next.load(wildcard(6)); // acquire
		if ( next == NULL )
		{
			mcs_node * tail_was_me = me;
			bool success;
			// SCFence infers acq_rel / release !!!
			// Could be release if wildcard(8) is acquire
			if ( (success = m_tail.compare_exchange_strong(
				tail_was_me,NULL,wildcard(7))) ) { // acq_rel
				// got null in tail, mutex is unlocked
				return;
			}

			// (*1) catch the race :
			rl::linear_backoff bo;
			for(;;) {
				// SCFence infers relaxed / acquire!!!
				// Could be relaxed if wildcard(7) is acq_rel
				next = me->next.load(wildcard(8)); // acquire
				if ( next != NULL )
					break;
				thrd_yield();
			}
		}

		// (*2) - store to next must be done,
		//  so no locker can be viewing my node any more        

		// let next guy in :
		next->gate.store( 0, wildcard(9) ); // release
	}
};
