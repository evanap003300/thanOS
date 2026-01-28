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
#include "std/string.h"
#include "std/vector.h"
#include "fs/tar.h"

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
	.revision = 0,
	.response = NULL  
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_module_request module_request = {
	.id = LIMINE_MODULE_REQUEST_ID,
	.revision = 0,
	.response = NULL,
	.internal_module_count = 0,
	.internal_modules = NULL
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

	void* heapStart = (void*)0x100000000;
	uint64_t page_count = 100;
	InitializeHeap(heapStart, page_count);
	terminal.printf("[OK] Heap Initalized\n");

	struct limine_file* module = module_request.response->modules[0];
		
	terminal.printf("[OK] Module Loaded\n");	
	terminal.printf("FS: Address: %x\n", module->address);
	terminal.printf("FS: Size:    %d bytes\n", module->size);
	terminal.printf("FS: Path:    %s\n", module->path);	

	Tar::parse((uint64_t)module->address);

	String sysInit = "System Initalized.\n";
	terminal.printf("%s", sysInit.c_str());

	shell.init();

	while (true) {
		__asm__ volatile ("hlt");
	}
}
