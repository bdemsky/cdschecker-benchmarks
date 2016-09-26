// mcs on stack

#ifndef _MCS_LOCK_H
#define _MCS_LOCK_H

#include <stdatomic.h>
#include <threads.h>
#include <unrelacy.h>

// Forward declaration
struct mcs_node;
struct mcs_mutex;

struct mcs_node {
	std::atomic<mcs_node *> next;
	std::atomic<int> gate;

	mcs_node() {
		next.store(0);
		gate.store(0);
	}
};

// C-callable interface
typedef void *CGuard;
void mcs_lock(mcs_mutex *mutex, CGuard guard);

void mcs_unlock(mcs_mutex *mutex, CGuard guard);

struct mcs_mutex {
public:
	// tail is null when lock is not held
	std::atomic<mcs_node *> m_tail;

	mcs_mutex() {
		m_tail.store( NULL );
	}
	~mcs_mutex() {
		RL_ASSERT( m_tail.load() == NULL );
	}

	class guard {
	public:
		mcs_mutex * m_t;
		mcs_node    m_node; // node held on the stack
		
		// Call the wrapper (instrument every lock/unlock)
		guard(mcs_mutex * t) : m_t(t) { mcs_lock(t, this); }
		~guard() { mcs_unlock(m_t, this); }
	};

	void lock(guard * I);

	void unlock(guard * I);
};

#endif
