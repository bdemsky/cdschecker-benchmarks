#include <stdatomic.h>
#include <inttypes.h>
#include "deque.h"
#include <stdlib.h>
#include <stdio.h>
#include "wildcard.h"

Deque * create_size(int size) {
	Deque * q = (Deque *) calloc(1, sizeof(Deque));
	Array * a = (Array *) calloc(1, sizeof(Array)+2*sizeof(atomic_int));
	atomic_store_explicit(&q->array, a, wildcard(1));
	atomic_store_explicit(&q->top, 0, wildcard(2));
	atomic_store_explicit(&q->bottom, 0, wildcard(3));
	atomic_store_explicit(&a->size, size, wildcard(4));
	return q;
}

Deque * create() {
	Deque * q = (Deque *) calloc(1, sizeof(Deque));
	Array * a = (Array *) calloc(1, sizeof(Array)+2*sizeof(atomic_int));
	atomic_store_explicit(&q->array, a, wildcard(1));
	atomic_store_explicit(&q->top, 0, wildcard(2));
	atomic_store_explicit(&q->bottom, 0, wildcard(3));
	atomic_store_explicit(&a->size, 2, wildcard(4));
	return q;
}

int take(Deque *q) {
	size_t b = atomic_load_explicit(&q->bottom, wildcard(5)) - 1;
	Array *a = (Array *) atomic_load_explicit(&q->array, wildcard(6));
	atomic_store_explicit(&q->bottom, b, wildcard(7));
	atomic_thread_fence(wildcard(8)); // seq_cst
	size_t t = atomic_load_explicit(&q->top, wildcard(9));
	int x;
	if (t <= b) {
		/* Non-empty queue. */
		x = atomic_load_explicit(&a->buffer[b %
		atomic_load_explicit(&a->size,wildcard(10))], wildcard(11));
		if (t == b) {
			/* Single last element in queue. */
			if (!atomic_compare_exchange_strong_explicit(&q->top, &t, t + 1,
				wildcard(12), wildcard(13))) // 12 -> seq_cst
				/* Failed race. */
				x = EMPTY;
			atomic_store_explicit(&q->bottom, b + 1, wildcard(14));
		}
	} else { /* Empty queue. */
		x = EMPTY;
		atomic_store_explicit(&q->bottom, b + 1, wildcard(15));
	}
	return x;
}

void resize(Deque *q) {
	Array *a = (Array *) atomic_load_explicit(&q->array, wildcard(16));
	size_t size=atomic_load_explicit(&a->size, wildcard(17));
	size_t new_size=size << 1;
	Array *new_a = (Array *) calloc(1, new_size * sizeof(atomic_int) + sizeof(Array));
	size_t top=atomic_load_explicit(&q->top, wildcard(18));
	size_t bottom=atomic_load_explicit(&q->bottom, wildcard(19));
	atomic_store_explicit(&new_a->size, new_size, wildcard(20));
	size_t i;
	for(i=top; i < bottom; i++) {
		atomic_store_explicit(&new_a->buffer[i % new_size],
		atomic_load_explicit(&a->buffer[i % size], wildcard(21)), wildcard(22));
	}
	atomic_store_explicit(&q->array, new_a, wildcard(23)); // release
	printf("resize\n");
}

void push(Deque *q, int x) {
	size_t b = atomic_load_explicit(&q->bottom, wildcard(24));
	size_t t = atomic_load_explicit(&q->top, wildcard(25)); // acquire
	Array *a = (Array *) atomic_load_explicit(&q->array, wildcard(26));
	if (b - t > atomic_load_explicit(&a->size, wildcard(27)) - 1) /* Full queue. */ {
		resize(q);
		//Bug in paper...should have next line...
		a = (Array *) atomic_load_explicit(&q->array, wildcard(28));
	}
	atomic_store_explicit(&a->buffer[b % atomic_load_explicit(&a->size,
	wildcard(29))], x, wildcard(30));
	atomic_thread_fence(wildcard(31)); // release
	//atomic_thread_fence(release); // release
	atomic_store_explicit(&q->bottom, b + 1, wildcard(32));
}

int steal(Deque *q) {
	size_t t = atomic_load_explicit(&q->top, wildcard(33)); // acquire
	atomic_thread_fence(wildcard(34)); // seq_cst
	size_t b = atomic_load_explicit(&q->bottom, wildcard(35)); // acquire
	int x = EMPTY;
	if (t < b) {
		/* Non-empty queue. */
		// acquire
		Array *a = (Array *) atomic_load_explicit(&q->array, wildcard(36));
		x = atomic_load_explicit(&a->buffer[t % atomic_load_explicit(&a->size,
		wildcard(37))], wildcard(38));
		// seq_cst
		if (!atomic_compare_exchange_strong_explicit(&q->top, &t, t + 1,
			wildcard(39), wildcard(40)))
			/* Failed race. */
			return ABORT;
	}
	return x;
}
