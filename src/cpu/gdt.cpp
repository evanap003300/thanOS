#include "cpu/gdt.h"

// Define static members
GDTEntry GDT::gdt_entries[7];
GDTDescriptor GDT::gdt_descriptor;
TSS GDT::tss;

// The stack the CPU lands on when an interrupt arrives from ring 3
alignas(16) static uint8_t interrupt_stack[16384];

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

	// TSS: stacks grow down, so rsp0 is the *top* of the stack
	tss.rsp0 = (uint64_t)interrupt_stack + sizeof(interrupt_stack);
	tss.iopb_offset = sizeof(TSS);

	// TSS descriptor: 16 bytes, spans entries 5 and 6.
	// Unlike code/data descriptors the base address matters here.
	uint64_t tss_base = (uint64_t)&tss;
	gdt_entries[5].limit_low = sizeof(TSS) - 1;
	gdt_entries[5].base_low = (uint16_t)(tss_base & 0xFFFF);
	gdt_entries[5].base_middle = (uint8_t)((tss_base >> 16) & 0xFF);
	gdt_entries[5].access = 0x89;
	gdt_entries[5].granularity = 0;
	gdt_entries[5].base_high = (uint8_t)((tss_base >> 24) & 0xFF);
	*(uint64_t*)&gdt_entries[6] = (tss_base >> 32) & 0xFFFFFFFF;

	gdt_descriptor.size = sizeof(gdt_entries) - 1;
	gdt_descriptor.offset = (uint64_t)&gdt_entries;

	load_gdt((uint64_t)&gdt_descriptor);
	load_tss(0x28);
}
