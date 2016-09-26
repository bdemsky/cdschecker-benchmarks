#ifndef _MPMC_QUEUE_H
#define _MPMC_QUEUE_H

#include <stdatomic.h>
#include <unrelacy.h>

template <typename t_element>
struct mpmc_boundq_1_alt
{
private:
	// buffer capacity
	size_t t_size;
	// elements should generally be cache-line-size padded :
	t_element               *m_array;

	// rdwr counts the reads & writes that have started
	atomic<unsigned int>    m_rdwr;
	// "read" and "written" count the number completed
	atomic<unsigned int>    m_read;
	atomic<unsigned int>    m_written;

public:

	mpmc_boundq_1_alt(int size)
	{
		t_size = size;
		m_array = new t_element[size];
		m_rdwr = 0;
		m_read = 0;
		m_written = 0;
	}

	//-----------------------------------------------------

	t_element * read_fetch();

	void read_consume(t_element *bin);

	t_element * write_prepare();

	void write_publish(t_element *bin);
};

mpmc_boundq_1_alt<int32_t>* createMPMC(int size);

void destroyMPMC(mpmc_boundq_1_alt<int32_t> *q);

int32_t * read_fetch(mpmc_boundq_1_alt<int32_t> *q);

void read_consume(mpmc_boundq_1_alt<int32_t> *q, int32_t *bin);

int32_t * write_prepare(mpmc_boundq_1_alt<int32_t> *q);

void write_publish(mpmc_boundq_1_alt<int32_t> *q, int32_t *bin);

#endif
