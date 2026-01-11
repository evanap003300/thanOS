#include "idt.h"

IDTEntry IDT::idt[256];
IDTR IDT::idtr;

extern "C" void load_idt(uint64_t idtr_addr);
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void isr32();
extern "C" void isr33();

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

	set_gate(0, (void*)isr0, 0x8E);
    	set_gate(1, (void*)isr1, 0x8E);
    	set_gate(2, (void*)isr2, 0x8E);
    	set_gate(3, (void*)isr3, 0x8E);
    	set_gate(4, (void*)isr4, 0x8E);
    	set_gate(5, (void*)isr5, 0x8E);
    	set_gate(6, (void*)isr6, 0x8E);
    	set_gate(7, (void*)isr7, 0x8E);
    	set_gate(8, (void*)isr8, 0x8E);
    	set_gate(9, (void*)isr9, 0x8E);
    	set_gate(10, (void*)isr10, 0x8E);
    	set_gate(11, (void*)isr11, 0x8E);
    	set_gate(12, (void*)isr12, 0x8E);
    	set_gate(13, (void*)isr13, 0x8E);
    	set_gate(14, (void*)isr14, 0x8E);
    	set_gate(15, (void*)isr15, 0x8E);
    	set_gate(16, (void*)isr16, 0x8E);
    	set_gate(17, (void*)isr17, 0x8E);
    	set_gate(18, (void*)isr18, 0x8E);
    	set_gate(19, (void*)isr19, 0x8E);
    	set_gate(20, (void*)isr20, 0x8E);
    	set_gate(21, (void*)isr21, 0x8E);
    	set_gate(22, (void*)isr22, 0x8E);
    	set_gate(23, (void*)isr23, 0x8E);
    	set_gate(24, (void*)isr24, 0x8E);
    	set_gate(25, (void*)isr25, 0x8E);
    	set_gate(26, (void*)isr26, 0x8E);
    	set_gate(27, (void*)isr27, 0x8E);
    	set_gate(28, (void*)isr28, 0x8E);
    	set_gate(29, (void*)isr29, 0x8E);
    	set_gate(30, (void*)isr30, 0x8E);
    	set_gate(31, (void*)isr31, 0x8E);
	set_gate(32, (void*)isr32, 0x8E);
	set_gate(33, (void*)isr33, 0x8E);

	load_idt((uint64_t)&idtr);
}
