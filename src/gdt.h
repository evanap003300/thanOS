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

class GDT {
	public:
		// 0: Null Descriptor
		// 1: Kernel code (ring 0)
		// 2: Kernel data (ring 0)
		// ... add other rings later for user
		GDTEntry gdt_entries[3];
		GDTDescriptor gdt_descriptor;
		void init();
};

extern "C" void load_gdt(uint64_t gdt_descriptor_address);
