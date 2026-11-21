#pragma once
#include <stdint.h>

// Called by the BSP from kmain: launch the other cores and wait
// for them to come online.
void smp_init();

// Phase 2 demo: every core hammers a shared counter, with and
// without the spinlock, to prove the lock prevents lost updates.
void smp_contention_test();

// Called by the BSP once the scheduler exists: the other cores start
// their own timers and begin taking processes off the run queue.
void smp_start_scheduling();

// Index of the core running this code: 0 for the BSP, then 1, 2, ...
// in the order the others were launched.
int smp_core_id();

// How many cores are online.
int smp_core_count();

// Incremented (atomically) by each core as it finishes booting.
extern volatile uint64_t cores_online;
