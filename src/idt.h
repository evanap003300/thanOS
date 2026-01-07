#pragma once 
#include <stdint.h>

struct __attribute__((packed)) IDTEntry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist; 
	uint8_t type_attributes;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
};

struct __attribute__((packed)) IDTR {
	uint16_t limit;
	uint64_t base;
};

class IDT {
	public: 
		static IDTEntry idt[256];
		static IDTR idtr;
		
		static void set_gate(uint8_t vector, void* istr, uint8_t flags);

		static void init();
};
