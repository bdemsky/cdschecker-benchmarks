#ifndef _RCU_H
#define _RCU_H

#include <atomic>
#include <threads.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>

#include "librace.h"

struct Data {
	// Declare atomic to expose them to CDSChecker.
    // Otherwise, we might miss the data race reports.
	atomic_int data1;
	atomic_int data2;
};


extern atomic<Data*> dataPtr;

void read(int *data1, int *data2);

void write(int data1, int data2);

#endif
