#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <atomic>
#include <mutex>
#include "unrelacy.h"

using std::atomic;
using std::atomic_int;
using std::mutex;

class Entry {
	public:
	int key;
	atomic_int value;
	int hash;
	atomic<Entry*> next;

	Entry() { }

	Entry(int h, int k, int v, Entry *n) {
		this->hash = h;
		this->key = k;
		this->value.store(v, relaxed);
		/** OPClearDefine: true */
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

	int hashKey(int x) {
		//int h = x->hashCode();
		int h = x;
		// Use logical right shift
		unsigned int tmp = (unsigned int) h;
		//return ((h << 7) - h + (tmp >> 9) + (tmp >> 17));
		return x;
	}

	bool eq(int x, int y) {
		//return x == y || x->equals(y);
		return x == y;
	}

	int get(int key);

	int put(int key, int value);

};

/** C Interface */
int get(HashMap *map, int key);
int put(HashMap *map, int key, int value);

#endif
