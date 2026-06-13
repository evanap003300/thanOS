#pragma once
#include <stdint.h>

// Called by the BSP from kmain: launch the other cores and wait
// for them to come online.
void smp_init();

// Incremented (atomically) by each core as it finishes booting.
extern volatile uint64_t cores_online;
