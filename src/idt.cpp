#include "idt.h"

IDTEntry IDT::idt[256];
IDTR IDT::idtr;

extern "C" void load_idt(uint64_t idtr_addr);

void IDT::set_gate(uint8_t vector, void* isr, uint8_t flags) {
	uint64_t addr = (uint64_t)isr;

	IDTEntry* entry = &idt[vector];

	entry->offset_low = (uint16_t)(addr & 0xFFFF);
	entry->selector = 0x08;
	entry->ist = 0;
	entry->type_attributes = flags;
	entry->offset_mid = (uint16_t)((addr >> 16) & 0xFFFF);
	entry->offset_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
	entry->reserved = 0;
}

void IDT::init() {
	idtr.limit = sizeof(idt) - 1;
	idtr.base = (uint64_t)&idt;

	for (int i = 0; i < 256; i++) {
		set_gate(i, nullptr, 0);
	}

	load_idt((uint64_t)&idtr);
}
