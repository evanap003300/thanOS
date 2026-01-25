#include <stdint.h>
#include "limine.h"
#include <stddef.h>
#include "graphics/render.h"
#include "cpu/idt.h"
#include "cpu/gdt.h"
#include "drivers/pic.h"
#include "shell/shell.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"
#include "memory/heap.h"

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0,
	.response = NULL  
};

// Halt and catch fire
static void hcf(void) {
	asm ("cli");
	for (;;) {
		asm ("hlt");
	}
}

// Init global renderer object
Renderer terminal;

// Kernel entry point
extern "C" void kmain(void) {
	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    	if (framebuffer == NULL) {
        	hcf();
    	}

	terminal = Renderer(framebuffer);

	terminal.clear();
	terminal.printf("System Initializing...\n");
	
	GDT::init();
	terminal.printf("[OK] GDT Loaded!\n");
	
	IDT::init();
	terminal.printf("[OK] IDT Loaded!\n");

	pic_remap();
	pic_unmask(0);
	pic_unmask(1);

	terminal.printf("[OK] Interrupts Loaded!\n");
	__asm__ volatile ("sti");

	terminal.draw_cursor(true);

	terminal.printf("[OK] PMM Initalized\n");
	pmm.init();

	uint64_t cr3_phys;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3_phys));

	kernel_vmm = PageTableManager((PageTable*)cr3_phys);
	terminal.printf("[OK] VMM Initalized\n");

	void* kmain_phys = kernel_vmm.virt_to_phys((void*)&kmain);
	terminal.printf("VMM: kmain Virtual Address: %x\n", &kmain);
	terminal.printf("VMM: kmain Physical Address: %x\n", kmain_phys);

	// testing heap
	void* heapStart = (void*)0x100000000;
	InitializeHeap(heapStart, 100);

	int* a = new int;
	*a = 5;
	terminal.printf("Dynamic Int: %d at %x\n", *a, a);

	char* str = new char[10];
	str[0] = 'H';
	str[1] = 'i';
	str[2] = '\0';
	
	terminal.printf("Dynamic String: %s at %x\n", str, str);

	delete a;
	delete[] str;

	terminal.printf("Freed Memory.\n");

	terminal.printf("System Initalized.\n");
	shell.init();

	while (true) {
		__asm__ volatile ("hlt");
	}
}
