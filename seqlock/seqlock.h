#include <stdatomic.h>
#include <threads.h>

typedef struct seqlock {
	// Sequence for reader consistency check
	atomic_int _seq;
	// It needs to be atomic to avoid data races
	atomic_int _data;

	seqlock() {
		atomic_init(&_seq, 0);
		atomic_init(&_data, 0);
	}

	int read() {
		while (true) {
			int old_seq = _seq.load(memory_order_acquire); // acquire
			if (old_seq % 2 == 1) continue;

			int res = _data.load(memory_order_acquire); // acquire
			if (_seq.load(memory_order_relaxed) == old_seq) { // relaxed
				return res;
			}
		}
	}
	
	void write(int new_data) {
		while (true) {
			// This might be a relaxed too
			int old_seq = _seq.load(memory_order_acquire); // acquire
			if (old_seq % 2 == 1)
				continue; // Retry

			// Should be relaxed!!! 
			if (_seq.compare_exchange_strong(old_seq, old_seq + 1,
				memory_order_relaxed, memory_order_relaxed)) // relaxed 
				break;
		}

		// Update the data
		_data.store(new_data, memory_order_release); // release

		_seq.fetch_add(1, memory_order_release); // release
	}

} seqlock_t;
