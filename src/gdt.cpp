#include "gdt.h"

extern "C" void load_gdt(uint64_t gdt_descriptor_address);

void GDT::init() {
	gdt_entries[0] = {0, 0, 0, 0, 0, 0};

	gdt_entries[1] = {
		.limit_low = 0,
		.base_low = 0,
		.base_middle = 0,
		.access = 0x9A,
		.granularity = 0xA0,
		.base_high = 0
	};

	gdt_entries[2] = {
		.limit_low = 0,
		.base_low = 0,
		.base_middle = 0,
		.access = 0x92,
		.granularity = 0xC0,
		.base_high = 0
	};

	gdt_descriptor.size = sizeof(gdt_entries) - 1;
	gdt_descriptor.offset = (uint64_t)&gdt_entries;

	load_gdt((uint64_t)&gdt_descriptor);
}
