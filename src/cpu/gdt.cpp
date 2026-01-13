#include "cpu/gdt.h"

extern "C" void load_gdt(uint64_t gdt_descriptor_address);

// Define static members
GDTEntry GDT::gdt_entries[3];
GDTDescriptor GDT::gdt_descriptor;

void GDT::init() {
	
	// Null Entry
	gdt_entries[0] = {0, 0, 0, 0, 0, 0};

	// Kernel code segment entry
	gdt_entries[1] = {0, 0, 0, 0x9A, 0xA0, 0};

	// Kernel data segment entry
    	gdt_entries[2] = {0, 0, 0, 0x92, 0xC0, 0};

	gdt_descriptor.size = sizeof(gdt_entries) - 1;
	gdt_descriptor.offset = (uint64_t)&gdt_entries;

	load_gdt((uint64_t)&gdt_descriptor);
}
