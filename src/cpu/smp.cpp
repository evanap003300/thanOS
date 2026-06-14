#include "cpu/smp.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "cpu/apic.h"
#include "cpu/spinlock.h"
#include "graphics/render.h"
#include "limine.h"
#include <stddef.h>

// Each core records its hardware LAPIC ID here, indexed by core id.
static uint32_t core_lapic_ids[MAX_CORES];

// --- Phase 2 contention demo ---
#define HAMMER_ITERATIONS 100000

static volatile uint64_t locked_counter = 0;     // incremented under the lock
static volatile uint64_t unlocked_counter = 0;   // incremented raw (racy)
static Spinlock counter_lock;
static volatile uint64_t total_cores = 1;
static volatile uint64_t hammer_done = 0;
static volatile bool hammer_go = false;

// Run on every core. The hammer_go gate releases all cores at once
// for maximum overlap, then each adds to both counters the same
// number of times.
static void hammer() {
	while (!__atomic_load_n(&hammer_go, __ATOMIC_ACQUIRE)) {
		__builtin_ia32_pause();
	}

	for (int i = 0; i < HAMMER_ITERATIONS; i++) {
		unlocked_counter++;          // no protection - updates can be lost

		counter_lock.acquire();
		locked_counter++;            // protected - every update counts
		counter_lock.release();
	}

	__atomic_fetch_add(&hammer_done, 1, __ATOMIC_SEQ_CST);
}

__attribute__((used, section(".limine_requests")))
volatile struct limine_mp_request mp_request = {
	.id = LIMINE_MP_REQUEST_ID,
	.revision = 0,
	.response = NULL,
	.flags = 0
};

// The BSP counts as one online core from the start.
volatile uint64_t cores_online = 1;

// Where every Application Processor begins. Limine jumps here in
// 64-bit mode, on the kernel page tables, with its own stack.
extern "C" void ap_entry(struct limine_mp_info* info) {
	int core_id = (int)info->extra_argument;

	// Load this core's own view of the per-core CPU tables, then park.
	// No interrupts yet - APs stay halted until the scheduler (Phase 4).
	GDT::load_on_core(core_id);
	IDT::load();
	apic_init_ap();
	core_lapic_ids[core_id] = lapic_id();

	__atomic_fetch_add(&cores_online, 1, __ATOMIC_SEQ_CST);

	// Join the contention test, then park.
	hammer();

	for (;;) {
		asm volatile ("hlt");
	}
}

void smp_init() {
	if (mp_request.response == NULL) {
		terminal.printf("SMP: no multiprocessor response\n");
		return;
	}

	struct limine_mp_response* mp = mp_request.response;
	uint64_t count = mp->cpu_count;

	// Record the BSP's own LAPIC ID (its LAPIC was enabled in kmain).
	core_lapic_ids[0] = lapic_id();

	terminal.printf("SMP: %d CPU(s) detected\n", count);

	int next_core_id = 1;
	for (uint64_t i = 0; i < count; i++) {
		struct limine_mp_info* cpu = mp->cpus[i];

		// Skip ourselves - the BSP is already core 0 and running.
		if (cpu->lapic_id == mp->bsp_lapic_id) {
			continue;
		}

		if (next_core_id >= MAX_CORES) {
			terminal.printf("SMP: capping at %d cores\n", MAX_CORES);
			break;
		}

		// Hand the core its id, then writing goto_address launches it.
		cpu->extra_argument = (uint64_t)next_core_id;
		next_core_id++;
		__atomic_store_n(&cpu->goto_address, (limine_goto_address)ap_entry, __ATOMIC_SEQ_CST);
	}

	// Spin until every core we launched has checked in.
	uint64_t expected = (count < MAX_CORES) ? count : MAX_CORES;
	while (__atomic_load_n(&cores_online, __ATOMIC_SEQ_CST) < expected) {
		asm volatile ("pause");
	}

	total_cores = expected;
	terminal.printf("SMP: %d cores online\n", cores_online);

	for (uint64_t i = 0; i < expected; i++) {
		terminal.printf("  core %d -> LAPIC id %d\n", i, core_lapic_ids[i]);
	}
}

void smp_contention_test() {
	uint64_t cores = total_cores;
	uint64_t expected = cores * HAMMER_ITERATIONS;

	// Release every core at once, then join in ourselves.
	__atomic_store_n(&hammer_go, true, __ATOMIC_RELEASE);
	hammer();

	// Wait for all cores to finish hammering.
	while (__atomic_load_n(&hammer_done, __ATOMIC_SEQ_CST) < cores) {
		__builtin_ia32_pause();
	}

	terminal.printf("LOCK TEST: %d cores x %d each, expected %d\n",
			cores, HAMMER_ITERATIONS, expected);
	terminal.printf("  with lock:    %d  %s\n", locked_counter,
			locked_counter == expected ? "(exact!)" : "(LOST UPDATES)");
	terminal.printf("  without lock: %d  %s\n", unlocked_counter,
			unlocked_counter == expected ? "(exact)" : "(lost updates - race!)");
}
