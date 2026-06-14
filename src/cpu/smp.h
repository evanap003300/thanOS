#pragma once
#include <stdint.h>

// Called by the BSP from kmain: launch the other cores and wait
// for them to come online.
void smp_init();

// Phase 2 demo: every core hammers a shared counter, with and
// without the spinlock, to prove the lock prevents lost updates.
void smp_contention_test();

// Incremented (atomically) by each core as it finishes booting.
extern volatile uint64_t cores_online;
