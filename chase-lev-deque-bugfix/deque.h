#ifndef DEQUE_H
#define DEQUE_H

#include <stdatomic.h>
#include <inttypes.h>

typedef struct {
	atomic_size_t size;
	atomic_int buffer[];
} Array;

typedef struct {
	atomic_size_t top, bottom;
	atomic_uintptr_t array; /* Atomic(Array *) */

    // This is just used to mask the uninitialized loads in the known bugs to
    // show that even without the CDSChecker's internal check, CDSSpec can
    // also
    // detects the known bug.
    void *newArray;
} Deque;

Deque * create_size(int size);
Deque * create();
int take(Deque *q);
void resize(Deque *q);
void push(Deque *q, int x);
int steal(Deque *q);

#define EMPTY 0xffffffff
#define ABORT 0xfffffffe

/** @Define:
#ifdef __cplusplus
extern "C" {
#endif

bool succ(int res);
bool fail(int res);

#ifdef __cplusplus
};
#endif
*/

#endif
