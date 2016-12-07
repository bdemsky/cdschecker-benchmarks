#include <threads.h> 
#include "mpmc-queue.h"

template <typename t_element>
t_element * mpmc_boundq_1_alt<t_element>::read_fetch() {
	// We can have a relaxed for sure here since the next CAS
	// will fix the problem
	unsigned int rdwr = m_rdwr.load(mo_acquire);
	unsigned int rd,wr;
	for(;;) {
		rd = (rdwr>>16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if ( wr == rd ) // empty
			return NULL;
 
         // XXX-injection-#1: Weaken the parameter "mo_acq_rel" to
        // "memory_order_release", run "make" to recompile, and then run:
        // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"

        // XXX-injection-#2: Weaken the parameter "mo_acq_rel" to
        // "memory_order_acquire", run "make" to recompile, and then run:
        // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"

        // We missed both injections (#1 & #2). For the reason, you can see the
        // discussion on MPMC queue in the last paragraph in Section 6.4.2 of
        // our paper. Note that we have more injections than the original
        // submission since we directly weakened mo_acq_rel to mo_relaxed.
        // Reviews suggest us to do a partial relaxations, i.e., mo_acq_rel ->
        // mo_acquire and mo_acq_rel -> mo_release.
		if ( m_rdwr.compare_exchange_weak(rdwr,rdwr+(1<<16),mo_acq_rel) )
			break;
		else
			thrd_yield();
	}

	// (*1)
	rl::backoff bo;
    // XXX-injection-#3: Weaken the parameter "mo_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./mpmc-queue/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected Admissibility (testcase1)    **********/
	while ( (m_written.load(mo_acquire) & 0xFFFF) != wr ) {
		thrd_yield();
	}
	/** @OPDefine: true */

	t_element * p = & ( m_array[ rd % t_size ] );

	return p;
}


template <typename t_element>
void mpmc_boundq_1_alt<t_element>::read_consume(t_element *bin) {
    // XXX-injection-#4: Weaken the parameter "mo_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"
	/**********    Detected Admissibility (testcase2)    **********/
	m_read.fetch_add(1,mo_release);
	/** @OPDefine: true */
}


template <typename t_element>
t_element * mpmc_boundq_1_alt<t_element>::write_prepare() {
	// We can have a relaxed for sure here since the next CAS
	// will fix the problem
	unsigned int rdwr = m_rdwr.load(mo_acquire);
	unsigned int rd,wr;
	for(;;) {
		rd = (rdwr>>16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if ( wr == ((rd + t_size)&0xFFFF) ) // full
			return NULL;

        // XXX-injection-#5: Weaken the parameter "mo_acq_rel" to
        // "memory_order_release", run "make" to recompile, and then run:
        // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"

        // XXX-injection-#6: Weaken the parameter "mo_acq_rel" to
        // "memory_order_acquire", run "make" to recompile, and then run:
        // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"

        // We missed both injections (#5 & #6). For the reason, you can see the
        // discussion on MPMC queue in the last paragraph in Section 6.4.2 of
        // our paper. Note that we have more injections than the original
        // submission since we directly weakened mo_acq_rel to mo_relaxed.
        // Reviews suggest us to do a partial relaxations, i.e., mo_acq_rel ->
        // mo_acquire and mo_acq_rel -> mo_release.
		if ( m_rdwr.compare_exchange_weak(rdwr,(rd<<16) | ((wr+1)&0xFFFF),mo_acq_rel) )
			break;
		else
			thrd_yield();
	}

	// (*1)
	rl::backoff bo;
    // XXX-injection-#7: Weaken the parameter "mo_acquire" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./mpmc-queue/testcase2 -m2 -Y -u3 -tSPEC"
	/**********    Detected Admissibility (testcase2)    **********/
    // run with -Y
	while ( (m_read.load(mo_acquire) & 0xFFFF) != rd ) {
		thrd_yield();
	}
	/** @OPDefine: true */

	t_element * p = & ( m_array[ wr % t_size ] );

	return p;
}

template <typename t_element>
void mpmc_boundq_1_alt<t_element>::write_publish(t_element *bin)
{
    // XXX-injection-#8: Weaken the parameter "mo_release" to
    // "memory_order_relaxed", run "make" to recompile, and then run:
    // "./run.sh ./mpmc-queue/testcase1 -m2 -y -u3 -tSPEC"
	/**********    Detected Admissibility (testcase1)    **********/
	m_written.fetch_add(1,mo_release);
	/** @OPDefine: true */
}

/** @DeclareState: 
	@Commutativity: write_prepare <-> write_publish (M1->C_RET == M2->bin)
	@Commutativity: write_publish <-> read_fetch (M1->bin == M2->C_RET)
	@Commutativity: read_fetch <-> read_consume (M1->C_RET == M2->bin) 
	@Commutativity: read_consume <-> write_prepare (M1->bin == M2->C_RET) */


mpmc_boundq_1_alt<int32_t>* createMPMC(int size) {
	return new mpmc_boundq_1_alt<int32_t>(size);
}

void destroyMPMC(mpmc_boundq_1_alt<int32_t> *q) {
	delete q;
}

/** @PreCondition: */
int32_t * read_fetch(mpmc_boundq_1_alt<int32_t> *q) {
	return q->read_fetch();
}

/** @PreCondition: */
void read_consume(mpmc_boundq_1_alt<int32_t> *q, int32_t *bin) {
	q->read_consume(bin);
}

/** @PreCondition: */
int32_t * write_prepare(mpmc_boundq_1_alt<int32_t> *q) {
	return q->write_prepare();
}

/** @PreCondition: */
void write_publish(mpmc_boundq_1_alt<int32_t> *q, int32_t *bin) {
	q->write_publish(bin);
}
