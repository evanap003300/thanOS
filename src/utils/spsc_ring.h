#pragma once
#include <stdint.h>
#include <stddef.h>

// Lock-free single-producer / single-consumer ring buffer.
//
// Exactly one context may push and exactly one may pop. Neither ever
// waits on the other: push() only writes head_, pop() only writes
// tail_. That makes it safe for an interrupt handler to push while the
// code it interrupted is halfway through a pop, and for the two sides
// to sit on different cores.
//
// N must be a power of two. head_ and tail_ are free-running counters:
// the slot is (counter & (N - 1)) and (head_ - tail_) is the fill level.
template <typename T, size_t N>
class SpscRing {
	static_assert(N >= 2 && (N & (N - 1)) == 0, "capacity must be a power of two");

public:
	// Producer side. Returns false (the item is dropped) if the ring is full.
	bool push(const T& item) {
		// Only the producer writes head_, so its own read can be relaxed
		uint64_t head = __atomic_load_n(&head_, __ATOMIC_RELAXED);
		uint64_t tail = __atomic_load_n(&tail_, __ATOMIC_ACQUIRE);

		if (head - tail == N) {
			return false;
		}

		slots_[head & (N - 1)] = item;

		// Publish. Release pairs with the consumer's acquire: once it
		// sees the new head_, the slot write above is visible too
		__atomic_store_n(&head_, head + 1, __ATOMIC_RELEASE);
		return true;
	}

	// Consumer side. Returns false if the ring is empty.
	bool pop(T& out) {
		uint64_t tail = __atomic_load_n(&tail_, __ATOMIC_RELAXED);
		uint64_t head = __atomic_load_n(&head_, __ATOMIC_ACQUIRE);

		if (head == tail) {
			return false;
		}

		out = slots_[tail & (N - 1)];

		// Hand the slot back to the producer
		__atomic_store_n(&tail_, tail + 1, __ATOMIC_RELEASE);
		return true;
	}

	// A snapshot - it can be stale by the time the caller acts on it
	bool empty() const {
		return __atomic_load_n(&head_, __ATOMIC_ACQUIRE) ==
		       __atomic_load_n(&tail_, __ATOMIC_ACQUIRE);
	}

private:
	// The two counters live on separate cache lines so the producer and
	// consumer don't false-share once they run on different cores
	alignas(64) uint64_t head_ = 0;
	alignas(64) uint64_t tail_ = 0;
	T slots_[N];
};
