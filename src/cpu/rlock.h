#pragma once
#include "cpu/spinlock.h"
#include "cpu/apic.h"

// A spinlock the SAME core may take repeatedly (re-entrantly), tracking
// nesting depth. The terminal needs this because its public methods call
// one another (printf -> print -> draw_char -> next_line). Like the
// interrupt-safe spinlock, it disables interrupts while held so an ISR
// on the same core can't deadlock against it.
struct ReentrantLock {
	Spinlock inner;
	volatile uint32_t owner = 0xFFFFFFFF;   // 0xFFFFFFFF = unowned
	volatile int depth = 0;

	uint64_t acquire() {
		uint64_t flags = Spinlock::save_and_disable_interrupts();
		uint32_t me = lapic_id();

		// Already ours on this core? Just go one level deeper.
		if (owner == me) {
			depth++;
			return flags;
		}

		inner.acquire();
		owner = me;
		depth = 1;
		return flags;
	}

	void release(uint64_t flags) {
		if (--depth == 0) {
			owner = 0xFFFFFFFF;
			inner.release();
		}
		Spinlock::restore_interrupts(flags);
	}
};
