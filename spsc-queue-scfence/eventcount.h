#include <unrelacy.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "wildcard.h" 

class eventcount
{
public:
	eventcount() : waiters(0)
	{
		count = 0;
	}

	void signal_relaxed()
	{
		unsigned cmp = count.load(wildcard(1)); // relaxed
		signal_impl(cmp);
	}

	void signal()
	{
		unsigned cmp = count.fetch_add(0, std::memory_order_seq_cst); // the fix
		signal_impl(cmp);
	}

	unsigned get()
	{
		unsigned cmp = count.fetch_or(0x80000000, wildcard(2)); // sc
		return cmp & 0x7FFFFFFF;
	}

	void wait(unsigned cmp)
	{
		unsigned ec = count.load(wildcard(3)); // sc
		if (cmp == (ec & 0x7FFFFFFF))
		{
			guard.lock($);
			ec = count.load(wildcard(4)); // sc
			if (cmp == (ec & 0x7FFFFFFF))
			{
				waiters += 1;
				cv.wait(guard);
			}
			guard.unlock($);
		}
	}

private:
	std::atomic<unsigned> count;
	rl::var<unsigned> waiters;
	std::mutex guard;
	std::condition_variable cv;

	void signal_impl(unsigned cmp)
	{
		if (cmp & 0x80000000)
		{
			guard.lock($);
			while (false == count.compare_exchange_weak(cmp,
				(cmp + 1) & 0x7FFFFFFF, wildcard(5))); // relaxed
			unsigned w = waiters($);
			waiters = 0;
			guard.unlock($);
			if (w)
				cv.notify_all($);
		}
	}
};
