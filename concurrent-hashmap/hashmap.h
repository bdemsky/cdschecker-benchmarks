#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <iostream>
#include <atomic>
#include "stdio.h" 
//#include <common.h>
#ifdef STANDALONE
#include <assert.h>
#define MODEL_ASSERT assert 
#else
#include <model-assert.h>
#endif
#include <stdlib.h>
#include <mutex>

#define relaxed memory_order_relaxed
#define release memory_order_release
#define acquire memory_order_acquire
#define acq_rel memory_order_acq_rel
#define seq_cst memory_order_seq_cst

using namespace std;

/**
	For the sake of simplicity, we do not use template but some toy structs to
	represent the Key and Value.
*/
struct Key {
	// Probably represent the coordinate (x, y, z)
	int x;
	int y;
	int z;

	int hashCode() {
		return x + 31 * y + 31 * 31 * z;
	}

	bool equals(Key *other) {
		if (!other)
			return false;
		return x == other->x && y == other->y && z == other->z;
	}

	Key(int x, int y, int z) :
		x(x),
		y(y),
		z(z)
	{

	}
};

struct Value {
	// Probably represent the speed vector (vX, vY, vZ)
	int vX;
	int vY;
	int vZ;

	Value(int vX, int vY, int vZ) :
		vX(vX),
		vY(vY),
		vZ(vZ)
	{

	}
};

class Entry {
	public:
	Key *key;
	atomic<Value*> value;
	int hash;
	atomic<Entry*> next;

	Entry(int h, Key *k, Value *v, Entry *n) {
		this->hash = h;
		this->key = k;
		this->value.store(v, relaxed);
		this->next.store(n, relaxed);
	}
};

class Segment {
	public:
	int count;
	mutex segMutex;

	void lock() {
		segMutex.lock();
	}

	void unlock() {
		segMutex.unlock();
	}

	Segment() {
		this->count = 0;
	}
};

class HashMap {
	public:
	atomic<Entry*> *table;

	int capacity;
	int size;

	static const int CONCURRENCY_LEVEL = 16;

	static const int SEGMENT_MASK = CONCURRENCY_LEVEL - 1;

	Segment *segments[CONCURRENCY_LEVEL];

	static const int DEFAULT_INITIAL_CAPACITY = 16;

	// Not gonna consider resize now...
	
	HashMap() {
		this->size = 0;
		this->capacity = DEFAULT_INITIAL_CAPACITY;
		this->table = new atomic<Entry*>[capacity];
		for (int i = 0; i < capacity; i++) {
			atomic_init(&table[i], NULL);
		}
		for (int i = 0; i < CONCURRENCY_LEVEL; i++) {
			segments[i] = new Segment;
		}
	}

	int hashKey(Key *x) {
		int h = x->hashCode();
		// Use logical right shift
		unsigned int tmp = (unsigned int) h;
		return ((h << 7) - h + (tmp >> 9) + (tmp >> 17));
	}

	bool eq(Key *x, Key *y) {
		return x == y || x->equals(y);
	}

	Value* get(Key *key) {
		int hash = hashKey(key);	// throws null pointer exception if key null

		// Try first without locking...
		atomic<Entry*> *tab = table;
		int index = hash & (capacity - 1);
		atomic<Entry*> *first = &tab[index];
		Entry *e;
		Value *res = NULL;

		// Should be a load acquire
		Entry *firstPtr = first->load(acquire);
		e = firstPtr;
		while (e != NULL) {
			if (e->hash == hash && eq(key, e->key)) {
				res = e->value.load(seq_cst);
				if (res != NULL)
					return res;
				else
					break;
				// Loading the next entry
				e = e->next.load(acquire);
			}
		}
	
		// Recheck under synch if key apparently not there or interference
		Segment *seg = segments[hash & SEGMENT_MASK];
		seg->lock(); // Critical region begins
		// Not considering resize now, so ignore the reload of table...

		// Synchronized by locking, no need to be load acquire
		Entry *newFirstPtr = first->load(relaxed);
		if (e != NULL || firstPtr != newFirstPtr) {
			e = newFirstPtr;
			while (e != NULL) {
				if (e->hash == hash && eq(key, e->key)) {
					res = e->value.load(seq_cst);
					return res;
				}
				// Synchronized by locking
				e = e->next.load(relaxed);
			}
		}
		seg->unlock(); // Critical region ends
		return NULL;
	}

	Value* put(Key *key, Value *value) {
		// Don't allow NULL value
		//ASSERT (value != NULL);

		int hash = hashKey(key);
		Segment *seg = segments[hash & SEGMENT_MASK];
		atomic<Entry*> *tab;

		seg->lock(); // Critical region begins
		tab = table;
		int index = hash & (capacity - 1);

		atomic<Entry*> *first = &tab[index];
		Entry *e;
		Value *res = NULL;
	
		// The written of the entry is synchronized by locking
		Entry *firstPtr = first->load(relaxed);
		e = firstPtr;
		while (e != NULL) {
			if (e->hash == hash && eq(key, e->key)) {
				// FIXME: This could be an acquire?? 
				res = e->value.load(acquire);
				e->value.store(value, seq_cst);
				seg->unlock(); // Don't forget to unlock before return
				return res;
			}
		}

		// Add to front of list
		Entry *newEntry = new Entry(hash, key, value, firstPtr);
		// Publish the newEntry to others
		first->store(newEntry, release);
		seg->unlock(); // Critical region ends
		return NULL;
	}
};

#endif
