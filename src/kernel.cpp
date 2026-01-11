#include <stdint.h>
#include "limine.h"
#include <stddef.h>
#include "render.h"
#include "idt.h"
#include "gdt.h"
#include "pic.h"

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
	
	// Check for errors
	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    	if (framebuffer == NULL) {
        	hcf();
    	}

	terminal = Renderer(framebuffer);

	terminal.clear();
	terminal.printf("System Initialized...\n");
	terminal.printf("Welcome to thanOS v%d.%d\n", 1, 0);
	terminal.printf("Kernel Address: %x\n", &kmain);
	terminal.printf("Status: %s\n", "Online");
	terminal.print("---------------------------------\n");
	
	terminal.printf("Init GDT... \n");
	GDT::init();
	terminal.printf("GDT Loaded!\n");
	
	terminal.printf("Init IDT... \n");
	IDT::init();
	terminal.printf("IDT Loaded!\n");

	// testing divide by zero (should cause interrupt)
	
	terminal.printf("Testing Interrupts...\n");
	
	volatile int a = 5;
	//volatile int b = 0;
	int c = a / 1; // change to b for testing error handling

	terminal.printf("Value of c: %d\n", c);

	// Trying out keyboard driver:
	pic_remap();
	pic_unmask(0);
	pic_unmask(1);

	terminal.printf("Enabling Interrupts...\n");
	__asm__ volatile ("sti");

	//hcf();
	
	terminal.printf("OS Online. Press 'A' to test!\n");

	terminal.draw_cursor(true);

	while (true) {
		__asm__ volatile ("hlt");
	}
}
