#include "cpu/smp.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "graphics/render.h"
#include "limine.h"
#include <stddef.h>

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

	__atomic_fetch_add(&cores_online, 1, __ATOMIC_SEQ_CST);

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

	terminal.printf("SMP: %d cores online\n", cores_online);
}
