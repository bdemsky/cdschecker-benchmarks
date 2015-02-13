#include <threads.h>
#include <stdlib.h>
#include "librace.h"
#include "model-assert.h"

#include "queue.h"
#include "wildcard.h"

#define MAX_FREELIST 4 /* Each thread can own up to MAX_FREELIST free nodes */
#define INITIAL_FREE 2 /* Each thread starts with INITIAL_FREE free nodes */

#define POISON_IDX 0x666

static unsigned int (*free_lists)[MAX_FREELIST];

/* Search this thread's free list for a "new" node */
static unsigned int new_node()
{
	int i;
	int t = get_thread_num();
	for (i = 0; i < MAX_FREELIST; i++) {
		//unsigned int node = load_32(&free_lists[t][i]);
		unsigned int node = free_lists[t][i];
		if (node) {
			//store_32(&free_lists[t][i], 0);
			free_lists[t][i] = 0;
			return node;
		}
	}
	/* free_list is empty? */
	MODEL_ASSERT(0);
	return 0;
}

/* Place this node index back on this thread's free list */
static void reclaim(unsigned int node)
{
	int i;
	int t = get_thread_num();

	/* Don't reclaim NULL node */
	//MODEL_ASSERT(node);

	for (i = 0; i < MAX_FREELIST; i++) {
		/* Should never race with our own thread here */
		//unsigned int idx = load_32(&free_lists[t][i]);
		unsigned int idx = free_lists[t][i];

		/* Found empty spot in free list */
		if (idx == 0) {
			//store_32(&free_lists[t][i], node);
			free_lists[t][i] = node;
			return;
		}
	}
	/* free list is full? */
	MODEL_ASSERT(0);
}

void init_queue(queue_t *q, int num_threads)
{
	int i, j;

	/* Initialize each thread's free list with INITIAL_FREE pointers */
	/* The actual nodes are initialized with poison indexes */
	free_lists = malloc(num_threads * sizeof(*free_lists));
	for (i = 0; i < num_threads; i++) {
		for (j = 0; j < INITIAL_FREE; j++) {
			free_lists[i][j] = 2 + i * MAX_FREELIST + j;
			atomic_init(&q->nodes[free_lists[i][j]].next, MAKE_POINTER(POISON_IDX, 0));
		}
	}

	/* initialize queue */
	atomic_init(&q->head, MAKE_POINTER(1, 0));
	atomic_init(&q->tail, MAKE_POINTER(1, 0));
	atomic_init(&q->nodes[1].next, MAKE_POINTER(0, 0));
}

void enqueue(queue_t *q, unsigned int val)
{
	int success = 0;
	unsigned int node;
	pointer tail;
	pointer next;
	pointer tmp;

	node = new_node();
	//store_32(&q->nodes[node].value, val);
	q->nodes[node].value = val;
	tmp = atomic_load_explicit(&q->nodes[node].next, wildcard(1)); // relaxed
	set_ptr(&tmp, 0); // NULL
	atomic_store_explicit(&q->nodes[node].next, tmp, wildcard(2)); // relaxed

	while (!success) {
		tail = atomic_load_explicit(&q->tail, wildcard(3)); // acquire
		// FIXME: SCFence makes this relaxed 
		next = atomic_load_explicit(&q->nodes[get_ptr(tail)].next, wildcard(4)); //acquire
		if (tail == atomic_load_explicit(&q->tail, wildcard(5))) { // relaxed

			/* Check for uninitialized 'next' */
			//MODEL_ASSERT(get_ptr(next) != POISON_IDX);

			if (get_ptr(next) == 0) { // == NULL
				pointer value = MAKE_POINTER(node, get_count(next) + 1);
				success = atomic_compare_exchange_strong_explicit(&q->nodes[get_ptr(tail)].next,
						&next, value, wildcard(6), wildcard(7)); // release & relaxed
			}
			if (!success) {
				unsigned int ptr =
				get_ptr(atomic_load_explicit(&q->nodes[get_ptr(tail)].next, wildcard(8))); // acquire
				pointer value = MAKE_POINTER(ptr,
						get_count(tail) + 1);
				atomic_compare_exchange_strong_explicit(&q->tail,
						&tail, value,
						wildcard(9), wildcard(10)); // release & relaxed
				thrd_yield();
			}
		}
	}
	atomic_compare_exchange_strong_explicit(&q->tail,
			&tail,
			MAKE_POINTER(node, get_count(tail) + 1),
			wildcard(11), wildcard(12)); // release & relaxed
}

bool dequeue(queue_t *q, unsigned int *retVal)
{
	unsigned int value;
	int success = 0;
	pointer head;
	pointer tail;
	pointer next;

	while (!success) {
		head = atomic_load_explicit(&q->head, wildcard(13)); // acquire
		// SCFence makes this acquire, and we actually need an acquire here!!!
		tail = atomic_load_explicit(&q->tail, wildcard(14)); // relaxed 
		next = atomic_load_explicit(&q->nodes[get_ptr(head)].next, wildcard(15)); // acquire
		if (atomic_load_explicit(&q->head, wildcard(16)) == head) { // relaxed
			if (get_ptr(head) == get_ptr(tail)) {

				/* Check for uninitialized 'next' */
				//MODEL_ASSERT(get_ptr(next) != POISON_IDX);

				if (get_ptr(next) == 0) { // NULL
					return false; // NULL
				}
				atomic_compare_exchange_strong_explicit(&q->tail,
						&tail,
						MAKE_POINTER(get_ptr(next), get_count(tail) + 1),
						wildcard(17), wildcard(18)); // release & relaxed
				thrd_yield();
			} else {
				//value = load_32(&q->nodes[get_ptr(next)].value);
				value = q->nodes[get_ptr(next)].value;
				// FIXME: SCFence makes this relaxed 
				success = atomic_compare_exchange_strong_explicit(&q->head,
						&head, MAKE_POINTER(get_ptr(next), get_count(head) + 1),
						wildcard(19), wildcard(20)); // release & relaxed
				if (!success)
					thrd_yield();
			}
		}
	}
	reclaim(get_ptr(head));
	*retVal = value;
	return true;
}
