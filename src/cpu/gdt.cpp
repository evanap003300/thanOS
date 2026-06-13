#include "cpu/gdt.h"

// Define static members
GDTEntry GDT::gdt_entries[5 + 2 * MAX_CORES];
GDTDescriptor GDT::gdt_descriptor;
TSS GDT::tss[MAX_CORES];

// One kernel stack per core - the stack the CPU lands on when an
// interrupt arrives from ring 3 on that core.
alignas(16) static uint8_t core_stacks[MAX_CORES][16384];

// Fill in core_id's TSS (its kernel stack) and the 16-byte TSS
// descriptor for it in the GDT.
static void setup_core_tss(int core_id) {
	int idx = 5 + core_id * 2;
	uint64_t base = (uint64_t)&GDT::tss[core_id];

	GDT::tss[core_id].rsp0 = (uint64_t)core_stacks[core_id] + sizeof(core_stacks[core_id]);
	GDT::tss[core_id].iopb_offset = sizeof(TSS);

	GDT::gdt_entries[idx].limit_low = sizeof(TSS) - 1;
	GDT::gdt_entries[idx].base_low = (uint16_t)(base & 0xFFFF);
	GDT::gdt_entries[idx].base_middle = (uint8_t)((base >> 16) & 0xFF);
	GDT::gdt_entries[idx].access = 0x89;
	GDT::gdt_entries[idx].granularity = 0;
	GDT::gdt_entries[idx].base_high = (uint8_t)((base >> 24) & 0xFF);
	*(uint64_t*)&GDT::gdt_entries[idx + 1] = (base >> 32) & 0xFFFFFFFF;
}

void GDT::init() {

	// Null Entry
	gdt_entries[0] = {0, 0, 0, 0, 0, 0};

	// Kernel code segment entry
	gdt_entries[1] = {0, 0, 0, 0x9A, 0xA0, 0};

	// Kernel data segment entry
	gdt_entries[2] = {0, 0, 0, 0x92, 0xC0, 0};

	// User data segment entry (same as kernel data, DPL=3)
	gdt_entries[3] = {0, 0, 0, 0xF2, 0xC0, 0};

	// User code segment entry (same as kernel code, DPL=3)
	gdt_entries[4] = {0, 0, 0, 0xFA, 0xA0, 0};

	// A TSS descriptor for every core (each with its own kernel stack)
	for (int i = 0; i < MAX_CORES; i++) {
		setup_core_tss(i);
	}

	gdt_descriptor.size = sizeof(gdt_entries) - 1;
	gdt_descriptor.offset = (uint64_t)&gdt_entries;

	load_gdt((uint64_t)&gdt_descriptor);
	load_tss(0x28);   // core 0 (the BSP)
}

void GDT::load_on_core(int core_id) {
	// Every core shares the same GDT, but loads its own TSS selector
	load_gdt((uint64_t)&gdt_descriptor);
	load_tss(0x28 + core_id * 0x10);
}
