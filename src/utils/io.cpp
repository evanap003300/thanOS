#include "utils/io.h"

extern "C" {
	void* __dso_handle = 0;
	void __cxa_atexit(void (*)(void *), void *, void *) {
	
	}
}

void outb(uint16_t port, uint8_t value) {
	__asm__ volatile("outb %0, %1" : : "a"(value), "Nd" (port));
}

uint8_t inb(uint16_t port) {
	uint8_t value;
	__asm__ volatile("inb %1, %0" : "=a"(value): "Nd"(port));
	return value;
}

void io_wait(void) {
	outb(0x80, 0);
}

uint16_t insw(uint16_t port) {
	uint16_t result;
	__asm__ volatile ("inw %1, %0" : "=a" (result) : "Nd" (port));
	return result;
}
