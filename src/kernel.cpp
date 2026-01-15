#include <stdint.h>
#include "limine.h"
#include <stddef.h>
#include "graphics/render.h"
#include "cpu/idt.h"
#include "cpu/gdt.h"
#include "drivers/pic.h"
#include "shell/shell.h"
#include "cpu/pmm.h"

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
	terminal.printf("Kernel Address: %x\n", &kmain);
	
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

	pmm.init();

	terminal.printf("System Initalized.\n");
	shell.init();

	while (true) {
		__asm__ volatile ("hlt");
	}
}
