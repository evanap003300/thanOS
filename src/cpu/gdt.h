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

class GDT {
	public:
		// 0:   Null descriptor
		// 1:   Kernel code (ring 0) - selector 0x08
		// 2:   Kernel data (ring 0) - selector 0x10
		// 3:   User data   (ring 3) - selector 0x18 (0x1B with RPL 3)
		// 4:   User code   (ring 3) - selector 0x20 (0x23 with RPL 3)
		// 5+6: TSS descriptor (16 bytes in long mode) - selector 0x28
		static GDTEntry gdt_entries[7];
		static GDTDescriptor gdt_descriptor;
		static TSS tss;
		static void init();
};

extern "C" void load_gdt(uint64_t gdt_descriptor_address);
extern "C" void load_tss(uint16_t selector);
