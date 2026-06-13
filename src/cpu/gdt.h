#pragma once
#include <stdint.h>

struct __attribute__((packed)) GDTEntry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
};

struct __attribute__((packed)) GDTDescriptor {
	uint16_t size;
	uint64_t offset;
};

// 64-bit Task State Segment. rsp0 is the kernel stack the CPU
// switches to when an interrupt arrives while running in ring 3.
struct __attribute__((packed)) TSS {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb_offset;
};

#define MAX_CORES 8

class GDT {
	public:
		// 0:   Null descriptor
		// 1:   Kernel code (ring 0) - selector 0x08
		// 2:   Kernel data (ring 0) - selector 0x10
		// 3:   User data   (ring 3) - selector 0x18 (0x1B with RPL 3)
		// 4:   User code   (ring 3) - selector 0x20 (0x23 with RPL 3)
		// 5+6:  TSS for core 0 - selector 0x28
		// 7+8:  TSS for core 1 - selector 0x38
		// ...   each core gets its own TSS (own kernel stack): 0x28 + core*0x10
		static GDTEntry gdt_entries[5 + 2 * MAX_CORES];
		static GDTDescriptor gdt_descriptor;
		static TSS tss[MAX_CORES];

		// BSP: build the whole table and load core 0's TSS
		static void init();
		// AP: load the shared GDT and this core's own TSS
		static void load_on_core(int core_id);
};

extern "C" void load_gdt(uint64_t gdt_descriptor_address);
extern "C" void load_tss(uint16_t selector);
