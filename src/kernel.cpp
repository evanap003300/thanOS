#include <stdint.h>
#include "limine.h"
#include <stddef.h>

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

    	uint32_t* fb_ptr = (uint32_t*)framebuffer->address;
    	size_t width = framebuffer->width;
    	size_t height = framebuffer->height;

    	for (size_t i = 0; i < width * height; i++) {
        	fb_ptr[i] = 0xFFFFFFFF;
    	}

	hcf();
}
