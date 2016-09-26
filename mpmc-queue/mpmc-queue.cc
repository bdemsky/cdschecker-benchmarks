#include <threads.h> 
#include "mpmc-queue.h"

template <typename t_element>
t_element * mpmc_boundq_1_alt<t_element>::read_fetch() {
	// FIXME: We can have a relaxed for sure here since the next CAS
	// will fix the problem
	unsigned int rdwr = m_rdwr.load(mo_acquire);
	unsigned int rd,wr;
	for(;;) {
		rd = (rdwr>>16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if ( wr == rd ) // empty
			return NULL;

		if ( m_rdwr.compare_exchange_weak(rdwr,rdwr+(1<<16),mo_acq_rel) )
			break;
		else
			thrd_yield();
	}

	// (*1)
	rl::backoff bo;
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
	/**********    Detected Admissibility (testcase2)    **********/
    // run with -Y
	m_read.fetch_add(1,mo_release);
	/** @OPDefine: true */
}


template <typename t_element>
t_element * mpmc_boundq_1_alt<t_element>::write_prepare() {
	// FIXME: We can have a relaxed for sure here since the next CAS
	// will fix the problem
	unsigned int rdwr = m_rdwr.load(mo_acquire);
	unsigned int rd,wr;
	for(;;) {
		rd = (rdwr>>16) & 0xFFFF;
		wr = rdwr & 0xFFFF;

		if ( wr == ((rd + t_size)&0xFFFF) ) // full
			return NULL;

		if ( m_rdwr.compare_exchange_weak(rdwr,(rd<<16) | ((wr+1)&0xFFFF),mo_acq_rel) )
			break;
		else
			thrd_yield();
	}

	// (*1)
	rl::backoff bo;
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
