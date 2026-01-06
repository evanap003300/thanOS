#include <stdint.h>
#include "limine.h"
#include <stddef.h>
#include "render.h"

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

	Renderer terminal(framebuffer);
	terminal.clear();
	terminal.print("System Initialized...\n");
	terminal.print("Welcome to thanOS v0.1\n");
	terminal.print("---------------------------------\n");
	terminal.print_number(67);
	terminal.print("\n");
	terminal.print_hex(4096);

	hcf();
}
