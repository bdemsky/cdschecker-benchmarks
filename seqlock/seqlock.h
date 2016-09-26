#ifndef _SEQLOCK_H
#define _SEQLOCK_H

#include <stdatomic.h>

class seqlock {
	public:
	// Sequence for reader consistency check
	atomic_int _seq;
	// It needs to be atomic to avoid data races
	atomic_int _data1;
	atomic_int _data2;

	seqlock();

	void read(int *data1, int *data2);

	void write(int data1, int data2);
};

/** C Interface */
void write(seqlock *lock, int data1, int data2);
void read(seqlock *lock, int *data1, int *data2);

#endif
