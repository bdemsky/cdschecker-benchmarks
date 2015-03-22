#include <stdatomic.h>
#include <unrelacy.h>
#include "wildcard.h"

template <typename t_element, size_t t_size>
struct mpmc_boundq_1_alt
{
private:

	// elements should generally be cache-line-size padded :
	t_element               m_array[t_size];

	// rdwr counts the reads & writes that have started
	atomic<unsigned int>    m_rdwr;
	// "read" and "written" count the number completed
	atomic<unsigned int>    m_read;
	atomic<unsigned int>    m_written;

public:

	mpmc_boundq_1_alt()
	{
		m_rdwr = 0;
		m_read = 0;
		m_written = 0;
	}
	

	/**
	@Global_define:
	Order_queue<unsigned int*> spec_queue;
	*/

	//-----------------------------------------------------

	t_element * read_fetch() {
		unsigned int rdwr = m_rdwr.load(wildcard(1)); // acquire, but can be relaxed
		unsigned int rd,wr;
		for(;;) {
			rd = (rdwr>>16) & 0xFFFF;
			wr = rdwr & 0xFFFF;

			if ( wr == rd ) { // empty
				return NULL;
			}

			// acq_rel
			if ( m_rdwr.compare_exchange_weak(rdwr,rdwr+(1<<16),wildcard(2)) )
				break;
			else
				thrd_yield();
		}

		// (*1)
		rl::backoff bo;
		// acquire
		while ( (m_written.load(wildcard(3)) & 0xFFFF) != wr ) {
			thrd_yield();
		}

		t_element * p = & ( m_array[ rd % t_size ] );
		
		/**
			@Commit_point_Check: true
			@Label: ANY
			@Check:
				spec_queue.peak() == p
		*/
		return p;
	}

	void read_consume() {
		m_read.fetch_add(1,wildcard(4)); // release
		/**
			@Commit_point_define: true
			@Label: Read_Consume_Success
			@Check:
				spec_queue.size() > 0
			@Action:
				spec_queue.remove();
		*/
	}

	//-----------------------------------------------------

	t_element * write_prepare() {
		unsigned int rdwr = m_rdwr.load(wildcard(5)); // acquire, but can be relaxed
		unsigned int rd,wr;
		for(;;) {
			rd = (rdwr>>16) & 0xFFFF;
			wr = rdwr & 0xFFFF;

			if ( wr == ((rd + t_size)&0xFFFF) ) // full
				return NULL;
			// acq_rel
			if ( m_rdwr.compare_exchange_weak(rdwr,(rd<<16) | ((wr+1)&0xFFFF),wildcard(6)) )
				break;
			else
				thrd_yield();
		}

		// (*1)
		rl::backoff bo;
		while ( (m_read.load(wildcard(7)) & 0xFFFF) != rd ) { // acquire
			thrd_yield();
		}


		t_element * p = & ( m_array[ wr % t_size ] );

		/**
			@Commit_point_check: ANY
			@Action: spec_queue.add(p);
		*/
		return p;
	}

	void write_publish()
	{
		m_written.fetch_add(1,wildcard(8)); // release
	}

	//-----------------------------------------------------


};

typedef struct mpmc_boundq_1_alt<int, 4> queue_t;
